#ifndef USERMONITOR_H
#define USERMONITOR_H

#include <QWidget>
#include <define.h>
#include<QLabel>
#include<QDesktopWidget>

#include<QTimer>
#include<QDateTime>
#include<QPushButton>
#include<QDate>
#include<QDebug>
#include<QTableWidget>
#include<QFileDialog>
#include<QProgressDialog>
#include<UserRegister.h>
#include <QSqlQuery>
#include <QSqlDatabase>
#include<QSqlError>
#include<QStandardItemModel>
#include<Keyboard.h>
#include<QGroupBox>
#include<QMouseEvent>
#include "datetimesetwidget.h"

#include "mainsdk.h"
#include "singleform.h"
class MainSdk;
class SingleForm;
namespace Ui {
class UserMonitor;
}
enum POLL_TIME_TYPE
{
	USER,
	FORMAT
};
class UserMonitor;
struct THREAD_PARAM
{
	UserMonitor *m_pThis;
	int          m_nCarrriage;
	bool         m_bForcePlay;
};
class UserMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit UserMonitor(QWidget *parent = nullptr);
    ~UserMonitor();
    int showWarn(QWidget *parent, const QString &title, const QString &content);
    void ResizeControl(int Width,int Height);
    void ResizeMainFunctionButton(int Width,int Height);
    void ResizeLabel(int width,int Height);
    void ResizeButton(int width,int Height);

    void SetControlStyleSheet();
    void SetMoniitorDefaultStyleSheet(int iIndex = -1);

    void InitControl();
    // 切换视频窗口样式表
    void SWitchVideoWinStyleSheet();
	
    void InitParams();
	//初始化设备列表
	void InitCarNVRButton();
	//创建单个NVR列表
	QGroupBox *createNVRGroup(int nIndex,int nCarNum =4);
    void InitRender();
    void UnInitRender();

    void SetMainButtonControlState(int LastSelectState,int CurSelectState);

    void StopPlayVideo();

    void SetCarrriageStatus();

    void GetDeviceInfoOfCarriage(bool bUpdate);

    void GetAlarmList();

    void SetPantoStyleSheet(int iCarriageIndex);
    bool eventFilter(QObject *, QEvent *);

    void SetCarrriage(int iCarrriage, bool bForcePlay = false);

	void SetCarrriageWorker(int iCarrriage, bool bForcePlay = false);

    void loadUserInfo();

    void updateUserTable();

    void setLoginType(int iLoginType);

    int searchFile();
	bool UpdateVersion();

	static void __stdcall IPCOnlineStatusChange(int iNvrNo, void* ptId, void *ptState,void* pUserData); 
	static void __stdcall SearchOver(void* pUserData);
	static void __stdcall NVRDisconnect(int nDisconnect,int iNvrNo,void* pUserData);

	static DWORD WINAPI SetCarrriageThread(void *arg);
	void SearchFileOver();
private:
    QString usbPath();
	QString usbWindPath();
	bool winIsUsb(char* volumePath);
    void updateSearchFile();

public slots:
    void ProcessUpdatePlayBackTimerOut();

protected:
    virtual void showEvent(QShowEvent *event);

    virtual void hideEvent(QHideEvent *event);

private slots:
    void on_pushButton_VideoMonitor_clicked();

    void on_pushButton_VideoPlayback_clicked();

    void on_pushButton_VideoDownload_clicked();

    void on_pushButton_prevCar_clicked();

    void on_pushButton_nextCar_clicked();

    void ProcessTimerOut();

    void ProcessPollTimerOut();

    void ProcessConnectTimerOut();



    void on_pushButton_pollstart_clicked();

    void on_pushButton_pollsuspend_clicked();



    void on_pushButton_BackCar1_clicked();

    void on_pushButton_BackCar2_clicked();

    void on_pushButton_BackCar3_clicked();

    void on_pushButton_BackCar4_clicked();

    void on_pushButton_BackCar5_clicked();

    void on_pushButton_BackCar6_clicked();

    void on_pushButton_BackCar7_clicked();

    void on_pushButton_BackCar8_clicked();

    void on_pushButton_play_clicked();

    void on_pushButton_suspend_clicked();

    void on_pushButton_back_clicked();

    void on_pushButton_forward_clicked();

    void on_pushButton_DayPlayBack_clicked();

    void on_pushButton_MonthPlayBack_clicked();

    void on_pushButtonStartQuery_clicked();

   // void InitDeviceSlots();


    void on_pushButton_DeviceStaus_clicked();

    void on_pushButton_Update_clicked();

    void on_pushButtonDeviceStatus_clicked();

    void on_pushButtonSaveStatus_clicked();

    void on_pushButtonAlarmList_clicked();

    void ProcessUpdateSaveStatusTimerOut();

    void on_comboBoxCarriageSelect_currentIndexChanged(int index);

    void SearchFileResultSlot(QVariant);

    void on_pushButtonDownload_clicked();




    void SearchFileTimerSlots();

    void DownloadFileTimerSlots();


    void on_pushButtonDownloadLastFile_clicked();

    void on_pushButtonDownloadBack_clicked();

    void on_pushButtonDownloadPlay_clicked();

    void on_pushButtonDownloadStop_clicked();

    void on_pushButtonDownloadForward_clicked();

    void on_pushButtonDownloadNextFile_clicked();



    void on_pushButtonSetPollTIme_clicked();

    void on_pushButtonAddUser_clicked();

    void RegisterUserInfoSlots(QVariant,int nError);

    void OnPowerCancelBtnClickedSlots();

    void on_pushButtonExit_clicked();

    void on_checkBoxSelectAllfiles_stateChanged(int arg1);

    void on_radioButton10s_clicked();

    void on_radioButton20s_clicked();

    void on_radioButton30s_clicked();

    void on_radioButtonCustomDefine_clicked();

    void on_pushButton_BacknextPage_clicked();

    void on_pushButton_nextPage_clicked();

    void StopPlayBackSlots(int,int);

    void KeyboardPressKeySlots(char);

    void ShowKeyboardSlots(int nShow);

    void on_pushButtonSetTime_clicked();

    void on_pushButtonCarriageQuery_clicked();

    void on_pushButton_Car1_clicked();

    void on_pushButton_Car2_clicked();

    void on_pushButton_Car3_clicked();

    void on_pushButton_Car4_clicked();

    void on_pushButton_Car5_clicked();

    void on_pushButton_Car6_clicked();

    void on_pushButton_Car7_clicked();

    void on_pushButton_Car8_clicked();

    void  SingleVideoWinToUserWinSlots();

    void StopRealMonitorSlots(int,int);

    void on_pushButtonCancel_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_horizontalSlider_sliderReleased();

    void on_tableWidgetFileList_itemDoubleClicked(QTableWidgetItem *item);

    void on_pushButtonSetTimeCancel_clicked();

    void on_pushButtonSysUpdate_clicked();

	void VideoPollTimeSlots();

	 
signals:
   void ChangetoSingleVideoWinSignals(QVariant);
   void SingleVideoWinPollSignals(int);
   void SingleVideoPlaySignals();
   void ChangetoLoginSignal();
   void sglUpdateState(int iState, QString sText);
   void ShowWarnSignals(QWidget *parent, QString title,  QString content);
   void SetCarNo(int iCarNo,bool bForcePlay);
private slots:
   void slotSignalFormCmd(int nCmd);
   void sltUpdateState(int iState, QString sText);

   void on_pushButtonReboot_clicked();
   void ShowWarnSlots(QWidget *parent,const QString title, const QString content);
   void ShowWarnSlotsSDK( QString title,  QString content);
   void slotSetCarNo(int iCarNo,bool bForcePlay);
protected:
    static unsigned int UpdateThreadFunc(void *param);
    unsigned int UpdateFunc();
    void CalVideoArea(int width,int height);
    void mousePressEvent(QMouseEvent *event);
private:
    Ui::UserMonitor *ui;
    int  m_iPlayBackCurrentPage;
    int  MainState;
    int  SubState;
	int  m_enumPoll;
    QTimer *timer;
    int m_iLastSelectCarriageIdx;
    int m_iCurSelectCarriageIdx;
    QTimer  *pollTimer;
    QTimer  *UpdateSaveStatusTimer;
    QTimer  *m_pUpdatePlayBackTimer;
    QTimer  *m_pUpdateConnectTimer;
	QTimer  *m_pVideoPollTimer;

    int m_iPlayState;
    int m_iOpenMediaState;
    int m_iCurPos;
    int m_iPlayRange;

    QTime m_StartTime;
    QTime m_StopTime;

    int stackWidth;
    int stackHeight;

    int m_screenWidth;
    int m_screenHeight;

    int     SearchRows;
    bool   bDownloading;
    QString  sPlayBackFileName;
    QProgressDialog *processDownloadDlg;
    QProgressDialog *processSingleFileDownloadDlg;
    QTimer *m_pSearchTimer,*m_pDownloadtimer;

    int    m_iSearchFileCnt;
    int    m_iSearchCarriageIndex;
    int    m_iSearchCameraIndex;
    int    m_iCurPlayFileIndex;
    std::vector<int> m_iSelSearchCameraIndexs;

    struct T_SEARCH_NODE
    {
        std::string sFile;
        int         iCam;
    };

    std::vector<T_SEARCH_NODE> m_aSearchFileList;

    
    int     m_nDateSetSelType;

    int CurBackSelectCarriageIdx;

    int m_iUserCount;
    CUserRegister *RegisterDlg;
    int    iVideoFormat;
    CKeyboard    *KeyBoardDlg;
    bool      bShowKeyboard;

    int     m_iSingleVideoWinIndex;
    bool    bSingleVideoWin;
    bool    bLoginout;
    bool    m_bUpdateDevStatus;
    MainSdk *m_pSdk;


public:
    QPushButton*                   m_RealCarButton[8];
    QPushButton*                   m_BackCarButton[8];
    QPushButton*                   CarNVR[8];
    QPushButton*                   CarCamera[8][32];

    DateTimeSetWidget              *m_pDateTimeWidget;
    T_WND_INFO                     m_RealMonitorVideos[VIDEO_WINDOWS_COUNT];
    T_WND_INFO                     m_tPlayBackVideo;
    int                            m_iConnectStates[MAX_CARRIAGE_NUM];

    WorkerThread    *m_pUpdateThread;
    int              m_iUpdateState;

    SingleForm      *m_pSingleform;
public:
	int  m_iPollTimerSeconds;
	POLL_TIME_TYPE m_enumPollTimeType;
	bool m_bInitPreview;
	QButtonGroup *m_btnGroupNVRCar;
	int           m_nNVRNumber;
};

#endif // USERMONITOR_H
