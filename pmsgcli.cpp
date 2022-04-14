
#include "pmsgcli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <WinSock2.h>
#include "NVRMacroSet.h"
#include "./SDK/state/state.h"
#include <QDebug>
#define FL8T2(x) QString::fromLocal8Bit(x)
#pragma pack(1)

enum E_STARTUP_MODE
{
    E_UNKNOWN = -1,
    E_CARRIAGE = 0,
    E_PANTOGRAPH = 1
};



typedef struct _T_PMSG_CONN_INFO
{
    int iSockfd;
    int iConnectStatus; // E_SERV_STATUS_UNCONNECT E_SERV_STATUS_CONNECT
    int iThreadRunFlag;
    int iServPort;
    char acIpAddr[20];
    void *pPrivateData;

#ifdef WIN32
    HANDLE ThreadHandle;
#else
    pthread_t ThreadHandle;
#endif
    HANDLE hPmsgMutex;
    PF_MSG_PROC_CALLBACK pMsgProcFunc;
} T_PMSG_CONN_INFO, *PT_PMSG_CONN_INFO;


#pragma pack()

BYTE GetMsgDataEcc(BYTE  *pcData, INT32 iLen)
{
    int i = 0;
    BYTE ucEcc = 0;
    
    if ((NULL == pcData) || (0 == iLen))	
    {
        return 0;	
    }
    
    for (i = 0; i < iLen; i++)
    {
        	ucEcc ^= pcData[i];
    }
    
    return ucEcc;
}

int CreateTcpSocket(char *pcIpaddr, unsigned short u16Port)
{
    int iSockfd = 0;
    struct sockaddr_in servaddr;
#ifdef WIN32
    int tv_out = 5000;
#else
    struct timeval tv_out;
    tv_out.tv_sec = 5;
    tv_out.tv_usec = 0;

#endif
    iSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (iSockfd < 0)
    {
       // perror("socket error:");
        return SOCKET_ERROR;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    servaddr.sin_addr.s_addr = inet_addr(pcIpaddr);
    servaddr.sin_port = htons(u16Port);

    int iMode = 1;
    int ret =ioctlsocket(iSockfd, FIONBIO, (u_long FAR*)&iMode);
    if(ret==SOCKET_ERROR)
    {
        closesocket(iSockfd);
        iSockfd = NULL;
        return SOCKET_ERROR;
    }
    if(SOCKET_ERROR == connect(iSockfd, (sockaddr *)&servaddr, sizeof(servaddr)))
    {
        struct timeval tm;
        tm.tv_sec  = 1;
        tm.tv_usec = 0;
        int ret = -1;
        fd_set set;
        FD_ZERO(&set);
        FD_SET(iSockfd, &set);

        if (select(-1, NULL, &set, NULL, &tm) <= 0)
        {
            closesocket(iSockfd);
            iSockfd = NULL;
            return SOCKET_ERROR;
        }
        else
        {
            int error = -1;
            int optLen = sizeof(int);
            getsockopt(iSockfd, SOL_SOCKET, SO_ERROR, (char*)&error, &optLen);

            if (0 != error)
            {
                closesocket(iSockfd);
                iSockfd = NULL;
                return SOCKET_ERROR;
            }
            else
            {
                iMode = 0;
                ioctlsocket(iSockfd, FIONBIO, (u_long FAR*)&iMode);
                ret = 1;
            }
        }
    }

    setsockopt(iSockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv_out, sizeof(tv_out));
    setsockopt(iSockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv_out, sizeof(tv_out));

    return iSockfd;
}

void DestroyTcpSocket(int iSockfd)
{
   printf("[%s] enter, sockfd %d\n", __FUNCTION__, iSockfd);
   
#ifdef WIN32
   closesocket(iSockfd);
#else
   close(iSockfd);

#endif
	
}
#ifdef WIN32
	DWORD WINAPI CliProcessThreadRead(void *arg)
#else
  void *CliProcessThreadRead(void *arg)
#endif
{
    char *pcRecvBuf = NULL;     //用来接收数据
    char *pcLeaveBuf = NULL;    //用来保存上次未解析完的数据
    unsigned char ucMsgHead = 0;
    int iBufLen = 3200;
    int iSocket = 0;
    unsigned int iPreLeaveLen = 0;   //一条完整信息前面已收到的数据长度
    unsigned int iLeaveLen = 0;      //收到的数据中还未解析的数据长度
    /*unsigned*/ int iRecvLen = 0;       //本次recv到的数据长度
    unsigned int iDataLen = 0;    //完整的一条信息的数据长度
    int iHearCount = 0;
    int iOffset = 0;        //接收到的数据中还未解析的数据的开始位置
    fd_set	tAllSet, tTmpSet;
    struct timeval tv;
    int iRet = 0,nCommonHeaderLength = sizeof(T_MSG_HEAD)+sizeof(BYTE);
    
    time_t tCurTime = 0, tOldTime = 0;
    T_PMSG_CONN_INFO *ptPmsgConnInfo = (T_PMSG_CONN_INFO *)arg;
    PMSG_HANDLE pMsgHandle = (PMSG_HANDLE)arg;
    
    if (NULL == ptPmsgConnInfo)
    {
        return NULL;	
    }
    
    printf("[%s] enter\n", __FUNCTION__);
    
    pcRecvBuf = (char *)malloc(iBufLen);
    if (NULL == pcRecvBuf)
    {
        return NULL;        	
    }
    memset(pcRecvBuf, 0, iBufLen);
    
    pcLeaveBuf = (char *)malloc(iBufLen);
    if (NULL == pcLeaveBuf)
    {
        free(pcRecvBuf);
        return NULL;        	
    }
    memset(pcLeaveBuf, 0, iBufLen);
	int nMsgHeaderLength = sizeof(T_MSG_HEAD);
	int nDataLength = 0,nToRecvLength = 0;
	unsigned char cmdType;
	memset(pcRecvBuf,0,sizeof(pcRecvBuf));
			int nPreDataMove = 0;
    while (ptPmsgConnInfo->iThreadRunFlag)
    {
        if (iSocket <= 0)
        {
            iSocket = CreateTcpSocket(ptPmsgConnInfo->acIpAddr, (unsigned short)ptPmsgConnInfo->iServPort);
            if (iSocket > 0)
            {
                FD_ZERO(&tAllSet);
                FD_SET(iSocket, &tAllSet);
                ptPmsgConnInfo->iSockfd = iSocket;
                ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_CONNECT;
                printf("create tcp socket %d\n", iSocket);
				
				if (ptPmsgConnInfo->pMsgProcFunc)
					ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_ONLINE,0,0);
            }
            
        }
        if (iSocket <= 0)
        {
#ifdef WIN32
            Sleep(2);

#else
            sleep(2);
#endif
            ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
			if (ptPmsgConnInfo->pMsgProcFunc)
				ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
            //printf("create tcp socket faile %s, %d\n", ptPmsgConnInfo->acIpAddr, ptPmsgConnInfo->iServPort);
            continue;
        }

        tv.tv_sec = 0;
        tv.tv_usec = 2000;
        tTmpSet = tAllSet;	//重新置位.
		
        if (select(iSocket + 1, &tTmpSet, NULL, NULL,&tv) > 0)
        {
            if (FD_ISSET(iSocket, &tTmpSet))
            {
                iHearCount = 0;
		
				if (nDataLength>0)
				{
					nToRecvLength   = nDataLength;
				}
				else
				{
					nToRecvLength   =nMsgHeaderLength;
				}
				
                iRecvLen = recv(iSocket, pcRecvBuf+	nPreDataMove,nToRecvLength, MSG_WAITALL);
                if (iRecvLen <= 0)
                {
					//异常终止TCP SOCKET
                    perror("recv:");
                    printf("nvr serv %s exit, recv error\n", ptPmsgConnInfo->acIpAddr);
                    DestroyTcpSocket(iSocket);
                    iSocket = -1;
                    ptPmsgConnInfo->iSockfd = -1;
                    ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
					if (ptPmsgConnInfo->pMsgProcFunc)
 						ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
                    continue;
                }

              //读取到的TCP长度:iRecvLen; 
               
                    if (iRecvLen < nMsgHeaderLength&&iRecvLen< nToRecvLength)
                    {
						perror("recv:");
						printf("nvr serv %s exit, recv error\n", ptPmsgConnInfo->acIpAddr);
						DestroyTcpSocket(iSocket);
						
						iSocket = -1;
						ptPmsgConnInfo->iSockfd = -1;
						ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
						   qDebug()<<FL8T2("异常终止TCP SOCKET:")<<iRecvLen; 
						if (ptPmsgConnInfo->pMsgProcFunc)
							ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
						continue;
						
                    }
                    ucMsgHead = (unsigned char)pcRecvBuf[0];
                    if (nDataLength==0)
                    {
						// 验证消息头的正确性
						if (0xFF != ucMsgHead)
						{
							DestroyTcpSocket(iSocket);
							iSocket = -1;
							ptPmsgConnInfo->iSockfd = -1;
							ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
							if (ptPmsgConnInfo->pMsgProcFunc)
								ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
							continue;
						}
						// 验证消息长度的正确性
						iDataLen = MAKEWORD(pcRecvBuf[3],pcRecvBuf[2]);
						if (iDataLen <0)
						{
							DestroyTcpSocket(iSocket);
							iSocket = -1;
							ptPmsgConnInfo->iSockfd = -1;
							ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
							if (ptPmsgConnInfo->pMsgProcFunc)
								ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
							continue;;
						}
						else
						{
							//结束位为验证 1字节
							nDataLength = iDataLen+1;
							//下次应接受到的数据长度:nDataLength; 
							cmdType=pcRecvBuf[1];
							nPreDataMove=iRecvLen;
							//只有心跳暂时不处理
						}
                    }
					else
					{
						if (ptPmsgConnInfo->pMsgProcFunc)
						{		 
							ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,cmdType , &pcRecvBuf[4], iDataLen);
							nDataLength=0;
							nPreDataMove=0;
							memset(pcRecvBuf,0,sizeof(pcRecvBuf));
						}
					}
                
            }
        }      
        tCurTime = time(NULL);
        if (tCurTime - tOldTime >5)
        {
            iRet = PMSG_SendPmsgData((PMSG_HANDLE)ptPmsgConnInfo, CLI_SERV_MSG_TYPE_HEART, NULL, 0);
            if (iRet <= 0)
            {
                DestroyTcpSocket(iSocket);
                iSocket = -1;
                ptPmsgConnInfo->iSockfd = -1;
                ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
				if (ptPmsgConnInfo->pMsgProcFunc)
					ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
            } 
            else
            {
                iHearCount = 0;
            }
			 tOldTime = tCurTime;
        }
        iHearCount ++;
        Sleep(3);
    }
    
    if (iSocket > 0)
    {
        DestroyTcpSocket(iSocket);	
		if (ptPmsgConnInfo->pMsgProcFunc)
			ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
        ptPmsgConnInfo->iSockfd = -1;
    }
    
    if (pcRecvBuf)
    {
        free(pcRecvBuf);
        pcRecvBuf = NULL;	
    }
    
    if (pcLeaveBuf)
    {
        free(pcLeaveBuf);
        pcLeaveBuf = NULL;	
    }
    
    printf("[%s] exit\n", __FUNCTION__);
    
    return NULL;
}
#ifdef WIN32
	DWORD WINAPI CliProcessThread(void *arg)
#else
  void *CliProcessThread(void *arg)
#endif
{
    char *pcRecvBuf = NULL;     //用来接收数据
    char *pcLeaveBuf = NULL;    //用来保存上次未解析完的数据
    char *pcMsgBuf = NULL;      //信息的数据位置
    unsigned char ucMsgHead = 0;
    int iBufLen = 3200;
    int iSocket = 0;
    unsigned int iPreLeaveLen = 0;   //一条完整信息前面已收到的数据长度
    unsigned int iLeaveLen = 0;      //收到的数据中还未解析的数据长度
    /*unsigned*/ int iRecvLen = 0;       //本次recv到的数据长度
    unsigned int iDataLen = 0;    //完整的一条信息的数据长度
    int iHearCount = 0;
    int iOffset = 0;        //接收到的数据中还未解析的数据的开始位置
    fd_set	tAllSet, tTmpSet;
    struct timeval tv;
    int iRet = 0,nCommonHeaderLength = sizeof(T_MSG_HEAD)+sizeof(BYTE);
    
    time_t tCurTime = 0, tOldTime = 0;
    T_PMSG_CONN_INFO *ptPmsgConnInfo = (T_PMSG_CONN_INFO *)arg;
    PMSG_HANDLE pMsgHandle = (PMSG_HANDLE)arg;
    
    if (NULL == ptPmsgConnInfo)
    {
        return NULL;	
    }
    
    printf("[%s] enter\n", __FUNCTION__);
    
    pcRecvBuf = (char *)malloc(iBufLen);
    if (NULL == pcRecvBuf)
    {
        return NULL;        	
    }
    memset(pcRecvBuf, 0, iBufLen);
    
    pcLeaveBuf = (char *)malloc(iBufLen);
    if (NULL == pcLeaveBuf)
    {
        free(pcRecvBuf);
        return NULL;        	
    }
    memset(pcLeaveBuf, 0, iBufLen);
    
    while (ptPmsgConnInfo->iThreadRunFlag)
    {
        if (iSocket <= 0)
        {
            iSocket = CreateTcpSocket(ptPmsgConnInfo->acIpAddr, (unsigned short)ptPmsgConnInfo->iServPort);
            if (iSocket > 0)
            {
                FD_ZERO(&tAllSet);
                FD_SET(iSocket, &tAllSet);
                ptPmsgConnInfo->iSockfd = iSocket;
                ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_CONNECT;
                printf("create tcp socket %d\n", iSocket);
            }
            
        }
        if (iSocket <= 0)
        {
#ifdef WIN32
            Sleep(2);

#else
            sleep(2);
#endif
            ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
            //printf("create tcp socket faile %s, %d\n", ptPmsgConnInfo->acIpAddr, ptPmsgConnInfo->iServPort);
            continue;
        }

        tv.tv_sec = 0;
        tv.tv_usec = 20000;

        tTmpSet = tAllSet;	//重新置位.
        if (select(iSocket + 1, &tTmpSet, NULL, NULL,&tv) > 0)
        {
            if (FD_ISSET(iSocket, &tTmpSet))
            {
                iHearCount = 0;
                iRecvLen = recv(iSocket, &pcRecvBuf[iPreLeaveLen], iBufLen - iPreLeaveLen - 1, 0);
				if(iRecvLen>=2000)
				{
					printf("");
				}
                if (iRecvLen <= 0)
                {
                    perror("recv:");
                    printf("nvr serv %s exit, recv error\n", ptPmsgConnInfo->acIpAddr);

                    DestroyTcpSocket(iSocket);
					if (ptPmsgConnInfo->pMsgProcFunc)
						ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
                    iSocket = -1;
                    ptPmsgConnInfo->iSockfd = -1;
                    ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
                    
                    continue;
                }
                if (iPreLeaveLen > 0)
                {
                    memcpy(pcRecvBuf, pcLeaveBuf, iPreLeaveLen);
                }
                
                iLeaveLen = iRecvLen + iPreLeaveLen;
                iOffset = 0;
                while (iLeaveLen > 0)
                {
                    if (iLeaveLen < nCommonHeaderLength )
                    {
                        memcpy(pcLeaveBuf, &pcRecvBuf[iOffset], iLeaveLen);
                        iPreLeaveLen = iLeaveLen;
                        break;    	
                    }
                        
                    pcMsgBuf = &pcRecvBuf[iOffset];

                    ucMsgHead = (unsigned char)pcMsgBuf[0];
                    
                    // 验证消息头的正确性
                    if (0xFF != ucMsgHead)
                    {
                        iPreLeaveLen = 0;
                        break;
                    }
                        
                    // 验证消息长度的正确性
                    iDataLen = MAKEWORD(pcMsgBuf[3],pcMsgBuf[2]);
                    if (iDataLen > iBufLen)
                    {
                        iPreLeaveLen = 0;
                        break;
                    }
                        
                    if (iDataLen <= iLeaveLen - nCommonHeaderLength)
                    {
                        if (ptPmsgConnInfo->pMsgProcFunc)
                        {
							if (iDataLen>0)
							{
									printf("other data");
							}
                            ptPmsgConnInfo->pMsgProcFunc(pMsgHandle, pcMsgBuf[1], &pcMsgBuf[4], iDataLen);
                        }
                        iLeaveLen -= iDataLen + nCommonHeaderLength;
                        iOffset += iDataLen + nCommonHeaderLength;
                        iPreLeaveLen = 0;
                        continue;
                    }
                    else
                    {
                        memcpy(pcLeaveBuf, &pcRecvBuf[iOffset], nCommonHeaderLength);
                        iPreLeaveLen = nCommonHeaderLength;
						//iOffset += iDataLen + nCommonHeaderLength;
                        break;	
                    }
                }
            }
        }
        
        tCurTime = time(NULL);
        if (tCurTime - tOldTime >5)
        {
            iRet = PMSG_SendPmsgData((PMSG_HANDLE)ptPmsgConnInfo, CLI_SERV_MSG_TYPE_HEART, NULL, 0);
            if (iRet <= 0)
            {
                DestroyTcpSocket(iSocket);
				if (ptPmsgConnInfo->pMsgProcFunc)
					ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
                iSocket = -1;
                ptPmsgConnInfo->iSockfd = -1;
                ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;

            } 
            else
            {
                iHearCount = 0;
            }
			 tOldTime = tCurTime;
        }
       
        iHearCount ++;

        /*if (iHearCount > 500)
        {
            DestroyTcpSocket(iSocket);
            iSocket = -1;
            ptPmsgConnInfo->iSockfd = -1;
            ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
            iHearCount = 0;
        }*/
        Sleep(3);
    }
    
    if (iSocket > 0)
    {
        DestroyTcpSocket(iSocket);	
		if (ptPmsgConnInfo->pMsgProcFunc)
			ptPmsgConnInfo->pMsgProcFunc(pMsgHandle,SERV_CLI_MSG_TYPE_OFFLINE,0,0);
        ptPmsgConnInfo->iSockfd = -1;
    }
    
    if (pcRecvBuf)
    {
        free(pcRecvBuf);
        pcRecvBuf = NULL;	
    }
    
    if (pcLeaveBuf)
    {
        free(pcLeaveBuf);
        pcLeaveBuf = NULL;	
    }
    
    printf("[%s] exit\n", __FUNCTION__);
    
    return NULL;
}


int PMSG_Init(void)
{
#ifdef WIN32
	WSADATA wsa={0};
	WSAStartup(MAKEWORD(2,2),&wsa);    
#endif

    return 0;
}

int PMSG_Uninit(void)
{
#ifdef WIN32
	WSACleanup();
#endif

    return 0;
}

PMSG_HANDLE PMSG_CreateConnect(char *pcIpAddr, int iPort, PF_MSG_PROC_CALLBACK pfMsgProcFunc,void *pPrivateData)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = NULL;    
    ptPmsgConnInfo = (PT_PMSG_CONN_INFO)malloc(sizeof(T_PMSG_CONN_INFO));
    if (NULL == ptPmsgConnInfo)
    {
        return 0;	
    }
    memset(ptPmsgConnInfo, 0, sizeof(PT_PMSG_CONN_INFO));
    
    ptPmsgConnInfo->hPmsgMutex = CreateMutex(nullptr,false,nullptr);

    ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
    ptPmsgConnInfo->iThreadRunFlag = 1;
    ptPmsgConnInfo->pMsgProcFunc = pfMsgProcFunc;
    strncpy(ptPmsgConnInfo->acIpAddr, pcIpAddr, sizeof(ptPmsgConnInfo->acIpAddr));
    ptPmsgConnInfo->iServPort = iPort;
    ptPmsgConnInfo->pPrivateData = pPrivateData;
    printf("servip %s, servport %d\n", ptPmsgConnInfo->acIpAddr, ptPmsgConnInfo->iServPort);

#ifdef _WIN32
	ptPmsgConnInfo->ThreadHandle = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CliProcessThreadRead, ptPmsgConnInfo, 0, NULL);
#else
    iRet = pthread_create(&ptPmsgConnInfo->ThreadHandle, NULL, CliProcessThread, (void *)ptPmsgConnInfo);
    if (iRet < 0)
    {
        free(ptPmsgConnInfo);
        ptPmsgConnInfo = NULL;
        return 0;	
    }
#endif

    return (PMSG_HANDLE)ptPmsgConnInfo;
}

void *PMSG_GetPrivateData(PMSG_HANDLE pMsgHandle)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;

    if (NULL == ptPmsgConnInfo)
    {
        return NULL;
    }

    return ptPmsgConnInfo->pPrivateData;
}

int PMSG_DestroyConnect(PMSG_HANDLE pMsgHandle)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    
    if (NULL == ptPmsgConnInfo)
    {
        return -1;	
    }
    ptPmsgConnInfo->iThreadRunFlag = 0;
    
    // join thread exit
    if (ptPmsgConnInfo->ThreadHandle)
    {
    #ifdef WIN32
        if (WAIT_OBJECT_0 == WaitForSingleObject(ptPmsgConnInfo->ThreadHandle, 1000))
        {
            CloseHandle(ptPmsgConnInfo->ThreadHandle);
        } 
        else
        {
            DWORD nExitCode;
            GetExitCodeThread(ptPmsgConnInfo->ThreadHandle, &nExitCode);
            if (STILL_ACTIVE == nExitCode)
            {
                TerminateThread(ptPmsgConnInfo->ThreadHandle, nExitCode);
                CloseHandle(ptPmsgConnInfo->ThreadHandle);
            }						
        }
    #else
        pthread_join(ptPmsgConnInfo->ThreadHandle, NULL);
    #endif
    }
    CloseHandle(ptPmsgConnInfo->hPmsgMutex);
    free(ptPmsgConnInfo);
    ptPmsgConnInfo = NULL;
    
    return 0;
}

int PMSG_GetConnectStatus(PMSG_HANDLE pMsgHandle)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    
    if (NULL == ptPmsgConnInfo)
    {
        return E_SERV_STATUS_UNCONNECT;	
    }

    return ptPmsgConnInfo->iConnectStatus;
}

int PMSG_SendRawData(PMSG_HANDLE pMsgHandle, char *pcData, int iDataLen)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    int iRet = 0;
    
    if ((NULL == ptPmsgConnInfo) || (NULL == pcData) || (iDataLen <= 0))
    {
        return -1;	
    }
    if (E_SERV_STATUS_CONNECT != ptPmsgConnInfo->iConnectStatus)
    {
        return -1;
    }

    WaitForSingleObject(ptPmsgConnInfo->hPmsgMutex,INFINITE);
    iRet = send(ptPmsgConnInfo->iSockfd, pcData, iDataLen, 0);
    ReleaseMutex(ptPmsgConnInfo->hPmsgMutex);
    
    return iRet;
}

int PMSG_SendPmsgData(PMSG_HANDLE pMsgHandle, unsigned char ucMsgCmd, char *pcData, int iDataLen)
{
    char acMsg[2048] = {0};
    //unsigned char ucEcc = 0;
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    PT_MSG_HEAD ptMsgHead = NULL;
    short i16SendLen = 0;
    int iRet = 0;
    
    if (NULL == ptPmsgConnInfo)
    {
        return -1;	
    }
    if (E_SERV_STATUS_CONNECT != ptPmsgConnInfo->iConnectStatus)
    {
        return -1;
    }
    if (iDataLen > (2048 - 4))
    {
        printf("[%s]Msglen %d is too long\n", __FUNCTION__, iDataLen);
        iDataLen = 2048 -4;
    }
    
    memset(acMsg, 0, sizeof(acMsg));
    ptMsgHead = (PT_MSG_HEAD)acMsg;
    if (iDataLen > 0)
    {
        memcpy(&acMsg[sizeof(T_MSG_HEAD)], pcData, iDataLen);
        i16SendLen = iDataLen;
    }
	else
	{
		i16SendLen = 2;
	}
    ptMsgHead->magic = 0xFF;
    ptMsgHead->cmd = ucMsgCmd;
    ptMsgHead->len = htons(i16SendLen);
            
    // 计算ECC校验
    BYTE ucEcc = GetMsgDataEcc((BYTE *)ptMsgHead, sizeof(T_PMSG_HEAD));
    if (iDataLen > 0)
    {
        ucEcc ^= GetMsgDataEcc((BYTE *)&acMsg[sizeof(T_PMSG_HEAD)], iDataLen);
    }       
    acMsg[sizeof(T_PMSG_HEAD) + iDataLen] = ucEcc;

    WaitForSingleObject(ptPmsgConnInfo->hPmsgMutex,INFINITE);
	if (iDataLen==0)
	{
		iRet = send(ptPmsgConnInfo->iSockfd, acMsg, sizeof(T_MSG_HEAD)+1 , 0);
	}
	else
	{
		iRet = send(ptPmsgConnInfo->iSockfd, acMsg, sizeof(T_MSG_HEAD) + i16SendLen+1, 0);
	}
    
    ReleaseMutex(ptPmsgConnInfo->hPmsgMutex);
    
    return iRet;
}
