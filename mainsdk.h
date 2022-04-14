#ifndef MAINSDK_H
#define MAINSDK_H

#include <QString>

#include "GlobleDefine.h"
#include "NVRMsgProc.h"
#include "CMPlayerInterface.h"
#include "fileConfig.h"
#include "workerthread.h"
#include "define.h"
#include "ftpApi.h"
#include "ftp/ftpwindow.h"
//TODO
//#include "UserMonitor.h"
enum
{
    E_USER_INFO_OPER = 0,
    E_USER_INFO_ADMIN,
    E_USER_INFO_SUPER_ADMIN,
    E_USER_INFO_OTHER,
};

struct T_USER_INFO
{
    char acName[32];
    char acPassWord[32];
    int  iUserType;
};

#define C2S(x) QString::fromUtf8(x)
#define FL8T(x) QString::fromLocal8Bit(x)
#define TIMER_START(_X) auto _X##_start =GetTickCount(), _X##_stop = _X##_start
#define TIMER_STOP(_X) _X##_stop = GetTickCount()
#define TIMER_MSEC(_X) (_X##_stop - _X##_start)

int ExecSysCmd(const char *cmd, char *result, int len);

class MainSdk:public QObject
{
	Q_OBJECT
public:
    MainSdk();
    ~MainSdk();

    int initPreview(T_WND_INFO *ptWndInfo, int nMaxNum);
    int UninitPreview();
    int changePreviewWnd(int nIndex, const T_WND_INFO *ptWndInfo);

    int initPlayBack(T_WND_INFO *ptWndInfo);
    int UninitPlayBack();
    int changePlayBackWnd(const T_WND_INFO *ptWndInfo);

    int startMonitor(int iCarriageNo);
    int stopMonitor(int iCarriageNo);

    int getCurrentMonitoriCarr();
    int getMonitorState(int iIndex);

    int startMonitor(int iCarriageNo, int iPos);
    int stopMonitor(int iCarriageNo, int iPos);

    int searchRecordFiles(T_NVR_SEARCH_RECORD *pData);

    int startPlayBack(int iCarriageNo, const char *pszFileName);
    int stopPlayBack(int iCarriageNo);
    int getPlayBackState(int *piPlayState, int *piOpenMediaState);
    int setPlayBackSpeed(double dSpeed);
    int getPlayRange();
    int getPlayPos();
    int setPlayPos(int nPos);

    int checkCarriage(int iCarriageNo);

    int initDownload(const char *ip,int iNvrNo);
    int UninitDownload();

    int downloadFile(int iCarriageNo, const std::vector<std::string> &files, const std::string &sPath);

    int GetNvrNum();

    const char* GetErrorString();

    int GetUserStyle();
    int SetUserInfo(const T_USER_INFO *pUserInfo);
	void SetIPCStatusChangeCallbackFun(STATUSCHANGECALLBACK callbackIPCStatusChange,void* pUserData);
	void SetSearchOverCallBack(SEARCHOVERCALLBACK callbackSearch,void* pUserData);
	void SetNVRDisconnectCallBack(NVRDISCONNECTCALLBACK callbackNVRDisconnect,void *pUserData);
	QString getFtpInfo();
	void NVRDisconnect(int nDisconnect,int iNvrNo);
	bool checkIPandPort(const char* ip, const char* port);
	bool getIPInfo(QString &strIp,QString &strPort,char* szRTSPUrl);
signals:
	 void ShowWarnSignalsSDK( QString title,  QString content);
public slots:
	void PftpProcSlot( int iPos);
	void PfrpError(int iErrorCode);
private:
    int m_iCurrentiCarriageNo;
    int m_iNvrNum;
    std::string     m_sError;
    T_USER_INFO     m_tUserInfo;
    double          m_dPlayBackSpeed;
	//FtpWindow *m_ftpWnd;
	int        m_nDownLoadFiles;
	int        m_nCurrentDownLoadFiles;

};


MainSdk * MAIN_GetSdk();

#endif // MAINSDK_H
