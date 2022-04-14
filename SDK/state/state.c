#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tchar.h>
#include <signal.h>
#ifdef WIN32

#include<windows.h>
#include<process.h>
#else
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#include <time.h>
#include "fileConfig.h"
#include "state.h"

//#include <QMutex>
//声明CRITICAL_SECTION 临界区变量


//static QMutex	  g_qmNVRInfoMutex;  
static T_NVR_INFO g_atNVRInfo[NVR_MAX_NUM];
static int 		  g_iNvrTotalNum = 0;
static int 		  g_iCarriageTotalNum = 0;
static char		g_acPecuVideoIdx[24];
static char		g_acFireVideoIdx[6];
static char		g_acDoorVideoIdx[48];
static char		g_cResUpdateState;      //资源的更新状态 0:未连接 1:开始更新 2：更新中 3：更新成功 4：更新失败
static char		g_cResUpdateProgress;	//素材更新进度
static char		g_acCCTVRunDir[256] = {0};
static int 		g_iDispState = DISP_STATE_UNKOWN;
//static char     g_acConfigFileDir[]="/home/adv/works/test";
static char		g_acBroadcastConfFile[]="BroadcastInfoConfig.ini";
static char		g_acCCTVConfigFile[]="CCTVConfig.ini";
static char     g_acIpAddr[16] = {0};
static STATUSCHANGECALLBACK g_pIPCStatusChangeCallBack=NULL;
static SEARCHOVERCALLBACK   g_pSearchOverCallBack= NULL;
static NVRDISCONNECTCALLBACK g_pNVRDisconnectBack= NULL;
void  *           g_pStatusChangeUserData = NULL;
void  *           g_pSearchOverUserData= NULL;
void  *           g_pNVRDisconnectUserData = NULL;
static E_FILE_SEARCH_STATE g_eFileSearch = E_FILE_IDLE;
static E_FILE_DOWN_STATE g_eFileDownState = E_FILE_DOWN_IDLE;
static int g_iFileProgress = 0;


static int PutDataList(T_DATA_LIST *pDataList, void *pData)
{
    T_DATA_LIST *pList = pDataList;
    while (pList)
    {
        if(NULL == pList->pData)
        {
            pList->pData = pData;
            break;
        }
        else if(NULL == pList->pLast)
        {
            pList->pLast = (T_DATA_LIST*)malloc(sizeof(T_DATA_LIST));
            pList->pLast->pData = pData;
            pList->pLast->pLast = NULL;
            break;
        }
        pList = pList->pLast;
    }
    return 0;
}

static int AddUserPasswordToRtsp(char *pRtsp,char *pRtspDst,int DstLen, char *pUser,char *pPassword)
{
	char * pstr1 = NULL;
  
    pstr1 = strstr(pRtsp, "://");
    if ( NULL == pstr1 ) 
    {
        return -1;
    }

	if( (0 == strlen(pUser)) || (0 == strlen(pPassword)))
	{
		_snprintf(pRtspDst,DstLen-1,"rtsp://%s",(char *)(pstr1+3));
	}
	else
	{
		_snprintf(pRtspDst,DstLen-1,"rtsp://%s:%s@%s",pUser,pPassword,(char *)(pstr1+3));
	}

   return 0;
}

static int ReadNvrAndIpc()
{
	int i = 0,j = 0;
	int iRet = 0;
	int iIPCNum = 0;
	char acKeyValue[32] = {0};
	char acParseStr[256] = {0};
	wchar_t acTestParseStr[256] = {0};

	char acNvrGroupValue[32] = {0};
	wchar_t acTest[32]={0};
	char acIpcGroupValue[32] = {0};
	char acIniPath[256] ={0};
	wchar_t acTestIniPath[256]={0};

	T_NVR_INFO *pNvrInfo = NULL;
	T_IPC_INFO *pIPCInfo = NULL;

	g_iNvrTotalNum = 0;
	memset(g_atNVRInfo, 0, sizeof(g_atNVRInfo));

    _snprintf(acIniPath,sizeof(acIniPath)-1,"%s/%s",g_acCCTVRunDir,g_acCCTVConfigFile);
    _snprintf(acTestIniPath,sizeof(acTestIniPath)-1,"%s/%s",g_acCCTVRunDir,g_acCCTVConfigFile);
    
	iRet =GetPrivateProfileStringA("Base", "NVRNum", (""), acParseStr, 256, acIniPath);
	//ReadParam(acIniPath, "[Base]", "CarriageNum", acParseStr);
	if(iRet > 0)
	{
		g_iNvrTotalNum = atoi(acParseStr);
	}
	if(g_iNvrTotalNum < 1)
	{
		g_iNvrTotalNum = 1;
	}
	if(g_iNvrTotalNum > NVR_MAX_NUM)
	{
		g_iNvrTotalNum = NVR_MAX_NUM;
	}

	iRet =GetPrivateProfileStringA("Base", "CarriageNum", (""), acParseStr, 256, acIniPath);
	//ReadParam(acIniPath, "[Base]", "CarriageNum", acParseStr);
	if(iRet > 0)
	{
		g_iCarriageTotalNum = atoi(acParseStr);
	}
	
	if(g_iCarriageTotalNum < 1)
	{
		g_iCarriageTotalNum = 1;
	}
	if(g_iCarriageTotalNum > NVR_MAX_NUM)
	{
		g_iCarriageTotalNum = NVR_MAX_NUM;
	}
	
	
	for(i=0;i<g_iNvrTotalNum;i++)
	{
		pNvrInfo = &g_atNVRInfo[i];
		memset(acParseStr,0,sizeof(acParseStr));
		memset(acKeyValue,0,sizeof(acKeyValue));
		memset(acNvrGroupValue,0,sizeof(acNvrGroupValue));
		memset(acIpcGroupValue,0,sizeof(acIpcGroupValue));
        sprintf(acNvrGroupValue,"NVR%d",i+1);
		pNvrInfo->iNo = i;
		//iRet = ReadParam(acIniPath, acNvrGroupValue, "IP", acParseStr);
		iRet =GetPrivateProfileStringA(acNvrGroupValue, "IP", (""), acParseStr, 256, acIniPath);
		if (iRet > 0)
		{
			strncpy(pNvrInfo->acIp, acParseStr, sizeof(pNvrInfo->acIp));
		}
		
        memset(acParseStr,0,sizeof(acParseStr));
		//iRet = ReadParam(acIniPath, acNvrGroupValue, "UserName", acParseStr);
		iRet =GetPrivateProfileStringA(acNvrGroupValue, "UserName", (""), acParseStr, 256, acIniPath);
		if (iRet > 0)
		{
			strncpy(pNvrInfo->tUserLogin.acUserName, acParseStr, 31);
		}
        memset(acParseStr,0,sizeof(acParseStr));
		iRet =GetPrivateProfileStringA(acNvrGroupValue, "Password", (""), acParseStr, 256, acIniPath);
		//iRet = ReadParam(acIniPath, acNvrGroupValue, "Password", acParseStr);
		if (iRet > 0)
		{
			strncpy(pNvrInfo->tUserLogin.acPassword, acParseStr, 31);
		}
		memset(acParseStr,0,sizeof(acParseStr));
		iRet =GetPrivateProfileStringA(acNvrGroupValue, "NVRName", (""), acParseStr, 256, acIniPath);
	
		//NVR name
		if (iRet > 0)
		{
			strncpy(pNvrInfo->acNVRName, acParseStr, 255);
		}
		
		iIPCNum = 0;
        memset(acParseStr,0,sizeof(acParseStr));
		//iRet = ReadParam(acIniPath, acNvrGroupValue, "IPCNum", acParseStr);
		iRet =GetPrivateProfileStringA(acNvrGroupValue, "IPCNum", (""), acParseStr, 256, acIniPath);
		if (iRet > 0)
		{
			iIPCNum = atoi(acParseStr);
		}
		if(iIPCNum <= 0)
		{
            iIPCNum = NVR_IPC_MAX_NUM;
		}
		if(iIPCNum > NVR_IPC_MAX_NUM)
		{
			iIPCNum = NVR_IPC_MAX_NUM;
		}
		pNvrInfo->iIPCNum = iIPCNum;
		for(j = 0; j < iIPCNum; j++)
		{
			pIPCInfo = &pNvrInfo->atIpcInfo[j];
			
            sprintf(acIpcGroupValue, "NVR%d_IPC%d", i+1, j+1);
            memset(acParseStr,0,sizeof(acParseStr));
			//iRet = ReadParam(acIniPath, acIpcGroupValue, "Pos", acParseStr);
			iRet =GetPrivateProfileStringA(acIpcGroupValue, "Pos", (""), acParseStr, 256, acIniPath);
			if (iRet > 0)
			{
				pIPCInfo->cPos = atoi(acParseStr);
			}
			if (pIPCInfo->cPos > 4)
			{
				pIPCInfo->cPos= 4;
			}
			if (pIPCInfo->cPos < 1)
			{
				pIPCInfo->cPos= 1;
			}
            memset(acParseStr,0,sizeof(acParseStr));
			//iRet = ReadParam(acIniPath, acIpcGroupValue, "CarriageNo", acParseStr);
			iRet =GetPrivateProfileStringA(acIpcGroupValue, "CarriageNo", (""), acParseStr, 256, acIniPath);
			if (iRet > 0)
			{
				pIPCInfo->cCarriageNo = atoi(acParseStr);
			}
            memset(acParseStr,0,sizeof(acParseStr));
			//iRet = ReadParam(acIniPath, acIpcGroupValue, "ChannelName", acParseStr);
			iRet =GetPrivateProfileStringA(acIpcGroupValue, "ChannelName", (""), acParseStr, 256, acIniPath);
			if (iRet > 0)
			{
                strncpy(pIPCInfo->acChannelName, acParseStr, sizeof(pIPCInfo->acChannelName)-1);
			}

            memset(acParseStr,0,sizeof(acParseStr));
            //iRet = ReadParam(acIniPath, acIpcGroupValue, "rtspMaster", acParseStr);
			iRet =GetPrivateProfileStringA(acIpcGroupValue, "rtspMaster", (""), acParseStr, 256, acIniPath);
            if (iRet > 0)
            {
                strncpy(pIPCInfo->acRtspMaster, acParseStr, sizeof(pIPCInfo->acRtspMaster)-1);
            }

            memset(acParseStr,0,sizeof(acParseStr));
            //iRet = ReadParam(acIniPath, acIpcGroupValue, "rtspSlave", acParseStr);
			iRet =GetPrivateProfileStringA(acIpcGroupValue, "rtspSlave", (""), acParseStr, 256, acIniPath);
            if (iRet > 0)
            {
                strncpy(pIPCInfo->acRtspSlave, acParseStr, sizeof(pIPCInfo->acRtspSlave)-1);
            }
			//IP
			 memset(acParseStr,0,sizeof(acParseStr));
			iRet =GetPrivateProfileStringA(acIpcGroupValue, "IP", (""), acParseStr, 32, acIniPath);
			if (iRet > 0)
			{
				strncpy(pIPCInfo->acIp, acParseStr, sizeof(pIPCInfo->acIp)-1);
			}
			pIPCInfo->bConf= 1;
		}		
	}	
	
	
	g_atNVRInfo;
	return 0;
}

int STATE_Init(void)
{
	int iRet = -1;
	iRet = ReadNvrAndIpc();
	return iRet;
}

int STATE_Uninit(void)
{
    int i = 0;
    for(i = 0; i < g_iNvrTotalNum; i++)
    {
        //g_atNVRInfo[i].tWarnStateList
    }

    return 0;
}


//设置CCTV的运行目录
int STATE_SetCCTVRunDir(const char *pcData,int iLen)
{
	memset(g_acCCTVRunDir,0,sizeof(g_acCCTVRunDir));
	strncpy(g_acCCTVRunDir,pcData,iLen);
	return 0;
}
//获取CCTV的运行目录
int STATE_GetCCTVRunDir(char *pcRunData,int iLen)
{
	strncpy(pcRunData,g_acCCTVRunDir,iLen);
	return 0;
}
int STATE_CleanIPCState(int iNvrNo)
{
	T_NVR_INFO *pNvrInfo = NULL;
	T_IPC_INFO *pIpcInfo = NULL;
	int i = 0;
	for (i;i<g_iNvrTotalNum;i++)
	{
		pNvrInfo =  &g_atNVRInfo[i];
		if (iNvrNo==pNvrInfo->iNo)
		{
			for(i = 0; i < pNvrInfo->iIPCNum; i++)
			{
				pIpcInfo = &pNvrInfo->atIpcInfo[i];
				pIpcInfo->tState.cOnlineState='0';
			}
		}
	}
	if (g_pNVRDisconnectBack)
	{
		g_pNVRDisconnectBack(1,iNvrNo,g_pNVRDisconnectUserData);
	}
	return 0;
}
int STATE_NVROnline(int iNvrNo)
{
	if (g_pNVRDisconnectBack)
	{
		g_pNVRDisconnectBack(0,iNvrNo,g_pNVRDisconnectUserData);
	}
	return 0;
}
// IPC
int STATE_SetIpcState(int iNvrNo, PT_IPC_ID ptId, T_IPC_STATE *ptState)
{
	//g_qmNVRInfoMutex.lock();
	
	int i = 0;
	T_NVR_INFO *pNvrInfo = NULL;
	T_IPC_INFO *pIpcInfo = NULL;
	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		//g_qmNVRInfoMutex.unlock();
         return -1;
    }
    ptState->sVersion = ntohs(ptState->sVersion);
	pNvrInfo = &g_atNVRInfo[iNvrNo];
	for(i = 0; i < pNvrInfo->iIPCNum; i++)
	{
		pIpcInfo = &pNvrInfo->atIpcInfo[i];
		if (ptId->cCarriageNo==2)
		{
			printf("");
		}
		if(ptId->cCarriageNo == pIpcInfo->cCarriageNo && 
			ptId->cPos == pIpcInfo->cPos)
			{
				int nNotice = 0;
				if (pIpcInfo->tState.cOnlineState!=ptState->cOnlineState)
				{
					nNotice =1;					
				}
				memcpy(&pIpcInfo->tState, ptState, sizeof(T_IPC_STATE));
				if (nNotice==1)
				{
					if (g_pIPCStatusChangeCallBack!=NULL&&g_pStatusChangeUserData!=NULL)
					{
						g_pIPCStatusChangeCallBack(iNvrNo,ptId,ptState,g_pStatusChangeUserData);
					}
				}
				
				//g_qmNVRInfoMutex.unlock();
				return 0;
			}
	}
	
	//g_qmNVRInfoMutex.unlock();
	return -1;
}

int STATE_GetIpcState(int iNvrNo,PT_IPC_ID ptId, T_IPC_STATE *ptState)
{
	//g_qmNVRInfoMutex.lock();
	
	int i = 0;
	T_NVR_INFO *pNvrInfo = NULL;
	T_IPC_INFO *pIpcInfo = NULL;
	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		// g_qmNVRInfoMutex.unlock();
		
         return -1;
    }
	pNvrInfo = &g_atNVRInfo[iNvrNo];
	for(i = 0; i < pNvrInfo->iIPCNum; i++)
	{
		pIpcInfo = &pNvrInfo->atIpcInfo[i];
		if(ptId->cCarriageNo == pIpcInfo->cCarriageNo && 
			ptId->cPos == pIpcInfo->cPos)
			{
				memcpy(ptState, &pIpcInfo->tState, sizeof(T_IPC_STATE));
				//g_qmNVRInfoMutex.unlock();
				
				return 0;
			}
	}
	//g_qmNVRInfoMutex.unlock();

	return -1;
}

int STATE_SetIpcWarnState(int iNvrNo,PT_IPC_ID ptId, T_IPC_WARN_STATE *ptState)
{
	//g_qmNVRInfoMutex.lock();

	int i = 0;
	T_NVR_INFO *pNvrInfo = NULL;
	T_IPC_INFO *pIpcInfo = NULL;
	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		//g_qmNVRInfoMutex.unlock();
	
         return -1;
    }
	pNvrInfo = &g_atNVRInfo[iNvrNo];
	for(i = 0; i < pNvrInfo->iIPCNum; i++)
	{
		pIpcInfo = &pNvrInfo->atIpcInfo[i];
		if(ptId->cCarriageNo == pIpcInfo->cCarriageNo && 
			ptId->cPos == pIpcInfo->cPos)
			{
                if(0 == memcmp(ptState, &pIpcInfo->tWarnState, sizeof(T_IPC_WARN_STATE)-4))
                {
					//g_qmNVRInfoMutex.unlock();
				
                    return 0;
                }
                memcpy(&pIpcInfo->tWarnState, ptState, sizeof(T_IPC_WARN_STATE));

                if(ptState->cLost || ptState->cShelter)
                {
                    T_IPC_WARN_STATE *ptWarn = (T_IPC_WARN_STATE *)malloc(sizeof(T_IPC_WARN_STATE));
                    memcpy(ptWarn, ptState, sizeof(T_IPC_WARN_STATE));
                    PutDataList(&pIpcInfo->tWarnStateList, ptWarn);
                }
				//g_qmNVRInfoMutex.unlock();
				
				return 0;
			}
	}
	//g_qmNVRInfoMutex.unlock();

	return -1;
}

int STATE_GetIpcWarnState(int iNvrNo,PT_IPC_ID ptId, T_IPC_WARN_STATE *ptState)
{
	//g_qmNVRInfoMutex.lock();

	int i = 0;
	T_NVR_INFO *pNvrInfo = NULL;
	T_IPC_INFO *pIpcInfo = NULL;
	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		////g_qmNVRInfoMutex.unlock();
	
         return -1;
    }
	pNvrInfo = &g_atNVRInfo[iNvrNo];
	for(i = 0; i < pNvrInfo->iIPCNum; i++)
	{
		pIpcInfo = &pNvrInfo->atIpcInfo[i];
		if(ptId->cCarriageNo == pIpcInfo->cCarriageNo && 
			ptId->cPos == pIpcInfo->cPos)
			{
				memcpy(ptState, &pIpcInfo->tState, sizeof(T_IPC_WARN_STATE));
				//g_qmNVRInfoMutex.unlock();
				
				return 0;
			}
	}
	//g_qmNVRInfoMutex.unlock();
	 
	return -1;
}

int STATE_SetIpcAttribute(int iNvrNo, PT_IPC_ID ptId, T_IPC_ATTRIBUTE_INFO *ptState)
{
	//g_qmNVRInfoMutex.lock();

	int i = 0;
	T_NVR_INFO *pNvrInfo = NULL;
	T_IPC_INFO *pIpcInfo = NULL;
	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		//g_qmNVRInfoMutex.unlock();
	
         return -1;
    }
	pNvrInfo = &g_atNVRInfo[iNvrNo];
	for(i = 0; i < pNvrInfo->iIPCNum; i++)
	{
		pIpcInfo = &pNvrInfo->atIpcInfo[i];
		if(ptId->cCarriageNo == pIpcInfo->cCarriageNo && 
			ptId->cPos == pIpcInfo->cPos)
			{
				memcpy(&pIpcInfo->tAttribute, ptState, sizeof(T_IPC_ATTRIBUTE_INFO));
				//g_qmNVRInfoMutex.unlock();.
				
				return 0;
			}
	}
	//g_qmNVRInfoMutex.unlock();
	
	return -1;
}

int STATE_GetIpcAttribute(int iNvrNo, PT_IPC_ID ptId, T_IPC_ATTRIBUTE_INFO *ptState)
{
	//g_qmNVRInfoMutex.lock();

	int i = 0;
	T_NVR_INFO *pNvrInfo = NULL;
	T_IPC_INFO *pIpcInfo = NULL;
	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		//g_qmNVRInfoMutex.unlock();
	
         return -1;
    }
	pNvrInfo = &g_atNVRInfo[iNvrNo];
	for(i = 0; i < pNvrInfo->iIPCNum; i++)
	{
		pIpcInfo = &pNvrInfo->atIpcInfo[i];
		if(ptId->cCarriageNo == pIpcInfo->cCarriageNo && 
			ptId->cPos == pIpcInfo->cPos)
			{
				memcpy(ptState, &pIpcInfo->tAttribute, sizeof(T_IPC_ATTRIBUTE_INFO));
				//g_qmNVRInfoMutex.unlock();
				
				return 0;
			}
	}
	//g_qmNVRInfoMutex.unlock();
	
	return -1;
}

int STATE_GeNvrNoByCarriageNo(char cCarriageNo)
{
	int i = 0, j = 0;
	for(i = 0; i < NVR_MAX_NUM; i++)
	{
		for(j = 0; j < NVR_IPC_MAX_NUM; j++)
		{
			if(cCarriageNo == g_atNVRInfo[i].atIpcInfo[j].cCarriageNo)
			{
				return i;
			}
		}		
	}
	return -1;
}

int STATE_GetAllIpcPosCarriageNo(char cCarriageNo, char *pacPos, int nMax)
{
	//g_qmNVRInfoMutex.lock();
	
	int nIndex = 0;
	int i = 0, j = 0;
	for(i = 0; i < NVR_MAX_NUM; i++)
	{
		for(j = 0; j < NVR_IPC_MAX_NUM; j++)
		{
			if(cCarriageNo == g_atNVRInfo[i].atIpcInfo[j].cCarriageNo&&g_atNVRInfo[i].atIpcInfo[j].bConf==1)
			{
				if(nIndex >= nMax)
				{
					//g_qmNVRInfoMutex.unlock();
					
					return nIndex;
				}
				pacPos[nIndex] = g_atNVRInfo[i].atIpcInfo[j].cPos;
				nIndex++;
			}
		}		
	}
	//g_qmNVRInfoMutex.unlock();
	
	return nIndex;
}

int STATE_GetIpcRtsp(char cCarriageNo,char cPos,char *rtsp, int nMaster)
{
	//g_qmNVRInfoMutex.lock();
	
    int i = 0, j = 0;
    for(i = 0; i < NVR_MAX_NUM; i++)
    {
        for(j = 0; j < NVR_IPC_MAX_NUM; j++)
        {
            if((cCarriageNo == g_atNVRInfo[i].atIpcInfo[j].cCarriageNo) && (cPos == g_atNVRInfo[i].atIpcInfo[j].cPos))
            {
                if(0 == nMaster)
                {
                    strcpy(rtsp, g_atNVRInfo[i].atIpcInfo[j].acRtspMaster);
					//g_qmNVRInfoMutex.unlock();
					
                    return 0;
                }
                if(1 == nMaster)
                {
                    strcpy(rtsp, g_atNVRInfo[i].atIpcInfo[j].acRtspSlave);
					//g_qmNVRInfoMutex.unlock();
					
                    return 0;
                }
            }
        }
    }
    return -1;
}

// Carriage
int STATE_GetCarriageNum()
{
    return g_iCarriageTotalNum;
}

// Nvr

int STATE_GetNvrNum()
{
    return g_iNvrTotalNum;
}

int STATE_GetNvrIpAddr(int iNvrNo, char *pIp)
{
	//g_qmNVRInfoMutex.lock();
	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		//g_qmNVRInfoMutex.unlock();
		
         return -1;
    }
	memcpy(pIp, g_atNVRInfo[iNvrNo].acIp, sizeof(g_atNVRInfo[iNvrNo].acIp));
	//g_qmNVRInfoMutex.unlock();
	
	return 0;
}

int STATE_GetNvrInfo(int iNvrNo,T_NVR_INFO *ptNvrInfo)
{
    if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
         return -1;
    }
    memcpy(ptNvrInfo, &g_atNVRInfo[iNvrNo], sizeof(T_NVR_INFO));
    return 0;
}

int STATE_SetNvrState(int iNvrNo,T_NVR_STATE *ptState)
{
	//g_qmNVRInfoMutex.lock();

	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		//g_qmNVRInfoMutex.unlock();
		 
         return -1;
    }
    ptState->sVersion = ntohs(ptState->sVersion);
    ptState->sHdiskSize = ntohs(ptState->sHdiskSize);
    ptState->sHdiskUseSize = ntohs(ptState->sHdiskUseSize);
	memcpy(&g_atNVRInfo[iNvrNo].tState, ptState, sizeof(T_NVR_STATE));
	//g_qmNVRInfoMutex.unlock();
	
	return 0;
}

int STATE_GetNvrState(int iNvrNo,T_NVR_STATE *ptState)
{
	//g_qmNVRInfoMutex.lock();
	
	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		//g_qmNVRInfoMutex.unlock();
		
         return -1;
    }
	memcpy(ptState, &g_atNVRInfo[iNvrNo].tState, sizeof(T_NVR_STATE));
	//g_qmNVRInfoMutex.unlock();
	
	return 0;
}

int STATE_SetNvrWarnState(int iNvrNo,T_NVR_WARN_STATE *ptState)
{
	//g_qmNVRInfoMutex.lock();

	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
		//g_qmNVRInfoMutex.unlock();

         return -1;
    }
    if(0 == memcmp(ptState, &g_atNVRInfo[iNvrNo].tWarnState, sizeof(T_NVR_WARN_STATE))-4)
    {
		//g_qmNVRInfoMutex.unlock();

        return 0;
    }
	memcpy(&g_atNVRInfo[iNvrNo].tWarnState, ptState, sizeof(T_NVR_WARN_STATE));


    if(ptState->cHdiskBad || ptState->cHdiskLost)
    {
        T_NVR_WARN_STATE *ptWarn = (T_NVR_WARN_STATE *)malloc(sizeof(T_NVR_WARN_STATE));
        memcpy(ptWarn, ptState, sizeof(T_NVR_WARN_STATE));
        PutDataList(&g_atNVRInfo[iNvrNo].tWarnStateList, ptWarn);
    }
	//g_qmNVRInfoMutex.unlock();

    return 0;
}

int STATE_GetNvrWarnState(int iNvrNo,T_NVR_WARN_STATE *ptState)
{
	//g_qmNVRInfoMutex.lock();

	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {

		//g_qmNVRInfoMutex.unlock();
         return -1;
    }    
    memcpy(ptState, &g_atNVRInfo[iNvrNo].tWarnState, sizeof(T_NVR_WARN_STATE));
	//g_qmNVRInfoMutex.unlock();

	return 0;
}

//获取Nvr相关的相机数量
int STATE_GetNvrIpcNum(int iNvrNo)
{
	if (iNvrNo < 0 || iNvrNo >= g_iNvrTotalNum)
    {
         return -1;
    }
	return g_atNVRInfo[iNvrNo].iIPCNum;
}

int STATE_SetFileSearchState(E_FILE_SEARCH_STATE iState,int iNotice)
{
    g_eFileSearch = iState;
	if (iNotice!=0)
	{
		if (g_pSearchOverCallBack&&g_pSearchOverUserData)
		{
			g_pSearchOverCallBack(g_pSearchOverUserData);
		}
		
	}
    return 0;
}


E_FILE_SEARCH_STATE STATE_GetFileSearchState()
{
    return g_eFileSearch;
}
int STATE_SetFileDownState(E_FILE_DOWN_STATE eState)  //0 空闲  1 正在进行  2 已完成
{
    g_eFileDownState = eState;
    return 0;
}
E_FILE_DOWN_STATE STATE_GetFileDownState( )
{
    return g_eFileDownState;
}

int STATE_SetFileDownProgress(int iProgress)
{
    g_iFileProgress = iProgress;
    return 0;
}
int STATE_GetFileDownProgress()
{
    return g_iFileProgress;
}
int STATE_FindUsbDev()
{
    FILE *pFile = 0;
    char acBuf[256] = {0};

    pFile = fopen("/proc/partitions", "rb");
    if (NULL == pFile)
    {
        return 0;
    }

    while (fgets(acBuf, sizeof(acBuf), pFile))
    {
        if (strstr(acBuf, "sd") != NULL)
        {
            fclose(pFile);
            return 1;
        }
    }

    fclose(pFile);
    return 0;
}
void STATE_SetIPCStatusCallback(STATUSCHANGECALLBACK callbackIPCStatusChange,void* pUserData)
{
	g_pIPCStatusChangeCallBack=(STATUSCHANGECALLBACK)callbackIPCStatusChange;
	g_pStatusChangeUserData = pUserData;
}
void STATE_SetSearchOverCallback(SEARCHOVERCALLBACK callbackSearchOver,void *pUserData)
{
	g_pSearchOverCallBack = (SEARCHOVERCALLBACK) callbackSearchOver;
	g_pSearchOverUserData= pUserData;

}

void STATE_SetNVRDisconnectCallback(NVRDISCONNECTCALLBACK callbackNVRDisconnect,void* pUserData)
{
	g_pNVRDisconnectBack = (NVRDISCONNECTCALLBACK) callbackNVRDisconnect;
	g_pNVRDisconnectUserData = pUserData;
}