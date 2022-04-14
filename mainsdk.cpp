#include "mainsdk.h"
#include "debug.h"
#include <sstream>
#include "FTP/gb2312_utf8.h"
#define  QFTP  0
//#pragma comment( linker, "/subsystem:\"console\" /entry:\"mainCRTStartup\"" )
//
typedef struct _PreviewInfo
{
	_PreviewInfo::_PreviewInfo()
	{
		hWnd=NULL;
		pCMPHandle=NULL;
		bPlaying = false;
	}
    HWND        hWnd;
    CMPHandle   pCMPHandle;
	bool        bPlaying;
}PreviewInfo;


static PreviewInfo s_tPreviewInfo[MAX_CARRIAGE_PRE_CAMERA_NUM];
static PreviewInfo s_tPlayBackInfo;
static PFTP_HANDLE g_ftpHandle = 0;
#define ZERO (fd_set *)0
static char *parseFileName(char *pcSrcStr)     //根据录像文件路径全名解析得到单纯的录像文件名
{
    char *pcTmp = NULL;
    if (NULL == pcSrcStr)
    {
        return NULL;
    }

    pcTmp = strrchr(pcSrcStr, '/');
    if (NULL == pcTmp)
    {
        return pcSrcStr;
    }

    if (0 == *(pcTmp+1))
    {
        return NULL;
    }
    return pcTmp+1;
}
void MainSdk::PfrpError(int iErrorCode)
{


}
void MainSdk::NVRDisconnect(int nDisconnect,int iNvrNo)
{
	if (E_FILE_DOWNING==STATE_GetFileDownState())
	{
// 		if (m_ftpWnd->getNvrNo()==iNvrNo)
// 		{
// 			STATE_SetFileDownProgress(0);
// 			m_nCurrentDownLoadFiles = m_nDownLoadFiles=0;
// 			STATE_SetFileDownState(E_FILE_DOWN_IDLE);
// 			emit ShowWarnSignalsSDK(FL8T("提示"),FL8T("下载失败"));
// 			m_ftpWnd->cancelDownloadAbourtNvrDisconnect();
// 		}
		
	}

}
 void MainSdk::PftpProcSlot( int iPos)
{
    if(100 == iPos)
    {
        STATE_SetFileDownProgress(100);//iPos=100,表示下载完毕。
		if (m_nDownLoadFiles==m_nCurrentDownLoadFiles+1)
		{
			STATE_SetFileDownState(E_FILE_DOWN_SUCC);
			m_nCurrentDownLoadFiles = m_nDownLoadFiles=0;
		}
		else
		{
			m_nCurrentDownLoadFiles++;
		}
        
    }
    else if(iPos < 0)//暂定iPos=-1表示被告知U盘已拔出, iPos=-2表示被告知U盘写入失败,iPos=-3表示被告知数据接收失败失败。 三种情况都隐藏进度条，并在信号处理函数中销毁FTP连接
    {
        STATE_SetFileDownProgress(100);
		m_nCurrentDownLoadFiles = m_nDownLoadFiles=0;
        STATE_SetFileDownState(E_FILE_DOWN_FAIL);
    }
    else
    {
        STATE_SetFileDownProgress(iPos);
    }
}

int ExecSysCmd(const char *cmd, char *result, int len)
{
	//TODO
    return 0;
}

static MainSdk *s_pMainSdl = NULL;
MainSdk * MAIN_GetSdk()
{
    if(s_pMainSdl == NULL)
    {
        s_pMainSdl = new MainSdk;
    }
    return s_pMainSdl;
}

MainSdk::MainSdk():m_iNvrNum(1)
{
    int i = 0;

    m_iCurrentiCarriageNo = -1;

    m_dPlayBackSpeed = 1.0;

    memset(&m_tUserInfo, 0, sizeof(m_tUserInfo));

    STATE_Init();
    m_iNvrNum = STATE_GetNvrNum();
    NVR_init(m_iNvrNum);
    g_iCarriageNum = STATE_GetCarriageNum();
    for(i = 0; i < MAX_CARRIAGE_NUM; i++)
    {
        g_atCarriages[i].cIpcNum = STATE_GetAllIpcPosCarriageNo(i+1, g_atCarriages[i].acIpcPos, MAX_CARRIAGE_PRE_CAMERA_NUM);
        g_atCarriages[i].cNvrNo = STATE_GeNvrNoByCarriageNo(i+1);
    }
// 	m_ftpWnd = NULL;
// 	m_ftpWnd = new FtpWindow(this);
// 	connect(m_ftpWnd, SIGNAL(signalDownLoadStep(int)),this , SLOT(PftpProcSlot(int)));
// 	connect(m_ftpWnd, SIGNAL(signalDownLoadError(int)),this , SLOT(PfrpError(int)));
	m_nDownLoadFiles = 0;
	m_nCurrentDownLoadFiles  =0;
}
int MainSdk::GetNvrNum()
{
	return m_iNvrNum;
}
MainSdk::~MainSdk()
{
    UninitDownload();
    UninitPlayBack();
    UninitPreview();
    NVR_Uninit();
    STATE_Uninit();
	//delete m_ftpWnd;
}

int MainSdk::checkCarriage(int iCarriageNo)
{
    if(iCarriageNo < 1 || iCarriageNo > g_iCarriageNum /*MAX_CARRIAGE_NUM */)
    {
        DebugPrint(DEBUG_ERROR_PRINT, "invalid iCarriageNo :%d \n", iCarriageNo);
        return -1;
    }

    return 0;
}

int MainSdk::initPreview(T_WND_INFO *ptWndInfo, int nMaxNum)
{
    int nWndNum = nMaxNum;
    if(nWndNum > MAX_CARRIAGE_PRE_CAMERA_NUM)
    {
        nWndNum = MAX_CARRIAGE_PRE_CAMERA_NUM;
    }
    for(int i = 0; i < nWndNum; i++)
    {
        s_tPreviewInfo[i].hWnd = ptWndInfo[i].hWnd;
        s_tPreviewInfo[i].pCMPHandle = CMP_Init((&ptWndInfo[i])->hWnd, CMP_VDEC_NORMAL);
		qDebug()<<"init player handle:"<< s_tPreviewInfo[i].pCMPHandle<<"hwnd:"<<(&ptWndInfo[i])->hWnd<<"index:"<<i;
    }

    return 0;
}
int MainSdk::UninitPreview()
{
	if (m_iCurrentiCarriageNo<0)
	{
		return -1;
	}
    for(int i = 0; i < MAX_CARRIAGE_PRE_CAMERA_NUM; i++)
    {
		
		char acRtsp[256]={0};
		memset(acRtsp, 0, 256);
		STATE_GetIpcRtsp(m_iCurrentiCarriageNo, g_atCarriages[m_iCurrentiCarriageNo-1].acIpcPos[i], acRtsp, 1);
// 		QString strIP,strPort = "";
// 		getIPInfo(strIP,strPort,acRtsp);
// 
// 		bool bTestOk = checkIPandPort(strIP.toStdString().c_str(),strPort.toStdString().c_str());
// 		if (!bTestOk)
// 		{
// 			continue;
// 		}
        if(s_tPreviewInfo[i].pCMPHandle)
        {
			TIMER_START(CMP_CloseMedia);
		    CMP_CloseMedia(s_tPreviewInfo[i].pCMPHandle);
			CMP_UnInit(s_tPreviewInfo[i].pCMPHandle);
			
			TIMER_STOP(CMP_CloseMedia);
			std::stringstream  strFormatInfo;
			strFormatInfo <<"rtsp:"<<acRtsp<<"handle:"<<s_tPreviewInfo[i].pCMPHandle<< " CMP_CloseMedia Cost  "<< TIMER_MSEC(CMP_CloseMedia) << "ms.\n<<";
			OutputDebugStringA(strFormatInfo.str().c_str());
        
            qDebug()<<"free all cmp handle index:"<<i<<"handle:"<<s_tPreviewInfo[i].pCMPHandle;
            s_tPreviewInfo[i].pCMPHandle = NULL;
            s_tPreviewInfo[i].hWnd = NULL;
			
        }
		else
		{
			qDebug()<<"no free all cmp handle index:"<<i<<"handle:"<<s_tPreviewInfo[i].pCMPHandle;
		}
    }
    return 0;
}

int MainSdk::changePreviewWnd(int nIndex, const T_WND_INFO *ptWndInfo)
{
    CMP_ChangeWnd(s_tPreviewInfo[nIndex].pCMPHandle, ptWndInfo->hWnd
		);
    return 0;
}

int MainSdk::initPlayBack(T_WND_INFO *ptWndInfo)
{
    s_tPlayBackInfo.hWnd = ptWndInfo->hWnd;
    s_tPlayBackInfo.pCMPHandle = CMP_Init(ptWndInfo->hWnd, CMP_VDEC_NORMAL);
    return 0;
}

int MainSdk::UninitPlayBack()
{
    if(s_tPlayBackInfo.pCMPHandle)
    {
        CMP_CloseMedia(s_tPlayBackInfo.pCMPHandle);
        CMP_UnInit(s_tPlayBackInfo.pCMPHandle);
        s_tPlayBackInfo.pCMPHandle = NULL;
        s_tPlayBackInfo.hWnd = NULL;
    }
    return 0;
}

int MainSdk::changePlayBackWnd(const T_WND_INFO *ptWndInfo)
{
    CMP_ChangeWnd(s_tPlayBackInfo.pCMPHandle, ptWndInfo->hWnd);
    return 0;
}
bool MainSdk::checkIPandPort(const char* ip, const char* port)
{
	struct sockaddr_in addr;
	struct fd_set mask;
	int fd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = inet_addr(ip);
	unsigned long flag = 1;
	// 调用ioctlsocket()设置套接字为非阻塞模式
	if (ioctlsocket(fd, FIONBIO, &flag) != 0)
	{
		// 设置失败处理
		printf("\r\nSock Error:%s", WSAGetLastError());
		closesocket(fd);
		return false;
	}

	::connect(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr));
	struct timeval timeout;
	// 调用connect()连接远程主机端口
	timeout.tv_sec = 1; // 超时限制为18秒
	timeout.tv_usec = 0;
	FD_ZERO(&mask); // 清空集合mask
	FD_SET(fd, &mask); // 将sock放入集合mask中

	// 用select() 处理扫描结果
	switch(select(fd , ZERO, &mask, ZERO, &timeout))
	{
	case -1:
		{
			printf("\r\nSelect() error");
			closesocket(fd);
			return false;
		}
		// sock超时处理
	case 0:
		{
			printf("\r\outtime error");
			closesocket(fd);
			return false;
		}
	default:
		{
			printf("\r\connect ok");
			closesocket(fd);
			return true;
		}
	}
	closesocket(fd);
	return true;
}

int MainSdk::startMonitor(int iCarriageNo, int iPos)
{
	
	int iRet  = checkCarriage(iCarriageNo);
	if(iRet < 0)
	{
		return -1;
	}
	if (iCarriageNo!=m_iCurrentiCarriageNo)
	{
		return -1;
	}
	stopMonitor(iCarriageNo,iPos);
	iRet = stopPlayBack(iCarriageNo);

	m_iCurrentiCarriageNo = iCarriageNo;

	int iNvrNo = g_atCarriages[iCarriageNo-1].cNvrNo;
	if(iNvrNo < 0)
	{
		DebugPrint(DEBUG_ERROR_PRINT, "invalid iCarriageNo :%d, iNvrNo:%d \n", iCarriageNo,iNvrNo);
		qDebug()<<"invalid iCarriageNo "<<iCarriageNo<<"iNvrNo"<<iNvrNo<<"pos"<<iPos;
		return -1;
	}

	if(E_SERV_STATUS_CONNECT != NVR_GetConnectStatus(iNvrNo))
	{
		qDebug()<<"invalid NVR_GetConnectStatus "<<iCarriageNo<<"iNvrNo"<<iNvrNo<<"pos"<<iPos;
		return -1;
	}

	int i = iPos-1;
	char acRtsp[256] = {0};
	int iIPCNum = (int)g_atCarriages[iCarriageNo-1].cIpcNum;
	memset(acRtsp, 0, 256);
	STATE_GetIpcRtsp(iCarriageNo, g_atCarriages[iCarriageNo-1].acIpcPos[i], acRtsp, 1);
	QString strIP,strPort = "";
	{
		T_IPC_STATE ptState;
		T_IPC_ID pcIPCID;
		pcIPCID.cCarriageNo = iCarriageNo;
		pcIPCID.cPos = g_atCarriages[iCarriageNo-1].acIpcPos[i];
		STATE_GetIpcState(iNvrNo,&pcIPCID, &ptState);
		if (ptState.cOnlineState==STATE_ONLINE)
		{
			CMPPLAY_STATE enumPlayStatus = CMP_GetPlayStatus(s_tPreviewInfo[i].pCMPHandle);
			if (enumPlayStatus==CMP_STATE_PLAY||
				enumPlayStatus==CMP_STATE_FAST_FORWARD||
				enumPlayStatus==CMP_STATE_PAUSE)
			{
				return 0;
			}
			TIMER_START(CMP_OpenMedia);
			int iRet = CMP_OpenMediaPreview(s_tPreviewInfo[i].pCMPHandle, acRtsp, CMP_TCP);
			if (iRet==0)
			{
				s_tPreviewInfo[i].bPlaying = true;
			}
			TIMER_STOP(CMP_OpenMedia);
			std::stringstream  strFormatInfo;
			strFormatInfo <<"rtsp open:"<<acRtsp<<"handle:"<<s_tPreviewInfo[i].pCMPHandle<< " CMP_OpenMediaPreview Cost  "<< TIMER_MSEC(CMP_OpenMedia) << "ms.\n<<";
			OutputDebugStringA(strFormatInfo.str().c_str());
			DebugPrint(DEBUG_ERROR_PRINT, "open %s \n", acRtsp);
			qDebug()<<"open url pos "<<acRtsp<<iCarriageNo<<"iNvrNo"<<iNvrNo<<"pos"<<iPos <<"handle: "<<s_tPreviewInfo[i].pCMPHandle<<"hWND: "<<s_tPreviewInfo[i].hWnd<<"index"<<i;
		}
	}
    return 0;
}
int MainSdk::stopMonitor(int iCarriageNo, int iPos)
{
	char acRtsp[256] = {0};
	
	memset(acRtsp, 0, 256);
	STATE_GetIpcRtsp(iCarriageNo, g_atCarriages[iCarriageNo-1].acIpcPos[iPos-1], acRtsp, 1);
	


	{
		if(s_tPreviewInfo[iPos-1].pCMPHandle)
		{
			TIMER_START(CMP_CloseMedia);
			CMP_CloseMedia(s_tPreviewInfo[iPos-1].pCMPHandle);
			//CMP_UnInit(s_tPreviewInfo[iPos].pCMPHandle);
			TIMER_STOP(CMP_CloseMedia);
			std::stringstream  strFormatInfo;
			strFormatInfo<< " CMP_CloseMedia Cost  " << TIMER_MSEC(CMP_CloseMedia) << "ms.\n";
			OutputDebugStringA(strFormatInfo.str().c_str());
		
			qDebug()<<"free pos cmp handle index:"<<iPos-1<<"handle:"<<s_tPreviewInfo[iPos-1].pCMPHandle;
			qDebug()<<"close pos "<<"url:"<<acRtsp<<"handle:"<<s_tPreviewInfo[iPos-1].pCMPHandle<<"car no:"<<iCarriageNo<<"pos"<<iPos-1;
			//s_tPreviewInfo[iPos].pCMPHandle = NULL;
			//s_tPreviewInfo[iPos].hWnd = NULL;
		}
	}
	
    return 0;
}


int MainSdk::startMonitor(int iCarriageNo)
{
    int iRet = -1;
    if(m_iCurrentiCarriageNo == iCarriageNo)
    {
        return 0;
    }

    iRet = checkCarriage(iCarriageNo);
    if(iRet < 0)
    {
        return -1;
    }

    //iRet = stopMonitor(iCarriageNo);

    //iRet = stopPlayBack(iCarriageNo);

    m_iCurrentiCarriageNo = iCarriageNo;

    int iNvrNo = g_atCarriages[iCarriageNo-1].cNvrNo;
    if(iNvrNo < 0)
    {
        DebugPrint(DEBUG_ERROR_PRINT, "invalid iCarriageNo :%d, iNvrNo:%d \n", iCarriageNo,iNvrNo);
        return -1;
    }

    if(E_SERV_STATUS_CONNECT != NVR_GetConnectStatus(iNvrNo))
    {
        return -1;
    }


    char acIp[32] = {0};
    memset(acIp, 0, sizeof(acIp));
    STATE_GetNvrIpAddr(iNvrNo, acIp);
    if(acIp[0] == 0)
    {
        return -1;
    }

    int i = 0;
    char acRtsp[256] = {0};
    for(i = 0; i < g_atCarriages[iCarriageNo-1].cIpcNum; i++)
    {
        memset(acRtsp, 0, 256);
        STATE_GetIpcRtsp(iCarriageNo, g_atCarriages[iCarriageNo-1].acIpcPos[i], acRtsp, 1);
		T_IPC_STATE ptState;
		T_IPC_ID pcIPCID;
		pcIPCID.cCarriageNo = iCarriageNo;
		pcIPCID.cPos = g_atCarriages[iCarriageNo-1].acIpcPos[i];
		STATE_GetIpcState(iNvrNo,&pcIPCID, &ptState);
		if (ptState.cOnlineState==STATE_ONLINE)
		{
			
			{
				CMPPLAY_STATE enumPlayStatus = CMP_GetPlayStatus(s_tPreviewInfo[i].pCMPHandle);
				if (enumPlayStatus==CMP_STATE_PLAY||
					enumPlayStatus==CMP_STATE_FAST_FORWARD||
					enumPlayStatus==CMP_STATE_PAUSE)
				{
					CMP_CloseMedia(s_tPreviewInfo[i].pCMPHandle);
					Sleep(1);
				}
				TIMER_START(CMP_OpenMedia2);
				CMP_OpenMediaPreview(s_tPreviewInfo[i].pCMPHandle, acRtsp, CMP_TCP);
				TIMER_STOP(CMP_OpenMedia2);
				std::stringstream  strFormatInfo;
				strFormatInfo <<"rtsp open:"<<acRtsp<<"handle:"<<s_tPreviewInfo[i].pCMPHandle<< " CMP_OpenMediaPreview Cost  "<< TIMER_MSEC(CMP_OpenMedia2) << "ms.\n<<";
				OutputDebugStringA(strFormatInfo.str().c_str());
				DebugPrint(DEBUG_ERROR_PRINT, "open %s \n", acRtsp);
				qDebug()<<"open one no pos "<<"url:"<<acRtsp<<"handle:"<<s_tPreviewInfo[i].pCMPHandle<<"car no:"<<iCarriageNo<<"index"<< i;
			}
		}
    }


    return 0;
}
bool MainSdk::getIPInfo(QString &strIp,QString &strPort,char* szRTSPUrl)
{
	bool bFind = false;
	QString strRTSPUrl = QString("%1").arg(szRTSPUrl);
	strRTSPUrl = strRTSPUrl.mid(7,strRTSPUrl.length());
	int nRTSPStartPosLine = strRTSPUrl.indexOf("/");
	int nRTSPStartPosMao = strRTSPUrl.indexOf(":");
	if (nRTSPStartPosMao>0)
	{
		strIp = strRTSPUrl.mid(0,nRTSPStartPosMao);
		bFind = true;
	}
	if (nRTSPStartPosLine>0)
	{
		strIp = strRTSPUrl.mid(0,nRTSPStartPosLine);
		bFind = true;
	}
	strPort="554";
	return bFind;
}
int MainSdk::stopMonitor(int iCarriageNo)
{
    int iRet = -1;
    iRet = checkCarriage(iCarriageNo);
    if(iRet < 0)
    {
        return -1;
    }
    if(m_iCurrentiCarriageNo <= 0)
    {
        return -1;
    } 
	qDebug()<<"stopMonitor car "<<"car no:"<<iCarriageNo;
	UninitPreview();
    m_iCurrentiCarriageNo = -1;
	
    return 0;
}
int MainSdk::getCurrentMonitoriCarr()
{
    return m_iCurrentiCarriageNo;
}
int MainSdk::getMonitorState(int iIndex)
{
	//todo
    return 1;
}

int MainSdk::searchRecordFiles(T_NVR_SEARCH_RECORD *pData)
{
    int iRet = -1;
    int iCarriageNo = pData->iCarriageNo;
    iRet = checkCarriage(iCarriageNo);
    if(iRet < 0)
    {
        return -1;
    }
    int iNvrNo = g_atCarriages[iCarriageNo-1].cNvrNo;
    if(iNvrNo < 0)
    {
        DebugPrint(DEBUG_ERROR_PRINT, "invalid iCarriageNo :%d, iNvrNo:%d \n", iCarriageNo,iNvrNo);
        return -1;
    }
    STATE_SetFileSearchState(E_FILE_SEARCHING,0);
    NVR_CleanFileInfo(iNvrNo);
    return NVR_SendCmdInfo(iNvrNo, CLI_SERV_MSG_TYPE_GET_RECORD_FILE,
            (char*)pData, sizeof(T_NVR_SEARCH_RECORD));
}

int MainSdk::startPlayBack(int iCarriageNo, const char *pszFileName)
{
    int iRet = -1;
    iRet = checkCarriage(iCarriageNo);
    if(iRet < 0)
    {
        return -1;
    }

    //iRet = stopMonitor(iCarriageNo);
    int iNvrNo = g_atCarriages[iCarriageNo-1].cNvrNo;
    if(E_SERV_STATUS_CONNECT != NVR_GetConnectStatus(iNvrNo))
    {
        return -1;
    }

    char acIp[32] = {0};
    memset(acIp, 0, sizeof(acIp));
    STATE_GetNvrIpAddr(iNvrNo, acIp);
    if(acIp[0] == 0)
    {
        return -1;
    }

    m_dPlayBackSpeed = 1.0;
    char acUrl[256] = {0};
    if(pszFileName[0] == '/')
    {
        sprintf(acUrl,"rtsp://%s:554%s", acIp,pszFileName);
    }
    else
    {
        sprintf(acUrl,"rtsp://%s:554/%s", acIp,pszFileName);
    }

	TIMER_START(CMP_OpenMediaFile);
	iRet = CMP_OpenMediaFile(s_tPlayBackInfo.pCMPHandle, acUrl, CMP_TCP);
	TIMER_STOP(CMP_OpenMediaFile);
	std::string  strFormatInfo;
	qDebug() << " CMP_OpenMediaFile Cost  " << TIMER_MSEC(CMP_OpenMediaFile) << "ms.\n";
    return iRet;
}

int MainSdk::stopPlayBack(int iCarriageNo)
{
    UninitPlayBack();
    return 0;
}

int MainSdk::getPlayBackState(int *piPlayState, int *piOpenMediaState)
{
    if(piPlayState)
    {
        *piPlayState = CMP_GetPlayStatus(s_tPlayBackInfo.pCMPHandle);
    }
    if(piOpenMediaState)
    {
		//todo
        //*piOpenMediaState = CMP_GetOpenMediaState(s_tPlayBackInfo.pCMPHandle);
    }
    return 0;
}

int MainSdk::setPlayBackSpeed(double dSpeed)
{
    if(CMP_GetPlayStatus(s_tPlayBackInfo.pCMPHandle) != CMP_STATE_PAUSE)
    {
        CMP_SetPlaySpeed(s_tPlayBackInfo.pCMPHandle,dSpeed);
    }
    return 0;
}

int MainSdk::getPlayRange()
{
    return CMP_GetPlayRange(s_tPlayBackInfo.pCMPHandle);
}

int MainSdk::getPlayPos()
{
    return CMP_GetPlayTime(s_tPlayBackInfo.pCMPHandle);
}

int MainSdk::setPlayPos(int nPos)
{
    return CMP_SetPosition(s_tPlayBackInfo.pCMPHandle, nPos);
}
static void PftpProc(PFTP_HANDLE PHandle, int iPos)
{
	if(100 == iPos)
	{
		STATE_SetFileDownProgress(100);//iPos=100,表示下载完毕。
		STATE_SetFileDownState(E_FILE_DOWN_SUCC);
	}
	else if(iPos < 0)//暂定iPos=-1表示被告知U盘已拔出, iPos=-2表示被告知U盘写入失败,iPos=-3表示被告知数据接收失败失败。 三种情况都隐藏进度条，并在信号处理函数中销毁FTP连接
	{
		STATE_SetFileDownProgress(100);
		STATE_SetFileDownState(E_FILE_DOWN_FAIL);
	}
	else
	{
		STATE_SetFileDownProgress(iPos);
	}
}

int MainSdk::initDownload(const char *ip,int iNvrNo)
{

#if QFTP
	int nRet=m_ftpWnd->connectToFtp(ip, "", "admin", "12345",iNvrNo);
	if (nRet<0)
	{
		m_sError = "下载连接失败!";
		DebugPrint(DEBUG_FTP_PRINT, "[%s] connect to ftp server:%s error!\n", __FUNCTION__, ip);
		return -1;
	}
	else
	{
		return 0;
	}
#else
	if(g_ftpHandle == NULL)
	{
		g_ftpHandle = FTP_CreateConnect(ip, FTP_SERVER_PORT, PftpProc, "admin", "12345");
		if (0 == g_ftpHandle)
		{
			m_sError = "下载连接失败!";
			DebugPrint(DEBUG_FTP_PRINT, "[%s] connect to ftp server:%s error!\n", __FUNCTION__, ip);
			return -1;
		}
		else
		{
			return 0;
		}
	}
#endif
	

	return 0;
}

int MainSdk::UninitDownload()
{
//     if(m_ftpWnd != NULL)
//     {
// 		m_ftpWnd->disConnectFtp();
//         //delete m_ftpWnd;
//         //m_ftpWnd = NULL;
//     }

	if(g_ftpHandle != NULL)
	{
		FTP_DestoryConnect(g_ftpHandle);
		g_ftpHandle = NULL;
	}
    return 0;
}

int MainSdk::downloadFile(int iCarriageNo, const std::vector<std::string> &files, const std::string &sPath)
{
    int iNvrNo = -1;
    int iRet = 0, row = 0;
    char acFullFileName[256] = {0};
    char acSaveFileName[256] = {0};
	char tmpacSaveFileName[256]={0};
    char acIpAddr[32] = {0};
    int iSize = files.size();
	int outlen =0;
	m_nDownLoadFiles = iSize;
    int iState = STATE_GetFileDownState();

    if(E_FILE_DOWNING == iState)
    {
        m_sError = "正在下载,请稍后!";
        return -1;
    }

    if(iCarriageNo <= 0 || iSize <= 0)
    {
        m_sError = "无可下载文件!";
        return -1;
    }

/*
    if ((access(USB_PATH, F_OK) < 0) || (0 == STATE_FindUsbDev()))
    {
        m_sError = "请插入U盘!";
        return -1;
    }
*/


    iNvrNo = g_atCarriages[iCarriageNo-1].cNvrNo;
    if(iNvrNo < 0)
    {
        DebugPrint(DEBUG_ERROR_PRINT, "invalid iCarriageNo :%d, iNvrNo:%d \n", iCarriageNo,iNvrNo);
        return -1;
    }
	NVR_GetConnectStatus(iNvrNo);
	if(E_SERV_STATUS_CONNECT != NVR_GetConnectStatus(iNvrNo))
	{
		 m_sError = "NVR不在线 取消下载!";
		return -1;
	}
    if (iSize > 0)
    {
        
            STATE_GetNvrIpAddr(iNvrNo,acIpAddr);
			if(initDownload(acIpAddr,iNvrNo) < 0)
			{
				return -1;
			}
 
			QString strFilePath = QString("%1").arg(sPath.c_str());
			std::string stdstrFilePath =  strFilePath.toStdString();
#if QFTP
	
		m_ftpWnd->setLocalPath(strFilePath);
#endif
		m_nDownLoadFiles = iSize;
        for (row = 0; row < iSize; row++)
        {
            memset(acFullFileName,0,sizeof(acFullFileName));
            strcpy(acFullFileName, files.at(row).c_str());
            if (parseFileName(acFullFileName) != NULL)
            {
				
                _snprintf(acSaveFileName, sizeof(acSaveFileName),"%s/%s",
                    sPath.c_str(), parseFileName(acFullFileName));

				CHelpUint::UTF_8ToGB2312(&outlen, tmpacSaveFileName, acSaveFileName, strlen(acSaveFileName), 256);

                DebugPrint(DEBUG_FTP_PRINT, "[%s] add download file:%s!\n", __FUNCTION__, acSaveFileName);
             
#if QFTP
				iRet = m_ftpWnd->AddloadFileList(acFullFileName,acSaveFileName);
#else
				iRet = FTP_AddDownLoadFile(g_ftpHandle,acFullFileName,tmpacSaveFileName);
#endif
				 
				if (iRet < 0)
				{
#if QFTP
					m_ftpWnd->disConnectFtp();
#else
					FTP_DestoryConnect(g_ftpHandle);
#endif
					m_sError = "文件下载失败!";
					return -1;
				}
            }
        }
		if (iSize>1)
		{
			
		}
#if QFTP
#else
        iRet = FTP_FileDownLoad(g_ftpHandle);
         if (iRet < 0)
         {
             FTP_DestoryConnect(g_ftpHandle);
             m_sError = "文件下载失败!";
             return -1;
         }
#endif
        STATE_SetFileDownState(E_FILE_DOWNING);

        m_sError = "文件正在下载!";

        return 0;
    }
    else
    {
        m_sError = "无可下载文件!";
        return -1;
    }
}

const char* MainSdk::GetErrorString()
{
    return m_sError.c_str();
}

int MainSdk::GetUserStyle()
{
    return m_tUserInfo.iUserType;
}
int MainSdk::SetUserInfo(const T_USER_INFO *pUserInfo)
{
    if(pUserInfo->acName[0])
    {
        memcpy(m_tUserInfo.acName, pUserInfo->acName, sizeof(m_tUserInfo.acName));
    }
    if(pUserInfo->acPassWord[0])
    {
        memcpy(m_tUserInfo.acPassWord, pUserInfo->acPassWord, sizeof(m_tUserInfo.acPassWord));
    }
    m_tUserInfo.iUserType = pUserInfo->iUserType;
    return 0;
}

void MainSdk::SetIPCStatusChangeCallbackFun(STATUSCHANGECALLBACK callbackIPCStatusChange,void *pUserData)
{
	STATE_SetIPCStatusCallback(callbackIPCStatusChange,pUserData);
}

void MainSdk::SetSearchOverCallBack(SEARCHOVERCALLBACK callbackSearch,void* pUserData)
{
	STATE_SetSearchOverCallback(callbackSearch,pUserData);
}

void MainSdk::SetNVRDisconnectCallBack(NVRDISCONNECTCALLBACK callbackNVRDisconnect,void *pUserData)
{
	STATE_SetNVRDisconnectCallback(callbackNVRDisconnect,pUserData);
}
QString MainSdk::getFtpInfo()
{
// 	if (m_ftpWnd)
// 	{
// 		return	m_ftpWnd->getFtpInfoInFTPWindow();
// 	}
	return "";
}