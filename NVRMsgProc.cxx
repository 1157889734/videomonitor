#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include<QMutex>
#ifdef WIN32
#include <Windows.h>
#else
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

#include <QDebug>
#include "NVRMsgProc.h"

using namespace std;

#define NVR_PORT 10100

typedef struct _T_CMD_PACKET_LIST
{
    T_CMD_PACKET tPkt;
    struct _T_CMD_PACKET_LIST *next;
}T_CMD_PACKET_LIST;

typedef struct _T_CMD_QUEUE
{
    T_CMD_PACKET_LIST *ptFirst, *ptLast;
    int iPktCount;
#ifdef WIN32
      QMutex tMutex;
#else
      pthread_mutex_t tMutex;
#endif

				
} T_CMD_QUEUE, *PT_CMD_QUEUE;

typedef struct _T_NVR_NET_INFO
{
	PMSG_HANDLE  NVRMsgHandle;
	PT_CMD_QUEUE PtCmdQueue;
	PT_CMD_QUEUE ptFileQueue;
	int			 iThreadRun;
}T_NVR_NET_INFO;

static T_NVR_NET_INFO g_tNVRNetnfo[6];
static int            s_iNvrTotalNum = 0;
static HANDLE         g_cmdProcessThreadHandle;
static int 			  g_iCmdProcessThreadRunFlag =0;
static bool           g_bSearchDataReturn=false;

PT_CMD_QUEUE CreateCmdQueue()
{
    T_CMD_QUEUE *ptCmdQueue = NULL;

    ptCmdQueue = (PT_CMD_QUEUE)malloc(sizeof(T_CMD_QUEUE));
    if (NULL == ptCmdQueue)
    {
        return NULL;
    }
    memset(ptCmdQueue, 0, sizeof(T_CMD_QUEUE));
#ifdef WIN32
#else
    pthread_mutexattr_t	mutexattr;
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_TIMED_NP);
    pthread_mutex_init(&ptCmdQueue->tMutex, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
#endif


    ptCmdQueue->ptLast = NULL;
    ptCmdQueue->ptFirst = NULL;
    ptCmdQueue->iPktCount= 0;

    return ptCmdQueue;
}

int CleanNodeFromCmdQueue(PT_CMD_QUEUE ptCmdQueue)
{
	T_CMD_PACKET_LIST *ptPktList = NULL, *ptTmp;

    if (NULL == ptCmdQueue)
    {
        return -1;
    }
#ifdef WIN32
    ptCmdQueue->tMutex.lock();
#else
    pthread_mutex_lock(&ptCmdQueue->tMutex);
#endif

    ptPktList = ptCmdQueue->ptFirst;
    while (ptPktList)
    {
        ptTmp = ptPktList;
		free(ptPktList->tPkt.pData);
        ptPktList = ptPktList->next;
        free(ptTmp);
    }
	
    ptCmdQueue->ptLast = NULL;
    ptCmdQueue->ptFirst = NULL;
    ptCmdQueue->iPktCount= 0;   


#ifdef WIN32
    ptCmdQueue->tMutex.unlock();
#else
    pthread_mutex_unlock(&ptCmdQueue->tMutex);
#endif

    return 0;
}
static int FindHandleNo(PMSG_HANDLE PHandle)
{
	for(int i=0;i<6;i++)
	{
		if(g_tNVRNetnfo[i].NVRMsgHandle == PHandle)
		{
			return i;
		}
	}
	return -1;
}


int DestroyCmdQueue(PT_CMD_QUEUE ptCmdQueue)
{
    if (NULL == ptCmdQueue)
    {
        return -1;
    }
    CleanNodeFromCmdQueue(ptCmdQueue);
    free(ptCmdQueue);
    ptCmdQueue = NULL;
    return 0;
}

int PutNodeToCmdQueue(PT_CMD_QUEUE ptCmdQueue, PT_CMD_PACKET ptPkt)
{
    T_CMD_PACKET_LIST *ptPktList = NULL;


    if ((NULL == ptCmdQueue) || (NULL == ptPkt))
    {
        return -1;
    }
    ptPktList = (T_CMD_PACKET_LIST *)malloc(sizeof(T_CMD_PACKET_LIST));
    if (NULL == ptPktList)
    {
        return -1;
    }

    memset(ptPktList, 0, sizeof(T_CMD_PACKET_LIST));
    ptPktList->tPkt = *ptPkt;

#ifdef WIN32
       ptCmdQueue->tMutex.lock();
#else
       pthread_mutex_lock(&ptCmdQueue->tMutex);
#endif

    
    if (NULL == ptCmdQueue->ptLast)
    {
	    ptCmdQueue->ptFirst = ptPktList;
    }
    else
    {
	    ptCmdQueue->ptLast->next = ptPktList;
    }
    ptCmdQueue->ptLast = ptPktList;
    ptCmdQueue->iPktCount++;
#ifdef WIN32
       ptCmdQueue->tMutex.unlock();
#else
       pthread_mutex_unlock(&ptCmdQueue->tMutex);
#endif

   
    return 0;
}

int GetNodeFromCmdQueue(PT_CMD_QUEUE ptCmdQueue, PT_CMD_PACKET ptPkt)
{
    T_CMD_PACKET_LIST *ptTmp = NULL;

    if ((NULL == ptCmdQueue) || (NULL == ptPkt))
    {
        return 0;
    }

#ifdef WIN32
     ptCmdQueue->tMutex.lock();
#else
     pthread_mutex_lock(&ptCmdQueue->tMutex);
#endif

    
    if (NULL == ptCmdQueue->ptFirst)
    {        
#ifdef WIN32
        ptCmdQueue->tMutex.unlock();
#else
         pthread_mutex_unlock(&ptCmdQueue->tMutex);
#endif

        return 0;
    }
	
#ifdef WIN32
     ptCmdQueue->tMutex.unlock();
#else
     pthread_mutex_unlock(&ptCmdQueue->tMutex);
#endif

    ptTmp = ptCmdQueue->ptFirst;
    ptCmdQueue->ptFirst = ptCmdQueue->ptFirst->next;
    if (NULL == ptCmdQueue->ptFirst)
    {
        ptCmdQueue->ptLast= NULL;
    }
    ptCmdQueue->iPktCount--;
    *ptPkt = ptTmp->tPkt;
    free(ptTmp);
    
    return 1;
}

int PmsgProc(PMSG_HANDLE PHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen)
{
	int iIPCNum = 0;
	int iNvrNo = FindHandleNo(PHandle);
	T_CMD_PACKET tPkt;
	int iNum =0;
	
    switch(ucMsgCmd)
    {
    case SERV_CLI_MSG_TYPE_HEART:
	{
		break;
	}
	case  SERV_CLI_MSG_TYPE_OFFLINE:
	{
		STATE_CleanIPCState(iNvrNo);
		//NVR_DisConnect(iNvrNo);
		break;
	}
	case SERV_CLI_MSG_TYPE_ONLINE:
	{
		NVR_SendCmdInfo(iNvrNo, CLI_SERV_MSG_TYPE_GET_IPC_STATUS, NULL, 0);
		//NVR_Connect(iNvrNo);
		break;
	}
	case SERV_CLI_MSG_TYPE_GET_PIC_ATTRIBUTE_RESP:
	{
        STATE_SetIpcAttribute(iNvrNo, (PT_IPC_ID)pcMsgData, (T_IPC_ATTRIBUTE_INFO*)(pcMsgData+sizeof(T_IPC_ID)));
		break;
	}
	case  SERV_CLI_MSG_TYPE_SET_CARRIAGE_NUM_RESP:
	{
		qDebug()<<pcMsgData;
		break;
	}
	case SERV_CLI_MSG_TYPE_GET_NVR_STATUS_RESP:
	{
		
        STATE_SetNvrState(iNvrNo, (T_NVR_STATE*)pcMsgData);
		break;
	}
	case SERV_CLI_MSG_TYPE_GET_IPC_STATUS_RESP:
	{
		
        STATE_SetIpcState(iNvrNo, (PT_IPC_ID)pcMsgData, (T_IPC_STATE*)(pcMsgData+sizeof(T_IPC_ID)));
		STATE_NVROnline(iNvrNo);
		break;
	}
	case SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP:
    {
		E_FILE_SEARCH_STATE eFileState;
        eFileState = STATE_GetFileSearchState();
		if((iNvrNo >= 0) && (E_FILE_SEARCHING == eFileState))
		{
			tPkt.iMsgCmd = SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP;
			g_bSearchDataReturn = true;
			if (iMsgDataLen > 2)
			{
				tPkt.pData = new char[iMsgDataLen-1]; //前2个字节为包的序号
				tPkt.iDataLen = iMsgDataLen-2;
				strncpy(tPkt.pData,pcMsgData+2,iMsgDataLen-2);
                //tPkt.pData[iMsgDataLen-2] = 0;
				PutNodeToCmdQueue(g_tNVRNetnfo[iNvrNo].ptFileQueue, &tPkt);
			}
			
        }
        break;
	}
	case SERV_CLI_MSG_TYPE_IPC_ALARM_REPORT:
	{
        static T_IPC_WARN_STATE tIpcWarnState;
        if(iMsgDataLen >= sizeof(T_IPC_ALARM_STATUS))
        {
            memcpy(&tIpcWarnState, pcMsgData+sizeof(T_IPC_ID), 2);
            tIpcWarnState.nTime = time(NULL);
            STATE_SetIpcWarnState(iNvrNo, (PT_IPC_ID)pcMsgData, &tIpcWarnState);
        }
        else
        {

        }


		break;
	}
    case SERV_CLI_MSG_TYPE_NVR_ALARM_REPORT:
	{
        static T_NVR_WARN_STATE tNvrWarnState;
        if(iMsgDataLen >= sizeof(T_NVR_ALARM_STATUS))
        {
            memcpy(&tNvrWarnState, pcMsgData, 2);
            tNvrWarnState.nTime = time(NULL);
            STATE_SetNvrWarnState(iNvrNo, &tNvrWarnState);
        }
        else
        {

        }

		break;
	}
    }
	if (g_bSearchDataReturn==true&&ucMsgCmd!=SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP)
	{
		g_bSearchDataReturn = false;
		STATE_SetFileSearchState(E_FILE_IDLE,1);
	}
	
// 	static auto nTimeStart =GetTickCount();
// 	auto nTimeEnd =GetTickCount();
// 	if (nTimeEnd-nTimeStart>1000*2)
// 	{
// 		
// 	}

	return 0;
}

void *CmdProcessThread(void *arg)
{
    T_CMD_PACKET tPkt;
	while(g_iCmdProcessThreadRunFlag)
	{
        for(int i=0;i<s_iNvrTotalNum;i++)
		{
            T_NVR_NET_INFO *ptNvrInfo = (T_NVR_NET_INFO *) &g_tNVRNetnfo[i];

			memset(&tPkt, 0, sizeof(tPkt));
			while(GetNodeFromCmdQueue(ptNvrInfo->PtCmdQueue, &tPkt))
			{
				PMSG_SendPmsgData(
ptNvrInfo->NVRMsgHandle,tPkt.iMsgCmd,tPkt.pData,tPkt.iDataLen);
				if(tPkt.iDataLen >0)
				{
					free(tPkt.pData);
					tPkt.pData = NULL;
				}
			}
		}
#ifdef WIN32
		Sleep(30);
#else
		usleep(30000);
#endif
		
	}
	
	return NULL;
}

int NVR_init(int iNvrNum)
{
	g_iCmdProcessThreadRunFlag = 1;
	PMSG_Init();
	memset(g_tNVRNetnfo,0,sizeof(g_tNVRNetnfo));

    s_iNvrTotalNum = iNvrNum;
    for(int i=0;i<s_iNvrTotalNum;i++)
	{
		NVR_Connect(i);
	}
#ifdef _WIN32
   g_cmdProcessThreadHandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)CmdProcessThread,(void *)NULL,0,NULL);
#else
    pthread_create(&g_cmdProcessThreadHandle, NULL, CmdProcessThread, NULL);
#endif

	return 0;

}

int NVR_Uninit()
{
	g_iCmdProcessThreadRunFlag =0;
#ifdef _WIN32
   ExitThread((DWORD)g_cmdProcessThreadHandle);
#else
   pthread_join(g_cmdProcessThreadHandle,NULL);
#endif
	PMSG_Uninit();
	return 0;
}
int NVR_Connect(int iNvrNo)
{
    char acIp[32] = {0};
    if(iNvrNo < 0 || iNvrNo >= s_iNvrTotalNum || g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
	}

	memset(acIp, 0, sizeof(acIp));
    STATE_GetNvrIpAddr(iNvrNo, acIp);
	if(acIp[0] == 0)
	{
		return -1;
	}
	g_tNVRNetnfo[iNvrNo].iThreadRun = 1;
	g_tNVRNetnfo[iNvrNo].PtCmdQueue = CreateCmdQueue();
	g_tNVRNetnfo[iNvrNo].ptFileQueue = CreateCmdQueue();
	g_tNVRNetnfo[iNvrNo].NVRMsgHandle = PMSG_CreateConnect(acIp,NVR_PORT,PmsgProc,NULL);
	return 0;
}

int NVR_DisConnect(int iNvrNo)
{
    if(iNvrNo <0 || iNvrNo >=s_iNvrTotalNum || 0 ==  g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
	}
		
	g_tNVRNetnfo[iNvrNo].iThreadRun = 0;
	//PMSG_DestroyConnect(g_tNVRNetnfo[iNvrNo].NVRMsgHandle);
	g_tNVRNetnfo[iNvrNo].NVRMsgHandle = 0;
	DestroyCmdQueue(g_tNVRNetnfo[iNvrNo].PtCmdQueue);
	g_tNVRNetnfo[iNvrNo].PtCmdQueue = NULL;
	return 0;
}

int NVR_SendCmdInfo(int iNvrNo,int iCmd,char *pData,int iDataLen)
{
    if(iNvrNo <0 || iNvrNo >= s_iNvrTotalNum ||
		0 == g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
    }

    if(E_SERV_STATUS_CONNECT != NVR_GetConnectStatus(iNvrNo))
    {
        return -1;
    }

    T_CMD_PACKET tPkt;
	memset(&tPkt, 0, sizeof(tPkt));
	tPkt.iMsgCmd = iCmd;
	if (iDataLen > 0)
	{
		tPkt.pData = (char *)malloc(iDataLen+1);
		memcpy(tPkt.pData,pData,iDataLen);
		tPkt.iDataLen = iDataLen;
	}
	PutNodeToCmdQueue(g_tNVRNetnfo[iNvrNo].PtCmdQueue, &tPkt);
	return 0;
}

int NVR_GetFileInfo(int iNvrNo,T_CMD_PACKET *ptPkt)
{
    if(iNvrNo <0 || iNvrNo >=s_iNvrTotalNum || 0 ==  g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
	}
	if(GetNodeFromCmdQueue(g_tNVRNetnfo[iNvrNo].ptFileQueue, ptPkt))
	{
		return 1;
	}
	
	return 0;
}

int NVR_CleanFileInfo(int iNvrNo)
{
    if(iNvrNo <0 || iNvrNo >=s_iNvrTotalNum || 0 ==  g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
	}
	return CleanNodeFromCmdQueue(g_tNVRNetnfo[iNvrNo].ptFileQueue);	
}

int NVR_GetFileQueueSize(int iNvrNo)
{
    if(iNvrNo <0 || iNvrNo >=s_iNvrTotalNum || 0 ==  g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
    {
        return -1;
    }
    return g_tNVRNetnfo[iNvrNo].ptFileQueue->iPktCount;
}

int NVR_GetConnectStatus(int iNvrNo)
{
    if(iNvrNo <0 || iNvrNo >=s_iNvrTotalNum || 0 ==  g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return E_SERV_STATUS_UNCONNECT;
	}
	return PMSG_GetConnectStatus(g_tNVRNetnfo[iNvrNo].NVRMsgHandle);
}
