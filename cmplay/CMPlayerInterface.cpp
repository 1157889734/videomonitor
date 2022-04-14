#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include <windows.h>
#include <list>
#include "CMPlayerInterface.h"
#include "mutex.h"
#include "vdec.h"
#include "debug.h"
#include <string.h>
#include <stdlib.h>
#include "rtspApi.h"
#include "rtspComm.h"
#include <atltrace.h>

enum CMP_OPEN_MEDIA_STATE
{
	CMP_OPEN_MEDIA_FAIL = 0,       
	CMP_OPEN_MEDIA_SUCC,     
	CMP_OPEN_MEDIA_LOGIN_FAIL, 
	CMP_OPEN_MEDIA_STREAM_FAIL,
	CMP_OPEN_MEDIA_PLAYCONTRO_FAIL,             
};

struct SVideoCmdMessage{
	int nType;//0 Play 1 Preview
	long lCmd;
	double dValue;
	SVideoCmdMessage(){
		nType = 1;
		lCmd = 0;
		dValue = 0;
	}

	SVideoCmdMessage(long lp1,double lp2,int np3){
		nType = np3;
		lCmd = lp1;
		dValue = lp2;
	}

	SVideoCmdMessage& operator=(SVideoCmdMessage& value)
	{
		nType = value.nType;
		lCmd = value.lCmd;
		dValue = value.dValue;
		return *this;
	}
};

#define PLAY_STREAM_TYPE_PREVIEW  0
#define PLAY_STREAM_TYPE_PLAYBACK 1

typedef struct _T_CMP_PLAYER_INFO
{
	char			acUrl[256];
	int 			iReconnectFlag;
	int				iThreadRunFlag;
	void 			*pUserInfo;
	VDEC_HADNDLE	VHandle;
	HANDLE			hPlayThread;
	CMPPLAY_STATE   ePlayState;  //播放状态
	int				iPlaySecs;           //播放了的时间（单位豪秒）
	int 			iAudioFlag;
	int				iPlayStreamType;     // 0: preview, 1: playback
	int				iTcpFlag;
	std::list<SVideoCmdMessage> lstMsg;	//
	CMutexLock		stcSecLock;	
	HWND 			hWnd;
	int				iPlayRange;			 //视频时长（单位秒）
	HANDLE			hOpenMediaEvent;
	int				iOpenMediaState;	
	int				iPlaySpeed;
	int				iStreamFlag;		//0为无流   默认有流
	timeval	   	    tPrevFrameTs;
	int 			iIgnoreFrameNum;
	int				iRtspHeartCount;
	unsigned int	uiPrevFrameTs;
	unsigned int	uiPlayBaseTime;
	CMP_VDEC_TYPE   eDecType;   //软、硬解码或者鱼眼矫正
} T_CMP_PLAYER_INFO, *PT_CMP_PLAYER_INFO;

static int				   g_iCMPlayerInitFlag = 0;

static int WinSocketInit(void)
{
	WSADATA	wsaData;
	int nRet = WSAStartup(0x202, &wsaData);
	if(nRet != 0)
	{
		WSACleanup();
		return -1;
	}	

	return 0;
}
static int WinSocketUninit(void)
{

	WSACleanup();

	return 0;
}


static void PushMessage(PT_CMP_PLAYER_INFO ptCmpPlayer, long lParm1,double dParm2,int nType = 0)
{
	if (NULL == ptCmpPlayer)
	{
		return ;
	}
	ptCmpPlayer->stcSecLock.Lock();
	SVideoCmdMessage sCmdmsg(lParm1,dParm2,nType);
	ptCmpPlayer->lstMsg.push_back(sCmdmsg);
	ptCmpPlayer->stcSecLock.Unlock();
}

static int GetMessage(PT_CMP_PLAYER_INFO ptCmpPlayer, long &lParm1,double &dParm2,int& nType)
{
	if (NULL == ptCmpPlayer)
	{
		return 0;
	}
	if(ptCmpPlayer->lstMsg.size()<1)
	{
		return 0;
	}
	ptCmpPlayer->stcSecLock.Lock();
	SVideoCmdMessage workmsg = *(ptCmpPlayer->lstMsg.begin());
	lParm1 = workmsg.lCmd;
	dParm2 = workmsg.dValue;
	nType = workmsg.nType;
	ptCmpPlayer->lstMsg.erase(ptCmpPlayer->lstMsg.begin());
	ptCmpPlayer->stcSecLock.Unlock();
	return 1;
}

static void InitMessgeList(PT_CMP_PLAYER_INFO ptCmpPlayer)
{
	if (NULL == ptCmpPlayer)
	{
		return ;
	}
	ptCmpPlayer->stcSecLock.Lock();
	ptCmpPlayer->lstMsg.clear();
	ptCmpPlayer->stcSecLock.Unlock();
}

void SetPlayState(CMPHandle hPlay, CMPPLAY_STATE ePlayState)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return ;
	}

	ptCmpPlayer->ePlayState = ePlayState;
}

int GetPlayState(CMPHandle hPlay)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return -1;
	}

	return ptCmpPlayer->ePlayState;
}

#define TIME2MSEC(x) ((x.tv_sec) *1000 + (x.tv_usec) /1000)

static int RtpSetDataCallBack(int iFrameType, int iStreamType, char *pcFrame, int iFrameLen, unsigned int uiTs, void *pUserData)
{
	int iRet = 0;
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)pUserData;

	if(NULL ==  ptCmpPlayer )	
	{
		return -1;
	}

	if((NULL == ptCmpPlayer->VHandle) && (STREAM_TYPE_VIDEO == iFrameType))
	{
		if(E_STREAM_TYPE_H265 == iStreamType)
		{
			ptCmpPlayer->VHandle = VDEC_CreateVideoDecCh(ptCmpPlayer->hWnd,1920, 1024,ptCmpPlayer->eDecType, H265_CODE_ID);
		}
		else if(E_STREAM_TYPE_H264 == iStreamType)
		{
			ptCmpPlayer->VHandle = VDEC_CreateVideoDecCh(ptCmpPlayer->hWnd,1920, 1024,ptCmpPlayer->eDecType, H264_CODE_ID);
		}

		if(ptCmpPlayer->VHandle)
		{
			VDEC_StartPlayStream(ptCmpPlayer->VHandle);
		}
		else
		{
			return -1;
		}
	}

	if (PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType) 
	{
		unsigned long ulNpt ;
		if (((ptCmpPlayer->tPrevFrameTs.tv_sec != 0) || (ptCmpPlayer->tPrevFrameTs.tv_usec != 0)))
		{
			//ulNpt = (Timestamp.tv_sec -ptCmpPlayer->tPrevFrameTs.tv_sec)*1000 + (Timestamp.tv_usec -ptCmpPlayer->tPrevFrameTs.tv_usec)/1000;

			ulNpt = TIME2MSEC(ptFrameInfo->Timestamp) - TIME2MSEC(ptCmpPlayer->tPrevFrameTs); 
			if ((ulNpt > 2000) )
			{
				DebugPrint(DEBUG_CMuiNpt_11,"[%s %d] 1uiNpt = %d iPlaySecs = %d",__FUNCTION__, __LINE__, ulNpt,ptCmpPlayer->iPlaySecs);
				if(ptFrameInfo->iFrameRate>0)
				{
					ulNpt = 1000/ptFrameInfo->iFrameRate;	
				}
				else
				{
					ulNpt = 40;
				}
			}
		}
		else 
		{
			ulNpt = 0;
		}
		ptCmpPlayer->iPlaySecs += ulNpt;
		ptCmpPlayer->tPrevFrameTs = ptFrameInfo->Timestamp;

		if (ptCmpPlayer->iPlaySecs / 1000 >= ptCmpPlayer->iPlayRange)
		{
			ptCmpPlayer->iPlaySecs = (ptCmpPlayer->iPlayRange - 1) * 1000;
		}
	}
	else if (PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType)
	{

		ptCmpPlayer->tPrevFrameTs = ptFrameInfo->Timestamp;
	}
	ptCmpPlayer->iRtspHeartCount    = 0;

	//ATLTRACE("%p, frame %d, %d, data:%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", ptCmpPlayer, iFrameLen, uiTs, pcFrame[0], pcFrame[1], pcFrame[2], pcFrame[3], pcFrame[4], pcFrame[5], pcFrame[6], pcFrame[7],
	 //                                                                            pcFrame[8], pcFrame[9], pcFrame[10], pcFrame[11], pcFrame[12], pcFrame[13], pcFrame[14], pcFrame[15]);
	VDEC_SendStream(ptCmpPlayer->VHandle, pcFrame,  iFrameLen, 0);

	return 0;
}

/*
static int RtpSetDataCallBackFun(PT_FRAME_INFO ptFrameInfo, PT_VIDEO_INFO ptVideoInfo,void *pUserData)
{
	int iRet = 0;
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)pUserData;

	if(NULL ==  ptCmpPlayer )	
	{
		return -1;
	}

	if(ptVideoInfo != NULL && (NULL == ptCmpPlayer->VHandle))
	{
		if(!strcmp(ptVideoInfo->pCodecName, "H265"))
		{
			ptCmpPlayer->VHandle = VDEC_CreateVideoDecCh(ptCmpPlayer->hWnd,ptVideoInfo->iWidth, ptVideoInfo->iHeight,ptCmpPlayer->eDecType, H265_CODE_ID);
		}
		else if(!strcmp(ptVideoInfo->pCodecName, "H264"))
		{
			ptCmpPlayer->VHandle = VDEC_CreateVideoDecCh(ptCmpPlayer->hWnd,ptVideoInfo->iWidth, ptVideoInfo->iHeight,ptCmpPlayer->eDecType, H264_CODE_ID);
		}
		else if(!strcmp(ptVideoInfo->pCodecName, "MP2T"))
		{
			ptCmpPlayer->VHandle = VDEC_CreateVideoDecCh(ptCmpPlayer->hWnd,ptVideoInfo->iWidth, ptVideoInfo->iHeight,ptCmpPlayer->eDecType, MPEG4_CODE_ID);
		}
		if(ptCmpPlayer->VHandle)
		{
			VDEC_StartPlayStream(ptCmpPlayer->VHandle);
		}
		else
		{
			return -1;
		}
	}
	
	if (PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType) 
	{
		unsigned long ulNpt ;
		if (((ptCmpPlayer->tPrevFrameTs.tv_sec != 0) || (ptCmpPlayer->tPrevFrameTs.tv_usec != 0)))
		{
			//ulNpt = (Timestamp.tv_sec -ptCmpPlayer->tPrevFrameTs.tv_sec)*1000 + (Timestamp.tv_usec -ptCmpPlayer->tPrevFrameTs.tv_usec)/1000;

			ulNpt = TIME2MSEC(ptFrameInfo->Timestamp) - TIME2MSEC(ptCmpPlayer->tPrevFrameTs); 
			if ((ulNpt > 2000) )
			{
				DebugPrint(DEBUG_CMuiNpt_11,"[%s %d] 1uiNpt = %d iPlaySecs = %d",__FUNCTION__, __LINE__, ulNpt,ptCmpPlayer->iPlaySecs);
				if(ptFrameInfo->iFrameRate>0)
				{
					ulNpt = 1000/ptFrameInfo->iFrameRate;	
				}
				else
				{
					ulNpt = 40;
				}
			}
		}
		else 
		{
			ulNpt = 0;
		}
		ptCmpPlayer->iPlaySecs += ulNpt;
		ptCmpPlayer->tPrevFrameTs = ptFrameInfo->Timestamp;

		if (ptCmpPlayer->iPlaySecs / 1000 >= ptCmpPlayer->iPlayRange)
		{
			ptCmpPlayer->iPlaySecs = (ptCmpPlayer->iPlayRange - 1) * 1000;
		}
	}
	else if (PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType)
	{

		ptCmpPlayer->tPrevFrameTs = ptFrameInfo->Timestamp;
	}
	ptCmpPlayer->iRtspHeartCount    = 0;
	
	VDEC_SendStream(ptCmpPlayer->VHandle, ptFrameInfo->pcFrame,  ptFrameInfo->iFrameLen, 0);

	return 0;
}
*/
static int ParseRtspUrl(char *pcRawUrl, char *pcUrl, char *pcUser, char *pcPasswd)
{
	char acStr[256];
	char *pcTmp = NULL;
	char *pcNextTmp = NULL;
	char *pcPos = NULL;
	char *pcContent = NULL;
	char *pcIpaddr = NULL;
	int iPos = 0;


	if ((NULL == pcRawUrl) || (NULL == pcUrl) || (NULL == pcUser) || (NULL == pcUser))
	{
		return -1;	
	}
	memset(acStr, 0, sizeof(acStr));
	strncpy(acStr, pcRawUrl, sizeof(acStr));

	/* rtsp:// */
	
	pcContent = acStr + 7;
	pcTmp = strtok_s(pcContent, "/",&pcNextTmp);

	if (pcTmp)
	{
		pcIpaddr = pcTmp;
		pcContent = pcNextTmp;
		if(strstr(pcIpaddr, "@"))
		{
			pcTmp = strtok_s(pcIpaddr, "@",&pcNextTmp);
			pcIpaddr = pcNextTmp;
		}
		else
		{
			pcTmp = NULL;	
		}
	}

	if (pcTmp)
	{
		pcPos = pcTmp;
		pcTmp = strtok_s(pcPos, ":",&pcNextTmp);

		if (pcTmp)
		{
			strcpy(pcUser, pcTmp);
		}

		if (pcNextTmp)
		{
			strcpy(pcPasswd, pcNextTmp);
		}

	}

	if (NULL == pcIpaddr)
	{
		return -1;	
	}

	if (pcContent)
	{
		sprintf_s(pcUrl, 256, "rtsp://%s/%s", pcIpaddr, pcContent);
	}
	else
	{
		sprintf_s(pcUrl, 256, "rtsp://%s", pcIpaddr);
	}


	return 0;
}

DWORD WINAPI MonitorPlayThread(void *arg)
{

	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)arg;
	RTSP_HANDLE RHandle = 0;
	int iTimeout = 0;
	int iRet = 0;
	char acUrl[256]={0};
	char acUser[32]="admin";
	char acPwd[32]="12345";
	int iReconnectFlag = 0;
	int iRunCount = 0;
	int iReconnectCount = 1;
	int *piThreadRunFlag = NULL;
	void *pUserData = NULL;
	//static unsigned long framecnt = 0;
	long lCmd = 0;
	double dValue = 0;
	int iType = 0;
	int iRtpProtocol = ptCmpPlayer->iTcpFlag;

	if (NULL == arg)
	{
		return NULL;
	}

	iReconnectFlag = ptCmpPlayer->iReconnectFlag;
	ParseRtspUrl(ptCmpPlayer->acUrl,acUrl,acUser,acPwd);

	//ATLTRACE("%p,url %s\n", ptCmpPlayer, ptCmpPlayer->acUrl);
	do{
		if (RHandle)
		{
			iRet = RTSP_CloseStream(RHandle, 1);
			iRet = RTSP_Logout(RHandle);
			RHandle = 0;
			Sleep(100);
			
		}

		RHandle = RTSP_Login(acUrl, acUser, acPwd);
		if (RHandle > 0)
		{
			iRet =  RTSP_OpenStream(RHandle, 0,  iRtpProtocol, RtpSetDataCallBack, ptCmpPlayer);
			if (iRet < 0)
			{
				RTSP_Logout(RHandle);
				RHandle = 0;

                if(PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType &&  ptCmpPlayer->hOpenMediaEvent)
                {
					DebugPrint(DEBUG_CMPLAY_10,"%s RTSP_OpenStream Fail",ptCmpPlayer->acUrl);
                    ptCmpPlayer->iOpenMediaState = CMP_OPEN_MEDIA_STREAM_FAIL;
                    SetEvent(ptCmpPlayer->hOpenMediaEvent);
                    continue;
                }
                Sleep(1000);
				continue;
			}
			RTSP_GetParam(RHandle, E_TYPE_PLAY_RANGE, (void *)&ptCmpPlayer->iPlayRange);
			DebugPrint(DEBUG_CMPLAY_10,"[%s %d] iPlayRange1 = %d",__FUNCTION__, __LINE__, ptCmpPlayer->iPlayRange);
			
			Sleep(10);

			iRet =  RTSP_PlayControl(RHandle, E_PLAY_STATE_PLAY, 0);
			if (iRet < 0)
			{
                if(PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType &&  ptCmpPlayer->hOpenMediaEvent)
                {
                    ptCmpPlayer->iOpenMediaState = CMP_OPEN_MEDIA_PLAYCONTRO_FAIL;
					DebugPrint(DEBUG_CMPLAY_10,"%s RTSP_PlayControl Fail",ptCmpPlayer->acUrl);
                    SetEvent(ptCmpPlayer->hOpenMediaEvent);
                }
				continue;
			}
			SetPlayState(ptCmpPlayer, CMP_STATE_PLAY);
			iTimeout = RTSP_GetRtspTimeout(RHandle);
			if (0 == iTimeout)
			{
				iTimeout = 60;	
			}
			iRunCount = iTimeout * 5;
            if(PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType &&  ptCmpPlayer->hOpenMediaEvent)
            {
                ptCmpPlayer->iOpenMediaState = CMP_OPEN_MEDIA_SUCC;
                SetEvent(ptCmpPlayer->hOpenMediaEvent);
            }
		}
		else
		{
            if(PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType &&  ptCmpPlayer->hOpenMediaEvent)
            {
                ptCmpPlayer->iOpenMediaState = CMP_OPEN_MEDIA_LOGIN_FAIL;
				DebugPrint(DEBUG_CMPLAY_10,"%s RTSP_Login Fail",ptCmpPlayer->acUrl);
                SetEvent(ptCmpPlayer->hOpenMediaEvent);
                continue;
            }
			Sleep(100);
			continue;
		}
		/*
		while (ptCmpPlayer->iThreadRunFlag)
		{
			iRet = GetMessage(ptCmpPlayer, lCmd, dValue, iType);
			if (iRet > 0)
			{
				RTSP_PlayControl(RHandle, lCmd, dValue);
			}

            if (TCP == iRtpProtocol)
            {
                iRet = RTSP_GetConnect(RHandle);
                if (iRet < 0)
                {
                    break;	
                }
            }
            else if (0 == iRunCount)
            {
                // rtp protocl is udp
                iRunCount = iTimeout * 5;
                iRet = RTSP_SendHeart(RHandle);
                if (iRet < 0)
                {
					DebugPrint(DEBUG_CMPLAY_10,"send heart failed %s",ptCmpPlayer->acUrl);	
                    break;
                }
				DebugPrint(DEBUG_CMPLAY_10,"send heart Succ iRunCount = %d iTimeout = %d File = %s",iRunCount,iTimeout,ptCmpPlayer->acUrl);
            }
			 if(PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType && ptCmpPlayer->ePlayState != CMP_STATE_PAUSE)
			 {
				 ptCmpPlayer->iRtspHeartCount++;
				 if (ptCmpPlayer->iPlayRange - ptCmpPlayer->iPlaySecs /1000 > 10)
				 {
					 // 
					 if (ptCmpPlayer->iRtspHeartCount > 100)
					 {
						 DebugPrint(DEBUG_CMPLAY_10,"iRtspHeartCount1 = %d File = %s",ptCmpPlayer->iRtspHeartCount,ptCmpPlayer->acUrl);
						 break;
					 }
				 }
				 else
				 {
					 // 文件播放快要结束
					 if (ptCmpPlayer->iRtspHeartCount > 20)
					 {
						 DebugPrint(DEBUG_CMPLAY_10,"iRtspHeartCount2 = %d File = %s",ptCmpPlayer->iRtspHeartCount,ptCmpPlayer->acUrl);
						 break;
					 }
				 }
			 }
			
            iRunCount--;
            Sleep(100);

		}

	}while (ptCmpPlayer->iThreadRunFlag && iReconnectFlag);
    
	*/
	if (RHandle)
	{

		iRet = RTSP_CloseStream(RHandle, 1);

		iRet = RTSP_Logout(RHandle);

		RHandle = 0;
	}

	if(ptCmpPlayer->VHandle)
	{
		VDEC_DestroyVideoDecCh(ptCmpPlayer->VHandle);
		ptCmpPlayer->VHandle = NULL;
	}
	DebugPrint(DEBUG_CMPLAY_10,"SetPlayState Stop File = %s",ptCmpPlayer->acUrl);
    SetPlayState(ptCmpPlayer, CMP_STATE_STOP);
    ptCmpPlayer->iPlaySecs = ptCmpPlayer->iPlayRange*1000;

	return NULL;

}

CMPPlayer_API CMPHandle CMP_Init(HWND hWnd, CMP_VDEC_TYPE eDecType)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = NULL;
	ptCmpPlayer = new T_CMP_PLAYER_INFO;

	/*SYSTEMTIME syttime;
	GetLocalTime(&syttime);
	if(syttime.wYear >2018 || (syttime.wYear == 2018 && syttime.wMonth >7))
	{
		DebugPrint(DEBUG_CMPLAY_10,"Soft Time expired");	
		return NULL;
	*/
	if (NULL == ptCmpPlayer)
	{
		return NULL;	
	}

    ptCmpPlayer->iPlayRange = 0;
	if (0 == g_iCMPlayerInitFlag)
	{
		g_iCMPlayerInitFlag = 1;
		WinSocketInit();
		VDEC_Init();;
	}

	memset(ptCmpPlayer->acUrl,0,sizeof(ptCmpPlayer->acUrl));
	ptCmpPlayer->ePlayState			= CMP_STATE_IDLE;
	ptCmpPlayer->iReconnectFlag		= 0;
	ptCmpPlayer->iPlaySecs = 0;
	ptCmpPlayer->iPlaySpeed = 1;
	ptCmpPlayer->iStreamFlag = 0;
	ptCmpPlayer->iIgnoreFrameNum = 0;
	ptCmpPlayer->hOpenMediaEvent = NULL;
	ptCmpPlayer->iOpenMediaState  = CMP_OPEN_MEDIA_FAIL;
	ptCmpPlayer->hPlayThread = NULL;
	memset(&ptCmpPlayer->tPrevFrameTs,0,sizeof(timeval));
	ptCmpPlayer->iRtspHeartCount = 0;
	ptCmpPlayer->uiPrevFrameTs  = 0;
	ptCmpPlayer->uiPlayBaseTime  = 0;
	ptCmpPlayer->eDecType = eDecType;

	ptCmpPlayer->VHandle = NULL;
	ptCmpPlayer->hWnd = hWnd;
	InitMessgeList(ptCmpPlayer);

	return (CMPHandle)ptCmpPlayer;
}

CMPPlayer_API int CMP_UnInit(CMPHandle hPlay)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return -1;
	}
	InitMessgeList(ptCmpPlayer);
	delete ptCmpPlayer;
	ptCmpPlayer=NULL;
	return 0;
}

CMPPlayer_API int CMP_OpenMediaPreview(CMPHandle hPlay, const char *pcRtspUrl, int iTcpFlag)
{
	int iRet = 0;
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return -1;
	}

	ptCmpPlayer->iPlayStreamType = PLAY_STREAM_TYPE_PREVIEW;
	ptCmpPlayer->iReconnectFlag = 1;
	sprintf(ptCmpPlayer->acUrl,  "%s", pcRtspUrl);
	ptCmpPlayer->iThreadRunFlag = 1;
	ptCmpPlayer->iTcpFlag = iTcpFlag;
	SetPlayState(ptCmpPlayer, CMP_STATE_IDLE);
	ptCmpPlayer->hPlayThread = CreateThread(NULL, 0, MonitorPlayThread, ptCmpPlayer, 0, NULL);
	DebugPrint(DEBUG_CMPLAY_10,"CMP_OpenMediaPreview = %s",ptCmpPlayer->acUrl);	
	return 0;
}


CMPPlayer_API int CMP_OpenMediaFile(CMPHandle hPlay, const char *pcRtspFile, int iTcpFlag)
{
	int iRet = 0;
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return -1;
	}

	ptCmpPlayer->iPlayStreamType = PLAY_STREAM_TYPE_PLAYBACK;
	ptCmpPlayer->iReconnectFlag = 0;
	sprintf(ptCmpPlayer->acUrl,  "%s", pcRtspFile);
	ptCmpPlayer->iThreadRunFlag = 1;
	ptCmpPlayer->iTcpFlag = iTcpFlag;
	ptCmpPlayer->hOpenMediaEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	ptCmpPlayer->hPlayThread = CreateThread(NULL, 0, MonitorPlayThread, ptCmpPlayer, 0, NULL);
	ptCmpPlayer->iOpenMediaState = CMP_OPEN_MEDIA_FAIL;
	DWORD dwRet = WaitForSingleObject(ptCmpPlayer->hOpenMediaEvent, INFINITE);
	ptCmpPlayer->iPlaySecs = 0;
	if(CMP_OPEN_MEDIA_SUCC != ptCmpPlayer->iOpenMediaState)
	{
		DebugPrint(DEBUG_CMPLAY_10,"CMP_OpenMediaFile = %s Fail",ptCmpPlayer->acUrl);	
		return -1;
	}

	return 0;
}


CMPPlayer_API int CMP_CloseMedia(CMPHandle hPlay)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return -1;
	}
	ptCmpPlayer->iThreadRunFlag = 0;
	if (ptCmpPlayer->hPlayThread)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject(ptCmpPlayer->hPlayThread, INFINITE))
		{
			CloseHandle(ptCmpPlayer->hPlayThread);
		} 
		else
		{
			DWORD nExitCode;
			GetExitCodeThread(ptCmpPlayer->hPlayThread, &nExitCode);
			if (STILL_ACTIVE == nExitCode)
			{
				TerminateThread(ptCmpPlayer->hPlayThread, nExitCode);

			}	
			CloseHandle(ptCmpPlayer->hPlayThread);
		}
		ptCmpPlayer->hPlayThread = NULL;
	}

	if(ptCmpPlayer->hOpenMediaEvent)
	{
		CloseHandle(ptCmpPlayer->hOpenMediaEvent);
		ptCmpPlayer->hOpenMediaEvent = NULL;
	}

	//VDEC_StopPlayStream(ptCmpPlayer->VHandle);
	SetPlayState(ptCmpPlayer, CMP_STATE_STOP);

	return 0;
}

CMPPLAY_STATE CMP_GetPlayStatus(CMPHandle hPlay)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return CMP_STATE_ERROR;
	}

	return ptCmpPlayer->ePlayState;
}


CMPPlayer_API int CMP_GetPlayRange(CMPHandle hPlay)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return -1;
	}
	return ptCmpPlayer->iPlayRange;
}

CMPPlayer_API int CMP_GetPlayTime(CMPHandle hPlay)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return 0;	
	}

	return ptCmpPlayer->iPlaySecs/1000;
}




CMPPlayer_API int CMP_PlayMedia(CMPHandle hPlay)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
	CMPPLAY_STATE ePlayState = CMP_STATE_IDLE;
	if (NULL == ptCmpPlayer)
	{
		return -1;	
	}

	ePlayState = CMP_GetPlayStatus(hPlay);
	if (ePlayState ==  CMP_STATE_ERROR)
	{
		return -1;
	}
	if (ePlayState == CMP_STATE_PLAY )
	{
		return 0;
	}

	PushMessage(ptCmpPlayer, E_PLAY_STATE_PLAY, 0);
	SetPlayState(ptCmpPlayer, CMP_STATE_PLAY);
	VDEC_StartPlayStream(ptCmpPlayer->VHandle);
	ptCmpPlayer->iPlaySpeed = 1; 
	return 0;
}

CMPPlayer_API int CMP_PauseMedia(CMPHandle hPlay)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
	CMPPLAY_STATE ePlayState = CMP_STATE_IDLE;
	if (NULL == ptCmpPlayer)
	{
		return -1;
	}
	ePlayState = CMP_GetPlayStatus(hPlay);
	if (ePlayState ==  CMP_STATE_ERROR)
	{
		return -1;
	}
	if (ePlayState == CMP_STATE_PAUSE )
	{
		return 0;
	}
	SetPlayState(ptCmpPlayer, CMP_STATE_PAUSE);
	PushMessage(ptCmpPlayer, E_PLAY_STATE_PAUSE, 0);
	VDEC_PausePlayStream(ptCmpPlayer->VHandle);
	return 0;
}

CMPPlayer_API int CMP_SetPosition(CMPHandle hPlay, __int64 nPosTime)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
	CMPPLAY_STATE ePlayState = CMP_STATE_IDLE;
	double dValue = nPosTime;

	if (NULL == ptCmpPlayer)
	{
		return -1;
	}

	ePlayState = CMP_GetPlayStatus(hPlay);
	if (CMP_STATE_ERROR != ePlayState)
	{
		PushMessage(ptCmpPlayer, E_PLAY_STATE_DRAG_POS, dValue);	
		SetPlayState(ptCmpPlayer, CMP_STATE_PLAY);
		ptCmpPlayer->iPlaySpeed = 1;
		ptCmpPlayer->iIgnoreFrameNum = 10;
		ptCmpPlayer->iPlaySecs = nPosTime *1000;
	}

	return 0;
}

CMPPlayer_API int CMP_SetPlaySpeed(CMPHandle hPlay, double dSpeed)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
	CMPPLAY_STATE ePlayState = CMP_STATE_IDLE;
	if (NULL == ptCmpPlayer)
	{
		return -1;
	}
	ePlayState = CMP_GetPlayStatus(hPlay);
	if (ePlayState ==  CMP_STATE_ERROR)
	{
		return -1;
	}

	PushMessage(ptCmpPlayer, E_PLAY_STATE_FAST_FORWARD, dSpeed);
	DebugPrint(DEBUG_CMPLAY_10,"[%s %d] CMP_SetPlaySpeed dSpeed = %.2f",__FUNCTION__, __LINE__, dSpeed);

	if (dSpeed > 0.11 && dSpeed < 0.13)
	{
		SetPlayState(ptCmpPlayer, CMP_STATE_SLOW_FORWARD);
		ptCmpPlayer->iPlaySpeed = 8;
	}
	else if (dSpeed > 0.24 && dSpeed < 0.26)
	{
		SetPlayState(ptCmpPlayer, CMP_STATE_SLOW_FORWARD);
		ptCmpPlayer->iPlaySpeed = 4;
	}
	else if (dSpeed > 0.49 && dSpeed < 0.51)
	{
		SetPlayState(ptCmpPlayer, CMP_STATE_SLOW_FORWARD);
		ptCmpPlayer->iPlaySpeed = 2;
	}
	else if (dSpeed > 1.9 && dSpeed < 2.1)
	{
		SetPlayState(ptCmpPlayer, CMP_STATE_FAST_FORWARD);
		ptCmpPlayer->iPlaySpeed = 2;  
	}
	else if (dSpeed > 3.9 && dSpeed < 4.1)
	{
		SetPlayState(ptCmpPlayer, CMP_STATE_FAST_FORWARD);
		ptCmpPlayer->iPlaySpeed = 4;
	}
	else if(dSpeed < 1.1 && dSpeed > 0.9)
	{
		SetPlayState(ptCmpPlayer, CMP_STATE_PLAY);
		ptCmpPlayer->iPlaySpeed = 1;
	}

	return 0;
}

CMPPlayer_API int CMP_SetVolume(CMPHandle hPlay, int nVolume)
{
	return 0;
}

CMPPlayer_API int CMP_ChangeWnd(CMPHandle hPlay,HWND hWnd )
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return NULL;	
	}

	VDEC_ChangeWindow(ptCmpPlayer->VHandle, hWnd);
	ptCmpPlayer->hWnd = hWnd;

	return 1;
}

int CMP_CaptureBmp(CMPHandle hPlay, char *pcFileName)
{
	PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

	if (NULL == ptCmpPlayer)
	{
		return NULL;	
	}

	//VDEC_CaptureBmp(ptCmpPlayer->VHandle, pcFileName);

	return 1;
}


CMPPlayer_API int CMP_DebugPrint(unsigned int uiDebugLevel, const char *format, ...)
{
	va_list args;
	int iLen = 0;
	int iRet = 0;
	char acBuf[768];
	char acTimeStr[40];
	int iTimeLen = 0;
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	sprintf(acTimeStr,"\r\n%04d_%02d_%02d %02d:%02d:%02d "
		,systime.wYear,systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond);
	iTimeLen = strlen(acTimeStr); 
	strcpy(acBuf,acTimeStr);
	va_start(args,format);
	iLen = vsnprintf(acBuf + iTimeLen, 728, format, args);
	if(iLen < 0)
	{
		printf("**********ERROR:Too few parameters at DebugPrint**********\n");
	}
	va_end(args);
	if (iLen >= 0)
	{
		return DebugPrintData(uiDebugLevel,acBuf,iTimeLen+iLen);
	}
	return 0;
}

CMPPlayer_API int CMP_DebugInit(int iPort)
{
	DebugInit(iPort);
	return 0;
}

CMPPlayer_API int CMP_DebugUninit()
{
	DebugUninit();
	return 0;
}