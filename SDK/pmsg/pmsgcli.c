#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef WIN
#else
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>

#include "types.h"
#endif

#include "pmsgcli.h"
#include "Mutex.h"



typedef struct _T_PMSG_CONN_INFO
{
    int iSockfd;
    int iConnectStatus;
    int iThreadRunFlag;
    int iServPort;
    char acIpAddr[20];

#ifdef WIN
    HANDLE ThreadHandle;
#else
    pthread_t ThreadHandle;
#endif
    Mutex tPmsgMutex;
    PF_MSG_PROC_CALLBACK pMsgProcFunc;
} T_PMSG_CONN_INFO, *PT_PMSG_CONN_INFO;

BYTE GetMsgDataEcc(BYTE *pcData, INT32 iLen)
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
    int iRet = 0;
    struct sockaddr_in servaddr;
    struct timeval tv_out;
    char acIpAddr[20];

    iSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (iSockfd < 0)
    {
        perror("socket error:");
        return -1;
    }
 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    servaddr.sin_addr.s_addr = inet_addr(pcIpaddr);
    servaddr.sin_port = htons(u16Port);
   

    iRet = connect(iSockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (iRet < 0)
    {
        //printf("sockfd %d connect addr %s  port %d error\n", iSockfd, tSysAddr.acCameraAddr, tSysAddr.sCameraPort);
       // perror(":");
        close(iSockfd);        
        return -1;
    }
    
    tv_out.tv_sec = 2;
    tv_out.tv_usec = 500;
    setsockopt(iSockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
    setsockopt(iSockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof(tv_out));

    return iSockfd;
}

void DestroyTcpSocket(iSockfd)
{
   printf("[%s] enter, sockfd %d\n", __FUNCTION__, iSockfd);
   
   close(iSockfd);		
}

#ifdef WIN
	DWORD WINAPI CliProcessThread(void *arg)
#else
  void *CliProcessThread(void *arg)
#endif
{
    unsigned char *pcRecvBuf = NULL;
    unsigned char *pcLeaveBuf = NULL;
    unsigned char *pcMsgBuf = NULL;
    unsigned char ucMsgHead = 0;
    int iBufLen = 2048;
    int iSocket = 0;
    int iPreLeaveLen = 0, iLeaveLen = 0, iRecvLen = 0;
    int iMsgDataLen = 0;
    int iHearCount = 0;
    int iRet = 0;
    int iOffset = 0;
    fd_set	tAllSet, tTmpSet;
    struct timeval tv;
    time_t tCurTime = 0, tOldTime = 0;
    T_PMSG_CONN_INFO *ptPmsgConnInfo = (T_PMSG_CONN_INFO *)arg;
    PMSG_HANDLE pMsgHandle = (PMSG_HANDLE)arg;
    
    if (NULL == ptPmsgConnInfo)
    {
        return NULL;	
    }
    
    printf("[%s] enter\n", __FUNCTION__);
    
    pcRecvBuf = (unsigned char *)malloc(iBufLen);
    if (NULL == pcRecvBuf)
    {
        return NULL;        	
    }
    memset(pcRecvBuf, 0, iBufLen);
    
    pcLeaveBuf = (unsigned char *)malloc(iBufLen);
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
            ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
            printf("create tcp socket faile %s, %d\n", ptPmsgConnInfo->acIpAddr, ptPmsgConnInfo->iServPort);
            sleep(2);
            continue;
        }
        tv.tv_sec = 0;
        tv.tv_usec = 20000;
        
        tTmpSet = tAllSet;	//重新置位.
        if (select(iSocket + 1, &tTmpSet, NULL, NULL, &tv) > 0)
        {
            if (FD_ISSET(iSocket, &tTmpSet))
            {
                iHearCount = 0;
                iRecvLen = recv(iSocket, &pcRecvBuf[iPreLeaveLen], iBufLen - iPreLeaveLen - 1, 0);
                if (iRecvLen <= 0)
                {
                    perror("recv:");
                    printf("nvr serv %s exit, recv error\n", ptPmsgConnInfo->acIpAddr);

                    DestroyTcpSocket(iSocket);
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
                    if (iLeaveLen < 5)
                    {
                        memcpy(pcLeaveBuf, &pcRecvBuf[iOffset], iLeaveLen);
                        iPreLeaveLen = iLeaveLen;
                        break;    	
                    }
                        
                    pcMsgBuf = &pcRecvBuf[iOffset];

                    ucMsgHead = (unsigned char)pcMsgBuf[0];
                    
                    // 验证消息头的正确性
                    if (MSG_START_FLAG != ucMsgHead)
                    {
                        iPreLeaveLen = 0;
                        break;
                    }
                        
                    // 验证消息长度的正确性
                    iMsgDataLen = pcMsgBuf[2] << 8 | pcMsgBuf[3];
                    if (iMsgDataLen > 1024)
                    {
                        iPreLeaveLen = 0;
                        break;
                    }
                        
                    if (iMsgDataLen <= iLeaveLen - 5)
                    {
                        if (ptPmsgConnInfo->pMsgProcFunc)
                        {
                            ptPmsgConnInfo->pMsgProcFunc(pMsgHandle, pcMsgBuf[1], &pcMsgBuf[4], iMsgDataLen);
                        }
                        iLeaveLen -= iMsgDataLen + 5;
                        iOffset += iMsgDataLen + 5;
                        iPreLeaveLen = 0;
                        continue;
                    }
                    else
                    {
                        memcpy(pcLeaveBuf, &pcRecvBuf[iOffset], iLeaveLen);
                        iPreLeaveLen = iLeaveLen;
                        break;	
                    }
                }
            }
        }
        
        tCurTime = time(NULL);
        if (tCurTime != tOldTime)
        {
            iRet = PMSG_SendPmsgData((PMSG_HANDLE)ptPmsgConnInfo, CLI_SERV_MSG_TYPE_HEART, NULL, 0);
            if (iRet <= 0)
            {
                DestroyTcpSocket(iSocket);
                iSocket = -1;
                ptPmsgConnInfo->iSockfd = -1;
                ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
                iHearCount = 0;
            } 
        }
        tOldTime = tCurTime;
        
        iHearCount ++;

        if (iHearCount > 1000)
        {
            DestroyTcpSocket(iSocket);
            iSocket = -1;
            ptPmsgConnInfo->iSockfd = -1;
            ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
            iHearCount = 0;
        }
    }
    
    if (iSocket > 0)
    {
        DestroyTcpSocket(iSocket);	
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

PMSG_HANDLE PMSG_CreateConnect(char *pcIpAddr, int iPort, PF_MSG_PROC_CALLBACK pfMsgProcFunc)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = NULL;
    int iRet = 0;
    
    ptPmsgConnInfo = (PT_PMSG_CONN_INFO)malloc(sizeof(T_PMSG_CONN_INFO));
    if (NULL == ptPmsgConnInfo)
    {
        return 0;	
    }
    memset(ptPmsgConnInfo, 0, sizeof(PT_PMSG_CONN_INFO));
    
    Mutex_Initialize(&ptPmsgConnInfo->tPmsgMutex);
    
    ptPmsgConnInfo->iThreadRunFlag = 1;
    ptPmsgConnInfo->pMsgProcFunc = pfMsgProcFunc;
    strncpy(ptPmsgConnInfo->acIpAddr, pcIpAddr, sizeof(ptPmsgConnInfo->acIpAddr));
    ptPmsgConnInfo->iServPort = iPort;
    
    printf("servip %s, servport %d\n", ptPmsgConnInfo->acIpAddr, ptPmsgConnInfo->iServPort);

#ifdef WIN
    ptRtpConn->ThreadHandle = CreateThread(NULL, 0, CliProcessThread, ptPmsgConnInfo, 0, NULL);
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
    #ifdef WIN
        if (WAIT_OBJECT_0 == WaitForSingleObject(ptPmsgConnInfo->ThreadHandle, 100))
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
    //pthread_mutex_destroy(&ptPmsgConnInfo->tPmsgMutex);
    Mutex_Destroy(&ptPmsgConnInfo->tPmsgMutex);
    free(ptPmsgConnInfo);
    ptPmsgConnInfo = NULL;
    
    return 0;
}

int PMSG_GetConnectStatus(PMSG_HANDLE pMsgHandle)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    
    if (NULL == ptPmsgConnInfo)
    {
        return -1;	
    }

    return ptPmsgConnInfo->iConnectStatus;
}

int PMSG_SendRawData(PMSG_HANDLE pMsgHandle, char *pcData, int iDataLen)
{
	  char acMsg[64];
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

    Mutex_Lock(&ptPmsgConnInfo->tPmsgMutex);
    iRet = send(ptPmsgConnInfo->iSockfd, pcData, iDataLen, 0);
    Mutex_Unlock(&ptPmsgConnInfo->tPmsgMutex);
    
    return iRet;
}

int PMSG_SendPmsgData(PMSG_HANDLE pMsgHandle, unsigned char ucMsgCmd, char *pcData, int iDataLen)
{
    char acMsg[1024];
    unsigned char ucEcc = 0;
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    PT_PMSG_HEAD ptMsgHead = NULL;
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
    if (iDataLen > (1024 - 5))
    {
        printf("[%s]Msglen %d is too long\n", __FUNCTION__, iDataLen);
        iDataLen = 1024 -5;	
    }
    
    memset(acMsg, 0, sizeof(acMsg));
    ptMsgHead = (PT_PMSG_HEAD)acMsg;
    if (iDataLen > 0)
    {
        memcpy(&acMsg[sizeof(T_PMSG_HEAD)], pcData, iDataLen);
        i16SendLen = iDataLen;
    }
    ptMsgHead->cMsgStartFLag = MSG_START_FLAG;
    ptMsgHead->cMsgType = ucMsgCmd;
    ptMsgHead->sMsgLen = htons(i16SendLen);
            
    // 计算ECC校验
    ucEcc = GetMsgDataEcc((BYTE *)ptMsgHead, sizeof(T_PMSG_HEAD));
    if (i16SendLen > 0)
    {
        ucEcc ^= GetMsgDataEcc(&acMsg[sizeof(T_PMSG_HEAD)], i16SendLen);
    }       
    acMsg[sizeof(T_PMSG_HEAD) + i16SendLen] = ucEcc;

    Mutex_Lock(&ptPmsgConnInfo->tPmsgMutex);
    iRet = send(ptPmsgConnInfo->iSockfd, acMsg, sizeof(T_PMSG_HEAD) + i16SendLen + 1, 0);
    Mutex_Unlock(&ptPmsgConnInfo->tPmsgMutex);
    
    return iRet;
}
