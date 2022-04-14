#include "UserMonitor.h"
#include "ui_UserMonitor.h"
#include <Windows.h>
#include <QMessageBox>
#include <QDir>
#include <QStorageInfo>
#if WIN32
#include<sstream>
#include <io.h>
#include<iostream>
#include<fstream>
#include "utility.h"
using namespace std;
#endif

UserMonitor::UserMonitor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserMonitor),
    m_iPlayBackCurrentPage(0),
    iVideoFormat(0),
    MainState(0),
    SubState(0),
	m_enumPoll(0),
    SearchRows(0),
    bDownloading(0),
    bShowKeyboard(0),
    bSingleVideoWin(0),
    sPlayBackFileName(""),
    bLoginout(false)
{
    ui->setupUi(this);
    QDesktopWidget *desktop=QApplication::desktop(); 
    //QRect clientRect = desktop->availableGeometry();
    //获取设备可用区宽高
    //int width=clientRect.width();
    //int height=clientRect.height();

    m_pSdk = NULL;
    memset(m_RealMonitorVideos, 0, sizeof(m_RealMonitorVideos));
    memset(&m_tPlayBackVideo, 0, sizeof(m_tPlayBackVideo));

    m_pUpdateThread = NULL;
    m_iUpdateState = -1;
	m_bInitPreview = false;

    m_pDateTimeWidget = NULL;
    m_iLastSelectCarriageIdx = 1;
    m_iCurSelectCarriageIdx = 1;
    m_iSearchCarriageIndex = -1;
    m_iSearchCameraIndex = -2;
    m_iCurPlayFileIndex = -1;
	m_iPollTimerSeconds = 30;

    m_nDateSetSelType = 0;

    m_bUpdateDevStatus = false;
	m_btnGroupNVRCar = NULL;

    m_iUserCount = 0;
	m_enumPoll = REALMONITOR_POLLOFF;
    memset(m_iConnectStates, 0, sizeof(m_iConnectStates));

	int width=(desktop)->width();
	int height=(desktop)->height();

    m_screenWidth = width;
    m_screenHeight = height;



    resize(width,height);
    setWindowFlags(Qt::FramelessWindowHint |Qt::Window);//无边框

    //setWindowFlags(Qt::FramelessWindowHint);//无边框
    //setGeometry(WINDOW_OFFSETX,WINDOW_OFFSETY,SYSTEM_WIN_WIDTH,SYSTEM_WIN_HEIGHT);

    m_pSingleform = new SingleForm();
    m_pSingleform->setGeometry(0, 0, width, height);
    connect(m_pSingleform, SIGNAL(signalCmd(int)), this, SLOT(slotSignalFormCmd(int)));

    if(desktop->width()>=1024)
    {
       iVideoFormat = VIDEO_1024_768;
    }
    else
    {
        iVideoFormat = VIDEO_800_600;
    }

    m_pSdk = MAIN_GetSdk();
	m_pSdk->SetIPCStatusChangeCallbackFun(IPCOnlineStatusChange,this);
	m_pSdk->SetSearchOverCallBack(SearchOver,this);
	m_pSdk->SetNVRDisconnectCallBack(NVRDisconnect,this);
	connect(this,SIGNAL( ShowWarnSignals(QWidget* , QString ,  QString )),this,SLOT(ShowWarnSlots(QWidget *, QString ,  QString )));
	connect(this,SIGNAL( SetCarNo(int,bool )),this,SLOT(slotSetCarNo(int,bool )));
	connect(m_pSdk,SIGNAL( ShowWarnSignalsSDK( QString ,  QString )),this,SLOT(ShowWarnSlotsSDK( QString ,  QString )));
    QStringList qss;
    qss.append(QString("%1}").arg(QString::fromLocal8Bit(" font-family:'微软雅黑';")));
    setStyleSheet(qss.join(""));

    ResizeMainFunctionButton(width,height);
    ResizeControl(width,height);
    ResizeLabel( width,height);
    ResizeButton(width,height);
    SetControlStyleSheet();

    InitParams();
    InitControl();
    SetCarrriageStatus();


    ui->stackedWidget->setCurrentIndex(0);

    InitRender();

    ui->pushButton_VideoDownload->hide();
    ui->widget_PlayCtrl->show();

    ui->dateTimeEdit_SetDT->hide();
    ui->pushButtonSetTimeCancel->hide();
	ui->pushButtonSysUpdate->hide();
	ui->lineEditCar->hide();
	ui->pushButtonCarriageQuery->hide();
	ui->label_11->hide();
    RegisterDlg  = new CUserRegister(this);
    KeyBoardDlg  = new CKeyboard(this);

    loadUserInfo();

	this->setFocusPolicy(Qt::NoFocus);
    QList<QWidget*> widgets = this->findChildren<QWidget*>();
    foreach (QWidget *pWid, widgets)
    {
		pWid->setFocusPolicy(Qt::StrongFocus);
    }
	
    ui->widget_VideoDownloadWin->setFocusPolicy(Qt::StrongFocus);
	this->setFocusPolicy(Qt::StrongFocus);
    timer =new QTimer(this);
    timer->stop();
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(ProcessTimerOut()));
    timer->start();

    m_pUpdateConnectTimer =new QTimer(this);
    m_pUpdateConnectTimer->setInterval(2000);
    connect(m_pUpdateConnectTimer, SIGNAL(timeout()), this, SLOT(ProcessConnectTimerOut()));

    pollTimer = new QTimer(this);
    pollTimer->setInterval(g_iTurnTime*1000);
    connect(pollTimer, SIGNAL(timeout()), this, SLOT(ProcessPollTimerOut()));


	//获取配置的轮询时间
	m_pVideoPollTimer = new QTimer(this);
 	m_pVideoPollTimer->setInterval(m_iPollTimerSeconds*1000);
	m_pVideoPollTimer->start();
	connect(m_pVideoPollTimer, SIGNAL(timeout()), this, SLOT(VideoPollTimeSlots()));

    m_pDownloadtimer = new QTimer(this);
    m_pDownloadtimer->setInterval(500);
    connect(m_pDownloadtimer, SIGNAL(timeout()), this, SLOT(DownloadFileTimerSlots()));

    m_pSearchTimer = new QTimer(this);
    m_pSearchTimer->setInterval(15000);
    connect(m_pSearchTimer, SIGNAL(timeout()), this, SLOT(SearchFileTimerSlots()));

    UpdateSaveStatusTimer = new QTimer(this);
    UpdateSaveStatusTimer->setInterval(2000);
    connect(UpdateSaveStatusTimer, SIGNAL(timeout()), this, SLOT(ProcessUpdateSaveStatusTimerOut()));

    m_pUpdatePlayBackTimer = new QTimer(this);
    m_pUpdatePlayBackTimer->setInterval(1000);
    connect(m_pUpdatePlayBackTimer, SIGNAL(timeout()), this, SLOT(ProcessUpdatePlayBackTimerOut()));

    connect(RegisterDlg,SIGNAL(RegisterUserInfoSignals(QVariant,int)),this,SLOT(RegisterUserInfoSlots(QVariant,int)));
    connect(RegisterDlg,SIGNAL(ShowKeyboardSignals(int)),this,SLOT(ShowKeyboardSlots(int)));
    connect(KeyBoardDlg,SIGNAL(KeyboardPressKeySignal(char)),this,SLOT(KeyboardPressKeySlots(char)));
    connect(KeyBoardDlg,SIGNAL(KeyboardPressKeySignal(char)),RegisterDlg,SLOT(InputKeySlots(char)));

    ui->lineEditPollTIme->installEventFilter(this);
    ui->lineEditCar->installEventFilter(this);


	m_nNVRNumber = 1;
	InitCarNVRButton();
}

UserMonitor::~UserMonitor()
{
    delete m_pSdk;
    m_pSdk = NULL;

    UnInitRender();

    delete ui;
}
 void UserMonitor::ShowWarnSlots(QWidget *parent, QString title,  QString content)
 {
	 showWarn(parent,title,content);
 }
int UserMonitor::showWarn(QWidget *parent, const QString &title, const QString &content)
{
    QMessageBox::warning(parent, title, content);
    this->showFullScreen();
    return 0;
}

void UserMonitor::ProcessUpdatePlayBackTimerOut()
{
    int iPlayState = -1;
    int iOpenMediaState = -1;
    m_pSdk->getPlayBackState(&iPlayState, &iOpenMediaState);
    if(m_iOpenMediaState == iOpenMediaState && m_iPlayState == iPlayState)
    {
        m_iPlayRange = m_pSdk->getPlayRange();
        m_iCurPos = m_pSdk->getPlayPos();
        ui->horizontalSlider->setRange(0, m_iPlayRange);
        ui->horizontalSlider->setValue(m_iCurPos);
        return;
    }
    m_iPlayState = iPlayState;
    m_iOpenMediaState = iOpenMediaState;
    if(iPlayState == CMP_STATE_PAUSE)
    {

    }
    else if(iPlayState == CMP_STATE_PLAY || iPlayState == CMP_STATE_FAST_FORWARD || iPlayState == CMP_STATE_SLOW_FORWARD/* || CMP_OPEN_MEDIA_SUCC == iOpenMediaState*/)
    {
        if(iVideoFormat == VIDEO_1024_768)
        {
            ui->pushButtonDownloadLastFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/pre_file_hot.png)}");
            ui->pushButtonDownloadBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/fast_back_hot.png)}");
            ui->pushButtonDownloadPlay->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play.png)}");
            ui->pushButtonDownloadStop->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop_hot.png)}");
            ui->pushButtonDownloadForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/fast_head_hot.png)}");
            ui->pushButtonDownloadNextFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/next_file_hot.png)}");
        }
        else
        {
            ui->pushButtonDownloadLastFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/pre_file_hot.png)}");
            ui->pushButtonDownloadBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/fast_back_hot.png)}");
            ui->pushButtonDownloadPlay->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play.png)}");
            ui->pushButtonDownloadStop->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop_hot.png)}");
            ui->pushButtonDownloadForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/fast_head_hot.png)}");
            ui->pushButtonDownloadNextFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/next_file_hot.png)}");
        }
    }
    else// if(iPlayState == CMP_STATE_IDLE || iPlayState == CMP_STATE_FAST_FORWARD /*CMP_OPEN_MEDIA_STREAM_FAIL == iOpenMediaState*/)
    {
        on_pushButtonDownloadStop_clicked();
    }
}

void UserMonitor::showEvent(QShowEvent *event)
{
    if(!pollTimer->isActive())
    {
        pollTimer->start();
    }
    if(!m_pUpdateConnectTimer->isActive())
    {
        m_pUpdateConnectTimer->start();
    }
}
void UserMonitor::hideEvent(QHideEvent *event)
{

}

/********************************全屏切换************************/
void UserMonitor::CalVideoArea(int width,int height)
{

}

void UserMonitor::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = mapFromGlobal(event->pos());
    int x = pos.x();
    int y = pos.y();

    if(MainState == VIDEO_REALMONITOR)
    {
        bSingleVideoWin = m_pSingleform->isVisible();
        if(bSingleVideoWin)
        {
            m_pSingleform->hide();
            m_pSingleform->Stop();
        }
        else
        {
            for(int i = 0; i < VIDEO_WINDOWS_COUNT; i++)
            {
                QWidget* pWnd = QWidget::find((WId)m_RealMonitorVideos[i].hWnd);
                if(pWnd == NULL)
                {
                    continue;
                }
                QPoint pt = ui->page->mapToGlobal(QPoint(m_RealMonitorVideos[i].nX, m_RealMonitorVideos[i].nY));
                if(x > pt.x() && x < pt.x()+m_RealMonitorVideos[i].nWidth &&
                        y > pt.y() && y <  pt.y()+  m_RealMonitorVideos[i].nHeight)
                {
                    this->hide();
                    m_pSingleform->showFullScreen();
                    m_iSingleVideoWinIndex = i;
                    bSingleVideoWin = true;
                    char acRtsp[256] = {0};
                    memset(acRtsp, 0, 256);
                    STATE_GetIpcRtsp(m_iCurSelectCarriageIdx, g_atCarriages[m_iCurSelectCarriageIdx-1].acIpcPos[i], acRtsp, 0);
					T_IPC_STATE ptState;
					T_IPC_ID pcIPCID;
					pcIPCID.cCarriageNo = m_iCurSelectCarriageIdx;
					pcIPCID.cPos = g_atCarriages[m_iCurSelectCarriageIdx-1].acIpcPos[i];
					STATE_GetIpcState(g_atCarriages[m_iCurSelectCarriageIdx-1].cNvrNo,&pcIPCID, &ptState);
					if (ptState.cOnlineState==STATE_ONLINE)
					{
						m_pSingleform->Start(acRtsp);
					}
                    
                    //qDebug() << acRtsp;
                    break;
                }
            }
        }
#if 0
        if(bSingleVideoWin)
        {
            bSingleVideoWin = false;
            for(int i = 0; i < VIDEO_WINDOWS_COUNT; i++)
            {
                QWidget* pWnd = QWidget::find((WId)m_RealMonitorVideos[i].hWnd);
                if(pWnd == NULL)
                {
                    continue;
                }
                if(i == m_iSingleVideoWinIndex)
                {
                    pWnd->setParent(ui->page);
                    pWnd->setGeometry(m_RealMonitorVideos[m_iSingleVideoWinIndex].nX, m_RealMonitorVideos[m_iSingleVideoWinIndex].nY,
                                      m_RealMonitorVideos[m_iSingleVideoWinIndex].nWidth, m_RealMonitorVideos[m_iSingleVideoWinIndex].nHeight);
                    pWnd->show();
                    m_pSdk->changePreviewWnd(m_iSingleVideoWinIndex, &m_RealMonitorVideos[m_iSingleVideoWinIndex]);
                    m_iSingleVideoWinIndex = -1;
                    ui->labelRealTime->show();
                    ui->horizontalSlider->show();
                }
                else
                {
                    pWnd->show();
                }
            }
        }
        else
        {
            for(int i = 0; i < VIDEO_WINDOWS_COUNT; i++)
            {
                QWidget* pWnd = QWidget::find((WId)m_RealMonitorVideos[i].hWnd);
                if(pWnd == NULL)
                {
                    continue;
                }
                QPoint pt = ui->page->mapToGlobal(QPoint(m_RealMonitorVideos[i].nX, m_RealMonitorVideos[i].nY));
                if(x > pt.x() && x < pt.x()+m_RealMonitorVideos[i].nWidth &&
                        y > pt.y() && y <  pt.y()+  m_RealMonitorVideos[i].nHeight)
                {
                    pWnd->setParent(this);
                    pWnd->setGeometry(0, 0, m_screenWidth, m_screenHeight);
                    pWnd->show();
                    T_WND_INFO tWndInfo;
                    memset(&tWndInfo, 0, sizeof(T_WND_INFO));
                    tWndInfo.hWnd = m_RealMonitorVideos[i].hWnd;
                    tWndInfo.nWidth = m_screenWidth;
                    tWndInfo.nHeight = m_screenHeight;
                    m_pSdk->changePreviewWnd(i, &tWndInfo);
                    m_iSingleVideoWinIndex = i;
                    bSingleVideoWin = true;
                    ui->horizontalSlider->hide();
                    ui->labelRealTime->hide();
                    break;
                }
            }
            if(m_iSingleVideoWinIndex >= 0 && m_iSingleVideoWinIndex < VIDEO_WINDOWS_COUNT)
            {
                for(int i = 0; i < VIDEO_WINDOWS_COUNT; i++)
                {
                    QWidget* pWnd = QWidget::find((WId)m_RealMonitorVideos[i].hWnd);
                    if(pWnd == NULL)
                    {
                        continue;
                    }
                    if(m_iSingleVideoWinIndex == i)
                    {
                        continue;
                    }
                    pWnd->hide();
                }
            }

        }
#endif
    }
    else if(MainState == VIDEO_PLAYBACK)
    {
        static QRect rthorizontalSlider;
        if(bSingleVideoWin)
        {
            bSingleVideoWin = false;
            ui->widget_VideoDownloadWin->setParent(ui->page3);
            ui->widget_VideoDownloadWin->setGeometry(m_tPlayBackVideo.nX, m_tPlayBackVideo.nY, m_tPlayBackVideo.nWidth, m_tPlayBackVideo.nHeight);
            ui->widget_VideoDownloadWin->show();
            m_pSdk->changePlayBackWnd(&m_tPlayBackVideo);
            ui->labelRealTime->show();
            ui->horizontalSlider->show();
        }
        else
        {
            QPoint pt = ui->page3->mapToGlobal(QPoint(m_tPlayBackVideo.nX, m_tPlayBackVideo.nY));
            if(x > pt.x() && x <  pt.x() + m_tPlayBackVideo.nWidth &&
                    y > pt.y() && y < pt.y() + m_tPlayBackVideo.nHeight)
            {


                if(rthorizontalSlider.width() <= 0 || rthorizontalSlider.height() <= 0)
                {
                    rthorizontalSlider = ui->horizontalSlider->geometry();
                }
                ui->labelRealTime->hide();
                ui->horizontalSlider->hide();

                ui->widget_VideoDownloadWin->setParent(this);
                ui->widget_VideoDownloadWin->setGeometry(0, 0, m_screenWidth, m_screenHeight);
                ui->widget_VideoDownloadWin->show();
                T_WND_INFO tWndInfo;
                tWndInfo = m_tPlayBackVideo;
                tWndInfo.nWidth = m_screenWidth;
                tWndInfo.nHeight = m_screenHeight;
                m_pSdk->changePlayBackWnd(&tWndInfo);
                bSingleVideoWin = true;
            }
        }
    }
    else if(MainState == VIDEO_DOWNLOAD)
    {


    }
}

void UserMonitor::SingleVideoWinToUserWinSlots()
{
}


/*****************************样式参数设置***********************/
void UserMonitor::SWitchVideoWinStyleSheet()
{
    if(MainState == VIDEO_REALMONITOR)
    {
        if(iVideoFormat == VIDEO_1024_768)
        {
            ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        }
        else
        {
            ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        }


    }
    else if(MainState==VIDEO_PLAYBACK)
    {
        if(iVideoFormat == VIDEO_1024_768)
        {
            ui->widget_PlayBackWin1->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_PlayBackWin2->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_PlayBackWin3->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_PlayBackWin4->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        }
        else
        {
            ui->widget_PlayBackWin1->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_PlayBackWin2->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_PlayBackWin3->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_PlayBackWin4->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        }

    }
    else if(MainState==VIDEO_DOWNLOAD)
    {
        if(iVideoFormat == VIDEO_1024_768)
        {
            ui->widget_VideoDownloadWin->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        }
        else
        {
            ui->widget_VideoDownloadWin->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中

        }
    }

}

void UserMonitor::SetMoniitorDefaultStyleSheet(int iIndex)
{
    if(iIndex < 0 || iIndex >= 4)
    {
        if(iVideoFormat == VIDEO_1024_768)
        {
            //monitor window
            ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中

        }
        else
        {
            //monitor window
            ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
            ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中

        }
    }
    else
    {
        QLabel *pLabels[] = {ui->widget_MonitorWin1,ui->widget_MonitorWin2,ui->widget_MonitorWin3,ui->widget_MonitorWin4};
        if(iVideoFormat == VIDEO_1024_768)
        {
            //monitor window
            pLabels[iIndex]->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        }
        else
        {
            pLabels[iIndex]->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        }
    }

}
//设置控件样式表
void UserMonitor::SetControlStyleSheet()
{
    if(iVideoFormat == VIDEO_1024_768)
    {

        QPixmap pixmapWin = QPixmap(":/imag/image/Monitor_new_nologo.png").scaled(this->size());
        QPalette paletteWin(this->palette());
        paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
        this->setPalette(paletteWin);

        ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image/VideoMonitor_On.png);"
                                    "background-color:rgb(5,23,89)"
                                                      "}");

        ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image/Playback.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");
        ui->pushButton_VideoDownload->setStyleSheet("QPushButton{border-image: url(:/imag/image/Download.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");

        ui->pushButton_DeviceStaus->setStyleSheet("QPushButton{border-image: url(:/imag/image/DeviceState.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");

        ui->pushButton_Update->setStyleSheet("QPushButton{border-image: url(:/imag/image/Update.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");

        ui->widget_CarChose->setStyleSheet("border:2px solid rgb(11,78,193);");
        ui->widget_PlayCtrl->setStyleSheet("border:2px solid rgb(11,78,193);");

        //monitor window
        ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中

        //label
        ui->labelCarChose->setStyleSheet("border:0px solid rgb(5,23,89);");
        ui->labelPlayCtrl->setStyleSheet("border:0px solid rgb(5,23,89);");

        ui->pushButton_Car1->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/1_on.png)}");
        ui->pushButton_Car2->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/2.png)}");
        ui->pushButton_Car3->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/3.png)}");
        ui->pushButton_Car4->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/4.png)}");
        ui->pushButton_Car5->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/5.png)}");
        ui->pushButton_Car6->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/6.png)}");
        ui->pushButton_Car7->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/7.png)}");
        ui->pushButton_Car8->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/8.png)}");

	
        ui->pushButton_prevCar->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/LastCar_on.png)}");
        ui->pushButton_nextCar->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/NextCar_on.png)}");
        ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollStart_on.png)}");
        ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollSuspend.png)}");
        ui->pushButton_pantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Pantograph.png)}");
        ui->pushButton_nextPage->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/NextPage_on.png)}");


        //viedo play back
        ui->widget_PlayBackWin1->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_PlayBackWin2->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_PlayBackWin3->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_PlayBackWin4->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_BackCarChose->setStyleSheet("border:2px solid rgb(11,78,193);");
        ui->widget_BackPlayCtrl->setStyleSheet("border:2px solid rgb(11,78,193);");

        //label
        ui->labelPlayBackCarChose->setStyleSheet("border:0px solid rgb(5,23,89);");
        ui->labelPlayBackCarCtrl->setStyleSheet("border:0px solid rgb(5,23,89);");


        ui->pushButton_BackCar1->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/1_on.png)}");
        ui->pushButton_BackCar2->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/2.png)}");
        ui->pushButton_BackCar3->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/3.png)}");
        ui->pushButton_BackCar4->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/4.png)}");
        ui->pushButton_BackCar5->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/5.png)}");
        ui->pushButton_BackCar6->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/6.png)}");
        ui->pushButton_BackCar7->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/7.png)}");
        ui->pushButton_BackCar8->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/8.png)}");


        ui->pushButton_play->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
        ui->pushButton_suspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
        ui->pushButton_back->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
        ui->pushButton_forward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");
        ui->pushButton_Backpantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Pantograph.png)}");
        ui->pushButton_BacknextPage->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/NextPage_on.png)}");
        ui->pushButton_DayPlayBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/DayPlayBack_on.png)}");
        ui->pushButton_MonthPlayBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/MonthPlayBack_on.png)}");

        ui->dateEditStartTime->setStyleSheet("border:3px solid rgb(11,78,193);");
        ui->dateEditEndTime->setStyleSheet("border:3px solid rgb(11,78,193);");
        ui->timeEditStartTime->setStyleSheet("border:3px solid rgb(11,78,193);");
        ui->timeEditEndTime->setStyleSheet("border:3px solid rgb(11,78,193);");

        //video download win
       // ui->widget_VideoDownloadWin->setStyleSheet("background:black;border:2px solid rgb(255,255,255);");
        ui->widget_VideoDownloadWin->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        //ui->widget_DownloadSet->setStyleSheet("background:white");

        //Download ctrl button
        ui->pushButtonStartQuery->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/search.png);border-radius:6px;}");
        ui->pushButtonDownload->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/download.png)}");
        ui->pushButtonDownloadLastFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/pre_file_hot.png)}");
        ui->pushButtonDownloadBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/fast_back.png)}");
        ui->pushButtonDownloadPlay->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_hot.png)}");
        ui->pushButtonDownloadStop->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
        ui->pushButtonDownloadForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/fast_head.png)}");
        ui->pushButtonDownloadNextFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/next_file_hot.png)}");


        //devicestatus ctrl button
        //ui->pushButtonDeviceStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/device_status.png)}");
        ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/device_status.png)}");
        ui->pushButtonAlarmList->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/alarm_list.png)}");

        //time label
        ui->labelRealTime->setStyleSheet("color:white;"
                                 "background-color:rgb(5,23,89);");

        //login  button
        ui->pushButtonExit->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/exit.png)}");
        ui->pushButtonCancel->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/logout.png)}");
    }
    else
    {
        QPixmap pixmapWin = QPixmap(":/imag/image_800/Monitor_new_nologo.png").scaled(this->size());
        QPalette paletteWin(this->palette());
        paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
        this->setPalette(paletteWin);

        ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/VideoMonitor_On.png);"
                                    "background-color:rgb(5,23,89)"
                                                      "}");

        ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Playback.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");
        ui->pushButton_VideoDownload->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Download.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");

        ui->pushButton_DeviceStaus->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/DeviceState.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");

        ui->pushButton_Update->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Update.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");

        ui->widget_CarChose->setStyleSheet("border:2px solid rgb(11,78,193);");
        ui->widget_PlayCtrl->setStyleSheet("border:2px solid rgb(11,78,193);");

        //monitor window
        ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中

        //label
        ui->labelCarChose->setStyleSheet("border:0px solid rgb(5,23,89);");
        ui->labelPlayCtrl->setStyleSheet("border:0px solid rgb(5,23,89);");

        ui->pushButton_Car1->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/1_on.png)}");
        ui->pushButton_Car2->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/2.png)}");
        ui->pushButton_Car3->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/3.png)}");
        ui->pushButton_Car4->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/4.png)}");
        ui->pushButton_Car5->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/5.png)}");
        ui->pushButton_Car6->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/6.png)}");
        ui->pushButton_Car7->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/7.png)}");
        ui->pushButton_Car8->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/8.png)}");

        ui->pushButton_prevCar->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/LastCar_on.png)}");
        ui->pushButton_nextCar->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/NextCar_on.png)}");
        ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollStart.png)}");
        ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollSuspend_on.png)}");
        ui->pushButton_pantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Pantograph.png)}");
        ui->pushButton_nextPage->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/NextPage_on.png)}");


        //viedo play back
        ui->widget_PlayBackWin1->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_PlayBackWin2->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_PlayBackWin3->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_PlayBackWin4->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_BackCarChose->setStyleSheet("border:2px solid rgb(11,78,193);");
        ui->widget_BackPlayCtrl->setStyleSheet("border:2px solid rgb(11,78,193);");

        //label
        ui->labelPlayBackCarChose->setStyleSheet("border:0px solid rgb(5,23,89);");
        ui->labelPlayBackCarCtrl->setStyleSheet("border:0px solid rgb(5,23,89);");

        ui->pushButton_BackCar1->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/1_on.png)}");
        ui->pushButton_BackCar2->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/2.png)}");
        ui->pushButton_BackCar3->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/3.png)}");
        ui->pushButton_BackCar4->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/4.png)}");
        ui->pushButton_BackCar5->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/5.png)}");
        ui->pushButton_BackCar6->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/6.png)}");
        ui->pushButton_BackCar7->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/7.png)}");
        ui->pushButton_BackCar8->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/8.png)}");


        ui->pushButton_play->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
        ui->pushButton_suspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
        ui->pushButton_back->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
        ui->pushButton_forward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");
        ui->pushButton_Backpantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Pantograph.png)}");
        ui->pushButton_BacknextPage->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/NextPage_on.png)}");
        ui->pushButton_DayPlayBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/DayPlayBack_on.png)}");
        ui->pushButton_MonthPlayBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/MonthPlayBack_on.png)}");

        ui->dateEditStartTime->setStyleSheet("border:3px solid rgb(11,78,193);");
        ui->dateEditEndTime->setStyleSheet("border:3px solid rgb(11,78,193);");
        ui->timeEditStartTime->setStyleSheet("border:3px solid rgb(11,78,193);");
        ui->timeEditEndTime->setStyleSheet("border:3px solid rgb(11,78,193);");

        //video download win
       // ui->widget_VideoDownloadWin->setStyleSheet("background:black;border:2px solid rgb(255,255,255);");
        ui->widget_VideoDownloadWin->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        //ui->widget_DownloadSet->setStyleSheet("background:white");

        //Download ctrl button
        ui->pushButtonStartQuery->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/search.png)}");
        ui->pushButtonDownload->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/download.png)}");
        ui->pushButtonDownloadLastFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/pre_file_hot.png)}");
        ui->pushButtonDownloadBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/fast_back.png)}");
        ui->pushButtonDownloadPlay->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_hot.png)}");
        ui->pushButtonDownloadStop->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
        ui->pushButtonDownloadForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/fast_head.png)}");
        ui->pushButtonDownloadNextFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/next_file_hot.png)}");


        //devicestatus ctrl button
//         ui->pushButtonDeviceStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/device_status.png)}");
// 		ui->pushButtonDeviceStatus->hide();
        ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/device_status.png)}");
        ui->pushButtonAlarmList->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/alarm_list.png)}");


        //time label
        ui->labelRealTime->setStyleSheet("color:white;"
                                 "background-color:rgb(5,23,89);");

        //login  button
        ui->pushButtonExit->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/exit.png)}");
        ui->pushButtonCancel->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/logout.png)}");
    }
}

//设置主控件按钮位置
void UserMonitor::ResizeMainFunctionButton(int Width,int Height)
{
     if(iVideoFormat == VIDEO_1024_768)
     {
         int MonitorButtonX = STATCKWIDGET_LEFT_MARGIN;
         int MonitorButtonY =Height *42/768 + MAIN_FUNCTION_BUTTON_GRAPS;
         ui->pushButton_VideoMonitor->move(MonitorButtonX,MonitorButtonY);
         ui->pushButton_VideoMonitor->resize(MAIN_FUNCTION_BUTTON_WIDTH,MAIN_FUNCTION_BUTTON_HEIGHT);

         int PlayBackButtonX  = MonitorButtonX +MAIN_FUNCTION_BUTTON_WIDTH +MAIN_FUNCTION_BUTTON_GRAPS;
         int PlayBackButtonY  = MonitorButtonY;
         ui->pushButton_VideoPlayback->move(PlayBackButtonX,PlayBackButtonY);
         ui->pushButton_VideoPlayback->resize(MAIN_FUNCTION_BUTTON_WIDTH,MAIN_FUNCTION_BUTTON_HEIGHT);

         //int DownloadButtonX  = PlayBackButtonX+ MAIN_FUNCTION_BUTTON_WIDTH+ MAIN_FUNCTION_BUTTON_GRAPS;
         //int DownloadButtonY  = MonitorButtonY;
         //ui->pushButton_VideoDownload->move(DownloadButtonX,PlayBackButtonY);
         //ui->pushButton_VideoDownload->resize(MAIN_FUNCTION_BUTTON_WIDTH,MAIN_FUNCTION_BUTTON_HEIGHT);

         int DeviceStausButtonX = Width - STATCKWIDGET_LEFT_MARGIN - MAIN_FUNCTION_BUTTON_WIDTH*2 - MAIN_FUNCTION_BUTTON_GRAPS;
         int DeviceStausButtonY = MonitorButtonY;
         ui->pushButton_DeviceStaus->move(DeviceStausButtonX,DeviceStausButtonY);
         ui->pushButton_DeviceStaus->resize(MAIN_FUNCTION_BUTTON_WIDTH,MAIN_FUNCTION_BUTTON_HEIGHT);

         int UpdateButtonX = Width - STATCKWIDGET_LEFT_MARGIN - MAIN_FUNCTION_BUTTON_WIDTH;
         int UpdateButtonY = MonitorButtonY;
         ui->pushButton_Update->move(UpdateButtonX,UpdateButtonY);
         ui->pushButton_Update->resize(MAIN_FUNCTION_BUTTON_WIDTH,MAIN_FUNCTION_BUTTON_HEIGHT);


     }
     else
     {
         int MonitorButtonX = STATCKWIDGET_LEFT_MARGIN_600P;
         int MonitorButtonY =Height *39/600 + MAIN_FUNCTION_BUTTON_GRAPS_600P;
         ui->pushButton_VideoMonitor->move(MonitorButtonX,MonitorButtonY);
         ui->pushButton_VideoMonitor->resize(MAIN_FUNCTION_BUTTON_WIDTH_600P,MAIN_FUNCTION_BUTTON_HEIGHT_600P);

         int PlayBackButtonX  = MonitorButtonX +MAIN_FUNCTION_BUTTON_WIDTH_600P +MAIN_FUNCTION_BUTTON_GRAPS_600P;
         int PlayBackButtonY  = MonitorButtonY;
         ui->pushButton_VideoPlayback->move(PlayBackButtonX,PlayBackButtonY);
         ui->pushButton_VideoPlayback->resize(MAIN_FUNCTION_BUTTON_WIDTH_600P,MAIN_FUNCTION_BUTTON_HEIGHT_600P);

         //int DownloadButtonX  = PlayBackButtonX+ MAIN_FUNCTION_BUTTON_WIDTH_600P+ MAIN_FUNCTION_BUTTON_GRAPS_600P;
         //int DownloadButtonY  = MonitorButtonY;
         //ui->pushButton_VideoDownload->move(DownloadButtonX,DownloadButtonY);
         //ui->pushButton_VideoDownload->resize(MAIN_FUNCTION_BUTTON_WIDTH_600P,MAIN_FUNCTION_BUTTON_HEIGHT_600P);

         int DeviceStausButtonX = Width - STATCKWIDGET_LEFT_MARGIN_600P - MAIN_FUNCTION_BUTTON_WIDTH_600P*2 - MAIN_FUNCTION_BUTTON_GRAPS_600P;
         int DeviceStausButtonY = MonitorButtonY;
         ui->pushButton_DeviceStaus->move(DeviceStausButtonX,DeviceStausButtonY);
         ui->pushButton_DeviceStaus->resize(MAIN_FUNCTION_BUTTON_WIDTH_600P,MAIN_FUNCTION_BUTTON_HEIGHT_600P);

         int UpdateButtonX = Width - STATCKWIDGET_LEFT_MARGIN_600P - MAIN_FUNCTION_BUTTON_WIDTH_600P;
         int UpdateButtonY = MonitorButtonY;
         ui->pushButton_Update->move(UpdateButtonX,UpdateButtonY);
         ui->pushButton_Update->resize(MAIN_FUNCTION_BUTTON_WIDTH_600P,MAIN_FUNCTION_BUTTON_HEIGHT_600P);

     }
}

void UserMonitor::ResizeLabel(int width,int Height)
{
    if(iVideoFormat == VIDEO_1024_768)
    {
        int RealTimeLabelX = STATCKWIDGET_LEFT_MARGIN;
        int RealTimeLabelY  =9;
        ui->labelRealTime->move(RealTimeLabelX,RealTimeLabelY);
        ui->labelRealTime->resize(TIME_LABEL_WIDTH,TIME_LABEL_HEIGHT);
    }
    else
    {
        int RealTimeLabelX = STATCKWIDGET_LEFT_MARGIN_600P;
        int RealTimeLabelY  =5;
        ui->labelRealTime->move(RealTimeLabelX,RealTimeLabelY);
        ui->labelRealTime->resize(TIME_LABEL_WIDTH_600P,TIME_LABEL_HEIGHT_600P);
    }
}

void UserMonitor::ResizeButton(int width,int Height)
{
    if(iVideoFormat == VIDEO_1024_768)
    {
        int ExitButtonX =  width - STATCKWIDGET_LEFT_MARGIN-LOGIN_BUTTON_WIDTH;
        int ExitButtonY =24;
        ui->pushButtonExit->move(ExitButtonX,ExitButtonY);
        ui->pushButtonExit->resize(LOGIN_BUTTON_WIDTH,LOGIN_BUTTON_HEIGHT);


        int CancelButtonX =  width - STATCKWIDGET_LEFT_MARGIN-LOGIN_BUTTON_WIDTH-MAIN_FUNCTION_BUTTON_GRAPS-LOGIN_BUTTON_WIDTH;
        int CancelButtonY =24;
        ui->pushButtonCancel->move(CancelButtonX,CancelButtonY);
        ui->pushButtonCancel->resize(LOGOUT_BUTTON_WIDTH,LOGOUT_BUTTON_HEIGHT);
    }
    else
    {
        int ExitButtonX =  width - STATCKWIDGET_LEFT_MARGIN_600P-LOGIN_BUTTON_WIDTH_600P;
        int ExitButtonY =24;
        ui->pushButtonExit->move(ExitButtonX,ExitButtonY);
        ui->pushButtonExit->resize(LOGIN_BUTTON_WIDTH_600P,LOGIN_BUTTON_HEIGHT_600P);

        int CancelButtonX =  width - STATCKWIDGET_LEFT_MARGIN_600P-LOGIN_BUTTON_WIDTH_600P-MAIN_FUNCTION_BUTTON_GRAPS_600P-LOGIN_BUTTON_WIDTH_600P;
        int CancelButtonY =24;
        ui->pushButtonCancel->move(CancelButtonX,CancelButtonY);
        ui->pushButtonCancel->resize(LOGOUT_BUTTON_WIDTH_600P,LOGOUT_BUTTON_HEIGHT_600P);

    }
}

//设置控件位置
void UserMonitor::ResizeControl(int Width,int Height)
{
   if(iVideoFormat == VIDEO_1024_768)
   {
       //statckwidget
       int LengthToTopMargin = Height *42/768 + MAIN_FUNCTION_BUTTON_HEIGHT+MAIN_FUNCTION_BUTTON_GRAPS*2;

       stackWidth =Width -STATCKWIDGET_LEFT_MARGIN *2;
       stackHeight =Height - LengthToTopMargin - STATCKWIDGET_BOTTOM_MARGIN;
       ui->stackedWidget->move(STATCKWIDGET_LEFT_MARGIN,LengthToTopMargin);
       ui->stackedWidget->resize(stackWidth,stackHeight);

       //video monitor label
       int labelHeight = (stackHeight -GRAPS_BETWEEN_CONTROLS*4-CARRIAGE_SELECT_WIDGET_HEIGHT)/2;
       int labelWidth = (stackWidth -GRAPS_BETWEEN_CONTROLS*3)/2;

        //label1
       int label1X = GRAPS_BETWEEN_CONTROLS;
       int label1Y = GRAPS_BETWEEN_CONTROLS;
       ui->widget_MonitorWin1->move(GRAPS_BETWEEN_CONTROLS,GRAPS_BETWEEN_CONTROLS);
       ui->widget_MonitorWin1->resize(labelWidth,labelHeight);

       //label2
        int label2X  = label1X +labelWidth + GRAPS_BETWEEN_CONTROLS;
        int label2Y =  label1Y;
        ui->widget_MonitorWin2->move(label2X,label2Y);
        ui->widget_MonitorWin2->resize(labelWidth,labelHeight);

        //label3
        int label3X = label1X;
        int label3Y = label1Y +labelHeight+ GRAPS_BETWEEN_CONTROLS;
        ui->widget_MonitorWin3->move(label3X,label3Y);
        ui->widget_MonitorWin3->resize(labelWidth,labelHeight);

        //label4
        int label4X = label2X;
        int label4Y = label3Y;
        ui->widget_MonitorWin4->move(label4X,label4Y);
        ui->widget_MonitorWin4->resize(labelWidth,labelHeight);

        //carriage select widget
        int carChoseWidgetX =label3X;
        int carChoseWidgetY =label3Y +labelHeight+GRAPS_BETWEEN_CONTROLS;
        ui->widget_CarChose->move(carChoseWidgetX,carChoseWidgetY);
        ui->widget_CarChose->resize(labelWidth,CARRIAGE_SELECT_WIDGET_HEIGHT);

        //label
        int carChoseLabelX = (labelWidth-REALMONITOR_LABEL_WIDTH)/2;
        int carChoseLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelCarChose->move(carChoseLabelX,carChoseLabelY);
        ui->labelCarChose->resize(REALMONITOR_LABEL_WIDTH,LABEL_HEIGHT);

        //carriage select button
        int car1ChoseButtonX = (labelWidth -BUTTON_WIDTH*8-GRAPS_BETWEEN_CONTROLS*7)/2;
        int car1ChoseButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN;
        ui->pushButton_Car1->move(car1ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car1->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int car2ChoseButtonX = car1ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car2->move(car2ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car2->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int car3ChoseButtonX = car2ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car3->move(car3ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car3->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int car4ChoseButtonX = car3ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car4->move(car4ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car4->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int car5ChoseButtonX = car4ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car5->move(car5ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car5->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int car6ChoseButtonX = car5ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car6->move(car6ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car6->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int car7ChoseButtonX = car6ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car7->move(car7ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car7->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int car8ChoseButtonX = car7ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car8->move(car8ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car8->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        //realMonitor control widget
        int playCtrlWidgetX = carChoseWidgetX + labelWidth + GRAPS_BETWEEN_CONTROLS;
        int playCtrlWidgetY = carChoseWidgetY;
        ui->widget_PlayCtrl->move(playCtrlWidgetX,playCtrlWidgetY);
        ui->widget_PlayCtrl->resize(labelWidth,CARRIAGE_SELECT_WIDGET_HEIGHT);

        //label
        int carControlLabelX = (labelWidth-REALMONITOR_LABEL_WIDTH)/2;
        int carControlLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelPlayCtrl->move(carControlLabelX,carControlLabelY);
        ui->labelPlayCtrl->resize(REALMONITOR_LABEL_WIDTH,LABEL_HEIGHT);

        //control button
        int prevCarButtonX  = (labelWidth -BUTTON_WIDTH*6-GRAPS_BETWEEN_CONTROLS*5)/2;
        int prevCarButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN;
        ui->pushButton_prevCar->move(prevCarButtonX,prevCarButtonY);
        ui->pushButton_prevCar->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int nextCarButtonX  = prevCarButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int nextCarButtonY = prevCarButtonY;
        ui->pushButton_nextCar->move(nextCarButtonX,nextCarButtonY);
        ui->pushButton_nextCar->resize(BUTTON_WIDTH,BUTTON_HEIGHT);


        int pollStartButtonX = nextCarButtonX +  BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int pollStartButtonY = nextCarButtonY;
        ui->pushButton_pollstart->move(pollStartButtonX,pollStartButtonY);
        ui->pushButton_pollstart->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int pollSuspendButtonX = pollStartButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int pollSuspendButtonY = nextCarButtonY;
        ui->pushButton_pollsuspend->move(pollSuspendButtonX,pollSuspendButtonY);
        ui->pushButton_pollsuspend->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int pantographButtonX = pollSuspendButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int pantographButtonY = pollSuspendButtonY;
        ui->pushButton_pantograph->move(pantographButtonX,pantographButtonY);
        ui->pushButton_pantograph->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int NextPageButtonX = pantographButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int NextPageButtonY = pantographButtonY;
        ui->pushButton_nextPage->move(NextPageButtonX,NextPageButtonY);
        ui->pushButton_nextPage->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

         //------------------------------------------
        //playback label
        int PlayBacklabelHeight = (stackHeight -GRAPS_BETWEEN_CONTROLS*4-PLAYBACK_CARRIAGE_SELECT_WIDGET_HEIGHT)/2;
        int PlayBacklabelWidth = (stackWidth -GRAPS_BETWEEN_CONTROLS*3)/2;

       int PlayBacklabel1X = GRAPS_BETWEEN_CONTROLS;
       int PlayBacklabel1Y = GRAPS_BETWEEN_CONTROLS;
       ui->widget_PlayBackWin1->move(PlayBacklabel1X,PlayBacklabel1Y);
       ui->widget_PlayBackWin1->resize(PlayBacklabelWidth,PlayBacklabelHeight);

       //label2
        int PlayBacklabel2X  = PlayBacklabel1X +PlayBacklabelWidth + GRAPS_BETWEEN_CONTROLS;
        int PlayBacklabel2Y =  PlayBacklabel1Y;
        ui->widget_PlayBackWin2->move(PlayBacklabel2X,PlayBacklabel2Y);
        ui->widget_PlayBackWin2->resize(PlayBacklabelWidth,PlayBacklabelHeight);

        //label3
        int PlayBacklabel3X = PlayBacklabel1X;
        int PlayBacklabel3Y = PlayBacklabel1Y +PlayBacklabelHeight+ GRAPS_BETWEEN_CONTROLS;
        ui->widget_PlayBackWin3->move(PlayBacklabel3X,PlayBacklabel3Y);
        ui->widget_PlayBackWin3->resize(PlayBacklabelWidth,PlayBacklabelHeight);

        //label4
        int PlayBacklabel4X = PlayBacklabel2X;
        int PlayBacklabel4Y = PlayBacklabel3Y;
        ui->widget_PlayBackWin4->move(PlayBacklabel4X,PlayBacklabel4Y);
        ui->widget_PlayBackWin4->resize(PlayBacklabelWidth,PlayBacklabelHeight);

        //playback carriage  select widget
        int PlayBackCarChoseWidgetX =PlayBacklabel3X;
        int PlayBackCarChoseWidgetY =PlayBacklabel3Y +PlayBacklabelHeight+GRAPS_BETWEEN_CONTROLS;
        ui->widget_BackCarChose->move(PlayBackCarChoseWidgetX,PlayBackCarChoseWidgetY);
        ui->widget_BackCarChose->resize(PlayBacklabelWidth,PLAYBACK_CARRIAGE_SELECT_WIDGET_HEIGHT);

        //label
        int PlayBackCarControlLabelX = (PlayBacklabelWidth-PLAYBACK_LABEL_WIDTH)/2;
        int PlayBackCarControlLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelPlayBackCarChose->move(PlayBackCarControlLabelX,PlayBackCarControlLabelY);
        ui->labelPlayBackCarChose->resize(PLAYBACK_LABEL_WIDTH,LABEL_HEIGHT);

       //playback select carriage button
        //carriage select button
        int PlayBackCar1ChoseButtonX = labelWidth -BUTTON_WIDTH*4-GRAPS_BETWEEN_CONTROLS*4;
        int PlayBackCar1ChoseButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN;
        ui->pushButton_BackCar1->move(PlayBackCar1ChoseButtonX,PlayBackCar1ChoseButtonY);
        ui->pushButton_BackCar1->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackCar2ChoseButtonX = PlayBackCar1ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_BackCar2->move(PlayBackCar2ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_BackCar2->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackCar3ChoseButtonX = PlayBackCar2ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_BackCar3->move(PlayBackCar3ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_BackCar3->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackCar4ChoseButtonX = PlayBackCar3ChoseButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_BackCar4->move(PlayBackCar4ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_BackCar4->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackCar5ChoseButtonX = PlayBackCar1ChoseButtonX;
        int PlayBackCar5ChoseButtonY = PlayBackCar1ChoseButtonY + BUTTON_HEIGHT + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_BackCar5->move(PlayBackCar5ChoseButtonX,PlayBackCar5ChoseButtonY);
        ui->pushButton_BackCar5->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackCar6ChoseButtonX = PlayBackCar2ChoseButtonX;
        int PlayBackCar6ChoseButtonY =PlayBackCar5ChoseButtonY;
        ui->pushButton_BackCar6->move(PlayBackCar6ChoseButtonX,PlayBackCar6ChoseButtonY);
        ui->pushButton_BackCar6->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackCar7ChoseButtonX = PlayBackCar3ChoseButtonX;
        int PlayBackCar7ChoseButtonY =PlayBackCar5ChoseButtonY;
        ui->pushButton_BackCar7->move(PlayBackCar7ChoseButtonX,PlayBackCar7ChoseButtonY);
        ui->pushButton_BackCar7->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackCar8ChoseButtonX = PlayBackCar4ChoseButtonX;
        int PlayBackCar8ChoseButtonY =PlayBackCar5ChoseButtonY;
        ui->pushButton_BackCar8->move(PlayBackCar8ChoseButtonX,PlayBackCar8ChoseButtonY);
        ui->pushButton_BackCar8->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        //playback dataedit
        int StartTimeEditX  =PlayBackCar1ChoseButtonX - GRAPS_BETWEEN_CONTROLS -TIMEEDIT_WIDTH;
        int StartTimeEditY  = BUTTON_TO_CHOSEWIDGET_MARGIN+ GRAPS_BETWEEN_CONTROLS;
        ui->timeEditStartTime->move(StartTimeEditX,StartTimeEditY);
        ui->timeEditStartTime->resize(TIMEEDIT_WIDTH,TIMEEDIT_HEIGHT);

        int EndTimeEditX  =PlayBackCar1ChoseButtonX - GRAPS_BETWEEN_CONTROLS -TIMEEDIT_WIDTH;
        int EndTimeEditY  = PlayBackCar5ChoseButtonY+ GRAPS_BETWEEN_CONTROLS;
        ui->timeEditEndTime->move(EndTimeEditX,EndTimeEditY);
        ui->timeEditEndTime->resize(TIMEEDIT_WIDTH,TIMEEDIT_HEIGHT);

        int StartEditX = StartTimeEditX - GRAPS_BETWEEN_CONTROLS -DATAEDIT_WIDTH;
        int StartEditY  = BUTTON_TO_CHOSEWIDGET_MARGIN+ GRAPS_BETWEEN_CONTROLS;
        ui->dateEditStartTime->move(StartEditX,StartEditY);
        ui->dateEditStartTime->resize(DATAEDIT_WIDTH,DATAEDIT_HEIGHT);

        int EndEditX = StartEditX;
        int EndEditY  = PlayBackCar5ChoseButtonY +GRAPS_BETWEEN_CONTROLS;
        ui->dateEditEndTime->move(EndEditX,EndEditY);
        ui->dateEditEndTime->resize(DATAEDIT_WIDTH,DATAEDIT_HEIGHT);


        //playback control widget
        int PlayBackCtrlWidgetX = PlayBackCarChoseWidgetX + PlayBacklabelWidth + GRAPS_BETWEEN_CONTROLS;
        int PlayBackCtrlWidgetY = PlayBackCarChoseWidgetY;
        ui->widget_BackPlayCtrl->move(PlayBackCtrlWidgetX,PlayBackCtrlWidgetY);
        ui->widget_BackPlayCtrl->resize(PlayBacklabelWidth,PLAYBACK_CARRIAGE_SELECT_WIDGET_HEIGHT);

        //label
        int PlayBackPlayControlLabelX = (PlayBacklabelWidth-REALMONITOR_LABEL_WIDTH)/2;
        int PlayBackPlayControlLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelPlayBackCarCtrl->move(PlayBackPlayControlLabelX,PlayBackPlayControlLabelY);
        ui->labelPlayBackCarCtrl->resize(REALMONITOR_LABEL_WIDTH,LABEL_HEIGHT);

        //playback control button
        int StartButtonX  = (PlayBacklabelWidth -BUTTON_WIDTH*6-GRAPS_BETWEEN_CONTROLS*5)/2;
        int StartButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN;
        ui->pushButton_play->move(StartButtonX,StartButtonY);
        ui->pushButton_play->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int SuspendButtonX  = StartButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int SuspendButtonY = StartButtonY;
        ui->pushButton_suspend->move(SuspendButtonX,SuspendButtonY);
        ui->pushButton_suspend->resize(BUTTON_WIDTH,BUTTON_HEIGHT);


        int BackButtonX = SuspendButtonX +  BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int BackButtonY = SuspendButtonY;
        ui->pushButton_back->move(BackButtonX,BackButtonY);
        ui->pushButton_back->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int ForwardButtonX = BackButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int ForwardButtonY = BackButtonY;
        ui->pushButton_forward->move(ForwardButtonX,ForwardButtonY);
        ui->pushButton_forward->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackpantographButtonX = ForwardButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int PlayBackpantographButtonY = ForwardButtonY;
        ui->pushButton_Backpantograph->move(PlayBackpantographButtonX,PlayBackpantographButtonY);
        ui->pushButton_Backpantograph->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackNextPageButtonX = PlayBackpantographButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int PlayBackNextPageButtonY = PlayBackpantographButtonY;
        ui->pushButton_BacknextPage->move(PlayBackNextPageButtonX,PlayBackNextPageButtonY);
        ui->pushButton_BacknextPage->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int DayCheckX = StartButtonX;
        int DayCheckY = StartButtonY+ BUTTON_HEIGHT+ GRAPS_BETWEEN_CONTROLS;
        int DayCheckButtonWidth = BUTTON_WIDTH*3+GRAPS_BETWEEN_CONTROLS*2;
        ui->pushButton_DayPlayBack->move(DayCheckX,DayCheckY);
        ui->pushButton_DayPlayBack->resize(DayCheckButtonWidth,BUTTON_HEIGHT);

        int MonthCheckX = ForwardButtonX;
        int MonthCheckY = DayCheckY;
        ui->pushButton_MonthPlayBack->move(MonthCheckX,MonthCheckY);
        ui->pushButton_MonthPlayBack->resize(DayCheckButtonWidth,BUTTON_HEIGHT);

        //video download

        //download set widget
        int setWidgetX = GRAPS_BETWEEN_CONTROLS;
        int setWidgetY  = GRAPS_BETWEEN_CONTROLS;
        int setWidgetHeight =stackHeight - BUTTON_HEIGHT - GRAPS_BETWEEN_CONTROLS*3;
        ui->widget_DownloadSet->move(setWidgetX,setWidgetY);
        ui->widget_DownloadSet->resize(DOWNLOAD_SET_WIDGET_WIDTH,setWidgetHeight);

      //  int videoWidgetX = GRAPS_BETWEEN_CONTROLS + DOWNLOAD_SET_WIDGET_WIDTH + GRAPS_BETWEEN_CONTROLS;
     //   int videoWidgetY = GRAPS_BETWEEN_CONTROLS;
     //   int videoWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*3 - DOWNLOAD_SET_WIDGET_WIDTH;
     //   int videoWidgetHeight = setWidgetHeight;
    //    ui->widget_VideoDownloadWin->move(videoWidgetX,videoWidgetY);
     //   ui->widget_VideoDownloadWin->resize(videoWidgetWidth,videoWidgetHeight);

          int videoWidgetX = GRAPS_BETWEEN_CONTROLS + DOWNLOAD_SET_WIDGET_WIDTH + GRAPS_BETWEEN_CONTROLS;
          int videoWidgetY = GRAPS_BETWEEN_CONTROLS;
          int videoWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*3 - DOWNLOAD_SET_WIDGET_WIDTH;
          int videoWidgetHeight = setWidgetHeight - PlayBackByFileSliderHeight - GRAPS_BETWEEN_CONTROLS*2;
          ui->widget_VideoDownloadWin->move(videoWidgetX,videoWidgetY);
          ui->widget_VideoDownloadWin->resize(videoWidgetWidth,videoWidgetHeight);

        int CtrlWidgetX = GRAPS_BETWEEN_CONTROLS;
        int CtrlWidgetY = GRAPS_BETWEEN_CONTROLS + setWidgetHeight + GRAPS_BETWEEN_CONTROLS;
        int CtrlWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*2;
        int CtrlHeight =stackHeight - GRAPS_BETWEEN_CONTROLS*2-setWidgetHeight;
        ui->widgetDownloadCtrl->move(CtrlWidgetX,CtrlWidgetY);
        ui->widgetDownloadCtrl->resize(CtrlWidth,CtrlHeight);

        //Download control button
        ui->pushButtonStartQuery->resize(FILE_SEARCH_BUTTON_WIDTH,FILE_SEARCH_BUTTON_HEIGHT);

        int DownloadButtonX = ( DOWNLOAD_SET_WIDGET_WIDTH - FILE_DOWNLOAD_BUTTON_WIDTH)/2;
        int DownloadButtonY = GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownload->move(DownloadButtonX,DownloadButtonY);
        ui->pushButtonDownload->resize(FILE_DOWNLOAD_BUTTON_WIDTH,FILE_DOWNLOAD_BUTTON_HEIGHT);

        int DownloadLastButtonX = videoWidgetX +(videoWidgetWidth-BUTTON_WIDTH*6-GRAPS_BETWEEN_CONTROLS*5)/2;
        ui->pushButtonDownloadLastFile->move(DownloadLastButtonX,DownloadButtonY);
        ui->pushButtonDownloadLastFile->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int DownloadBackX = DownloadLastButtonX + BUTTON_WIDTH+GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadBack->move(DownloadBackX,DownloadButtonY);
        ui->pushButtonDownloadBack->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int DownloadPlayX = DownloadBackX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadPlay->move(DownloadPlayX,DownloadButtonY);
        ui->pushButtonDownloadPlay->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int DownloadStopX = DownloadPlayX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadStop ->move(DownloadStopX,DownloadButtonY);
        ui->pushButtonDownloadStop->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int DownloadForwardX =DownloadStopX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadForward ->move(DownloadForwardX,DownloadButtonY);
        ui->pushButtonDownloadForward->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int DownloadNextX =DownloadForwardX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadNextFile ->move(DownloadNextX,DownloadButtonY);
        ui->pushButtonDownloadNextFile->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        //horizontalSlider

        int SliderX = GRAPS_BETWEEN_CONTROLS + DOWNLOAD_SET_WIDGET_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int SliderY = setWidgetHeight - PlayBackByFileSliderHeight - GRAPS_BETWEEN_CONTROLS;
        int SliderWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*3 - DOWNLOAD_SET_WIDGET_WIDTH;
        int SliderHeight = PlayBackByFileSliderHeight;
        ui->horizontalSlider->move(SliderX,SliderY);
        ui->horizontalSlider->resize(SliderWidth,SliderHeight);


        //-----------------------------------------------------------
        //device  status

        int DeviceStatusX = stackWidth - MAIN_FUNCTION_BUTTON_GRAPS -DEVICE_STATUS_BUTTON_WIDTH*3;
        int DeviceStatusY = MAIN_FUNCTION_BUTTON_GRAPS;
//         ui->pushButtonDeviceStatus ->move(DeviceStatusX,DeviceStatusY);
//         ui->pushButtonDeviceStatus->resize(DEVICE_STATUS_BUTTON_WIDTH,DEVICE_STATUS_BUTTON_HEIGHT);


        int SaveStatusX  = stackWidth - MAIN_FUNCTION_BUTTON_GRAPS -DEVICE_STATUS_BUTTON_WIDTH*2;
        int SaveStatusY  = MAIN_FUNCTION_BUTTON_GRAPS;
        ui->pushButtonSaveStatus ->move(SaveStatusX,SaveStatusY);
        ui->pushButtonSaveStatus->resize(DEVICE_STATUS_BUTTON_WIDTH,DEVICE_STATUS_BUTTON_HEIGHT);

        int AlarmListStatusX  = stackWidth - MAIN_FUNCTION_BUTTON_GRAPS -DEVICE_STATUS_BUTTON_WIDTH;
        int AlarmListStatusY  = MAIN_FUNCTION_BUTTON_GRAPS;
        ui->pushButtonAlarmList ->move(AlarmListStatusX,AlarmListStatusY);
        ui->pushButtonAlarmList->resize(DEVICE_STATUS_BUTTON_WIDTH,DEVICE_STATUS_BUTTON_HEIGHT);


        int DeviceStatusStackWidgetX   =  GRAPS_BETWEEN_CONTROLS;
        int DeviceStatusStackWidgetY  = MAIN_FUNCTION_BUTTON_GRAPS + DEVICE_STATUS_BUTTON_HEIGHT+ GRAPS_BETWEEN_CONTROLS;
        int DeviceStatusStackWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*2;
        int DeviceStatusStackWidgetHeight = stackHeight - DeviceStatusStackWidgetY - GRAPS_BETWEEN_CONTROLS;
        ui->stackedWidget2 ->move(DeviceStatusStackWidgetX,DeviceStatusStackWidgetY);
        ui->stackedWidget2->resize(DeviceStatusStackWidgetWidth,DeviceStatusStackWidgetHeight);


        int SaveStatusTableWidgetX  = GRAPS_BETWEEN_CONTROLS;
        int SaveStatusTableWidgetY  = GRAPS_BETWEEN_CONTROLS;
        int SaveStatusTableWidgetWidth = DeviceStatusStackWidgetWidth -GRAPS_BETWEEN_CONTROLS*2;
        int SaveStatusTableWidgetHeight =DeviceStatusStackWidgetHeight - GRAPS_BETWEEN_CONTROLS*2;
        ui->tableWidgetSaveStatus ->move(SaveStatusTableWidgetX,SaveStatusTableWidgetY);
        ui->tableWidgetSaveStatus->resize(SaveStatusTableWidgetWidth,SaveStatusTableWidgetHeight);
       //ui->tableWidgetSaveStatus->setMinimumSize(SaveStatusTableWidgetWidth,SaveStatusTableWidgetHeight);
       // ui->tableWidgetSaveStatus->setMinimumHeight(SaveStatusTableWidgetHeight-30);


        int AlarmListTableWidgetX  = GRAPS_BETWEEN_CONTROLS;
        int AlarmListTableWidgetY  = GRAPS_BETWEEN_CONTROLS;
        int AlarmListWidgetWidth = DeviceStatusStackWidgetWidth - GRAPS_BETWEEN_CONTROLS*2;
        int AlarmListWidgetHeight =DeviceStatusStackWidgetHeight - GRAPS_BETWEEN_CONTROLS*2;
        ui->tableWidgetAlarmList ->move(AlarmListTableWidgetX,AlarmListTableWidgetY);
        ui->tableWidgetAlarmList->resize(AlarmListWidgetWidth,AlarmListWidgetHeight);


        //**************************update*****************
        int setUpdateWidgetX = GRAPS_BETWEEN_CONTROLS;
        int setUpdateWidgetY  = GRAPS_BETWEEN_CONTROLS;
        int setUpdateWidgetHeight =stackHeight -STATCKWIDGET_LEFT_MARGIN- GRAPS_BETWEEN_CONTROLS;
        ui->widgetUpdate->move(setUpdateWidgetX,setUpdateWidgetY);
        ui->widgetUpdate->resize(DOWNLOAD_SET_WIDGET_WIDTH,setUpdateWidgetHeight);

        int setUpdateTableWidgetX = GRAPS_BETWEEN_CONTROLS + DOWNLOAD_SET_WIDGET_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int setUpdateTableWidgetY = GRAPS_BETWEEN_CONTROLS;
        int setUpdateTableWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*3 - DOWNLOAD_SET_WIDGET_WIDTH;
        int setUpdateTableWidgetHeight = stackHeight -STATCKWIDGET_LEFT_MARGIN- GRAPS_BETWEEN_CONTROLS;
        ui->tableWidgetPower->move(setUpdateTableWidgetX,setUpdateTableWidgetY);
        ui->tableWidgetPower->resize(setUpdateTableWidgetWidth,setUpdateTableWidgetHeight);


   }
   else
   {
       //statckwidget
       int LengthToTopMargin = Height *39/600 + MAIN_FUNCTION_BUTTON_HEIGHT_600P+MAIN_FUNCTION_BUTTON_GRAPS_600P*2;

       stackWidth =Width -STATCKWIDGET_LEFT_MARGIN_600P *2;
       stackHeight =Height - LengthToTopMargin - STATCKWIDGET_BOTTOM_MARGIN_600P;
       ui->stackedWidget->move(STATCKWIDGET_LEFT_MARGIN_600P,LengthToTopMargin);
       ui->stackedWidget->resize(stackWidth,stackHeight);

       //video monitor label
       int labelHeight = (stackHeight -GRAPS_BETWEEN_CONTROLS*4-CARRIAGE_SELECT_WIDGET_HEIGHT_600P)/2;
       int labelWidth = (stackWidth -GRAPS_BETWEEN_CONTROLS*3)/2;

        //label1
       int label1X = GRAPS_BETWEEN_CONTROLS;
       int label1Y = GRAPS_BETWEEN_CONTROLS;
       ui->widget_MonitorWin1->move(GRAPS_BETWEEN_CONTROLS,GRAPS_BETWEEN_CONTROLS);
       ui->widget_MonitorWin1->resize(labelWidth,labelHeight);

       //label2
        int label2X  = label1X +labelWidth + GRAPS_BETWEEN_CONTROLS;
        int label2Y =  label1Y;
        ui->widget_MonitorWin2->move(label2X,label2Y);
        ui->widget_MonitorWin2->resize(labelWidth,labelHeight);

        //label3
        int label3X = label1X;
        int label3Y = label1Y +labelHeight+ GRAPS_BETWEEN_CONTROLS;
        ui->widget_MonitorWin3->move(label3X,label3Y);
        ui->widget_MonitorWin3->resize(labelWidth,labelHeight);

        //label4
        int label4X = label2X;
        int label4Y = label3Y;
        ui->widget_MonitorWin4->move(label4X,label4Y);
        ui->widget_MonitorWin4->resize(labelWidth,labelHeight);

        //carriage select widget
        int carChoseWidgetX =label3X;
        int carChoseWidgetY =label3Y +labelHeight+GRAPS_BETWEEN_CONTROLS;
        ui->widget_CarChose->move(carChoseWidgetX,carChoseWidgetY);
        ui->widget_CarChose->resize(labelWidth,CARRIAGE_SELECT_WIDGET_HEIGHT_600P);

        //label
        int carChoseLabelX = (labelWidth-REALMONITOR_LABEL_WIDTH_600P)/2;
        int carChoseLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelCarChose->move(carChoseLabelX,carChoseLabelY);
        ui->labelCarChose->resize(REALMONITOR_LABEL_WIDTH_600P,LABEL_HEIGHT_600P);

        //carriage select button
        int car1ChoseButtonX = (labelWidth -BUTTON_WIDTH_600P*8-GRAPS_BETWEEN_CONTROLS*7)/2;
        int car1ChoseButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN_600P;
        ui->pushButton_Car1->move(car1ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car1->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int car2ChoseButtonX = car1ChoseButtonX + BUTTON_WIDTH_600P+ GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car2->move(car2ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car2->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int car3ChoseButtonX = car2ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car3->move(car3ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car3->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int car4ChoseButtonX = car3ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car4->move(car4ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car4->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int car5ChoseButtonX = car4ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car5->move(car5ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car5->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int car6ChoseButtonX = car5ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car6->move(car6ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car6->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int car7ChoseButtonX = car6ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car7->move(car7ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car7->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int car8ChoseButtonX = car7ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_Car8->move(car8ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_Car8->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        //realMonitor control widget
        int playCtrlWidgetX = carChoseWidgetX + labelWidth + GRAPS_BETWEEN_CONTROLS;
        int playCtrlWidgetY = carChoseWidgetY;
        ui->widget_PlayCtrl->move(playCtrlWidgetX,playCtrlWidgetY);
        ui->widget_PlayCtrl->resize(labelWidth,CARRIAGE_SELECT_WIDGET_HEIGHT_600P);

        //label
        int carControlLabelX = (labelWidth-REALMONITOR_LABEL_WIDTH_600P)/2;
        int carControlLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelPlayCtrl->move(carControlLabelX,carControlLabelY);
        ui->labelPlayCtrl->resize(REALMONITOR_LABEL_WIDTH_600P,LABEL_HEIGHT_600P);

        //control button
        int prevCarButtonX  = (labelWidth -BUTTON_WIDTH_600P*6-GRAPS_BETWEEN_CONTROLS*5)/2;
        int prevCarButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN_600P;
        ui->pushButton_prevCar->move(prevCarButtonX,prevCarButtonY);
        ui->pushButton_prevCar->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int nextCarButtonX  = prevCarButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int nextCarButtonY = prevCarButtonY;
        ui->pushButton_nextCar->move(nextCarButtonX,nextCarButtonY);
        ui->pushButton_nextCar->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);


        int pollStartButtonX = nextCarButtonX +  BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int pollStartButtonY = nextCarButtonY;
        ui->pushButton_pollstart->move(pollStartButtonX,pollStartButtonY);
        ui->pushButton_pollstart->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int pollSuspendButtonX = pollStartButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int pollSuspendButtonY = nextCarButtonY;
        ui->pushButton_pollsuspend->move(pollSuspendButtonX,pollSuspendButtonY);
        ui->pushButton_pollsuspend->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int pantographButtonX = pollSuspendButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int pantographButtonY = pollSuspendButtonY;
        ui->pushButton_pantograph->move(pantographButtonX,pantographButtonY);
        ui->pushButton_pantograph->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int NextPageButtonX = pantographButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int NextPageButtonY = pantographButtonY;
        ui->pushButton_nextPage->move(NextPageButtonX,NextPageButtonY);
        ui->pushButton_nextPage->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

         //------------------------------------------
        //playback label
        int PlayBacklabelHeight = (stackHeight -GRAPS_BETWEEN_CONTROLS*4-PLAYBACK_CARRIAGE_SELECT_WIDGET_HEIGHT_600P)/2;
        int PlayBacklabelWidth = (stackWidth -GRAPS_BETWEEN_CONTROLS*3)/2;

       int PlayBacklabel1X = GRAPS_BETWEEN_CONTROLS;
       int PlayBacklabel1Y = GRAPS_BETWEEN_CONTROLS;
       ui->widget_PlayBackWin1->move(PlayBacklabel1X,PlayBacklabel1Y);
       ui->widget_PlayBackWin1->resize(PlayBacklabelWidth,PlayBacklabelHeight);

       //label2
        int PlayBacklabel2X  = PlayBacklabel1X +PlayBacklabelWidth + GRAPS_BETWEEN_CONTROLS;
        int PlayBacklabel2Y =  PlayBacklabel1Y;
        ui->widget_PlayBackWin2->move(PlayBacklabel2X,PlayBacklabel2Y);
        ui->widget_PlayBackWin2->resize(PlayBacklabelWidth,PlayBacklabelHeight);

        //label3
        int PlayBacklabel3X = PlayBacklabel1X;
        int PlayBacklabel3Y = PlayBacklabel1Y +PlayBacklabelHeight+ GRAPS_BETWEEN_CONTROLS;
        ui->widget_PlayBackWin3->move(PlayBacklabel3X,PlayBacklabel3Y);
        ui->widget_PlayBackWin3->resize(PlayBacklabelWidth,PlayBacklabelHeight);

        //label4
        int PlayBacklabel4X = PlayBacklabel2X;
        int PlayBacklabel4Y = PlayBacklabel3Y;
        ui->widget_PlayBackWin4->move(PlayBacklabel4X,PlayBacklabel4Y);
        ui->widget_PlayBackWin4->resize(PlayBacklabelWidth,PlayBacklabelHeight);

        //playback carriage  select widget
        int PlayBackCarChoseWidgetX =PlayBacklabel3X;
        int PlayBackCarChoseWidgetY =PlayBacklabel3Y +PlayBacklabelHeight+GRAPS_BETWEEN_CONTROLS;
        ui->widget_BackCarChose->move(PlayBackCarChoseWidgetX,PlayBackCarChoseWidgetY);
        ui->widget_BackCarChose->resize(PlayBacklabelWidth,PLAYBACK_CARRIAGE_SELECT_WIDGET_HEIGHT_600P);

        //label
        int PlayBackCarControlLabelX = (PlayBacklabelWidth-PLAYBACK_LABEL_WIDTH_600P)/2;
        int PlayBackCarControlLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelPlayBackCarChose->move(PlayBackCarControlLabelX,PlayBackCarControlLabelY);
        ui->labelPlayBackCarChose->resize(PLAYBACK_LABEL_WIDTH_600P,LABEL_HEIGHT_600P);

       //playback select carriage button
        //carriage select button
        int PlayBackCar1ChoseButtonX = labelWidth -BUTTON_WIDTH_600P*4-GRAPS_BETWEEN_CONTROLS*4;
        int PlayBackCar1ChoseButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN_600P;
        ui->pushButton_BackCar1->move(PlayBackCar1ChoseButtonX,PlayBackCar1ChoseButtonY);
        ui->pushButton_BackCar1->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackCar2ChoseButtonX = PlayBackCar1ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_BackCar2->move(PlayBackCar2ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_BackCar2->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackCar3ChoseButtonX = PlayBackCar2ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_BackCar3->move(PlayBackCar3ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_BackCar3->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackCar4ChoseButtonX = PlayBackCar3ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_BackCar4->move(PlayBackCar4ChoseButtonX,car1ChoseButtonY);
        ui->pushButton_BackCar4->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackCar5ChoseButtonX = PlayBackCar1ChoseButtonX;
        int PlayBackCar5ChoseButtonY = PlayBackCar1ChoseButtonY + BUTTON_HEIGHT_600P+ GRAPS_BETWEEN_CONTROLS;
        ui->pushButton_BackCar5->move(PlayBackCar5ChoseButtonX,PlayBackCar5ChoseButtonY);
        ui->pushButton_BackCar5->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackCar6ChoseButtonX = PlayBackCar2ChoseButtonX;
        int PlayBackCar6ChoseButtonY =PlayBackCar5ChoseButtonY;
        ui->pushButton_BackCar6->move(PlayBackCar6ChoseButtonX,PlayBackCar6ChoseButtonY);
        ui->pushButton_BackCar6->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackCar7ChoseButtonX = PlayBackCar3ChoseButtonX;
        int PlayBackCar7ChoseButtonY =PlayBackCar5ChoseButtonY;
        ui->pushButton_BackCar7->move(PlayBackCar7ChoseButtonX,PlayBackCar7ChoseButtonY);
        ui->pushButton_BackCar7->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackCar8ChoseButtonX = PlayBackCar4ChoseButtonX;
        int PlayBackCar8ChoseButtonY =PlayBackCar5ChoseButtonY;
        ui->pushButton_BackCar8->move(PlayBackCar8ChoseButtonX,PlayBackCar8ChoseButtonY);
        ui->pushButton_BackCar8->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        //playback dataedit
        int StartTimeEditX  =PlayBackCar1ChoseButtonX - GRAPS_BETWEEN_CONTROLS -TIMEEDIT_WIDTH_600P;
        int StartTimeEditY  = BUTTON_TO_CHOSEWIDGET_MARGIN_600P+ GRAPS_BETWEEN_CONTROLS;
        ui->timeEditStartTime->move(StartTimeEditX,StartTimeEditY);
        ui->timeEditStartTime->resize(TIMEEDIT_WIDTH_600P,TIMEEDIT_HEIGHT_600P);

        int EndTimeEditX  =PlayBackCar1ChoseButtonX - GRAPS_BETWEEN_CONTROLS -TIMEEDIT_WIDTH_600P;
        int EndTimeEditY  = PlayBackCar5ChoseButtonY+ GRAPS_BETWEEN_CONTROLS;
        ui->timeEditEndTime->move(EndTimeEditX,EndTimeEditY);
        ui->timeEditEndTime->resize(TIMEEDIT_WIDTH_600P,TIMEEDIT_HEIGHT_600P);

        int StartEditX = StartTimeEditX - GRAPS_BETWEEN_CONTROLS -DATAEDIT_WIDTH_600P;
        int StartEditY  = BUTTON_TO_CHOSEWIDGET_MARGIN_600P+ GRAPS_BETWEEN_CONTROLS;
        ui->dateEditStartTime->move(StartEditX,StartEditY);
        ui->dateEditStartTime->resize(DATAEDIT_WIDTH_600P,DATAEDIT_HEIGHT_600P);

        int EndEditX = StartEditX;
        int EndEditY  = PlayBackCar5ChoseButtonY +GRAPS_BETWEEN_CONTROLS;
        ui->dateEditEndTime->move(EndEditX,EndEditY);
        ui->dateEditEndTime->resize(DATAEDIT_WIDTH_600P,DATAEDIT_HEIGHT_600P);


        //playback control widget
        int PlayBackCtrlWidgetX = PlayBackCarChoseWidgetX + PlayBacklabelWidth + GRAPS_BETWEEN_CONTROLS;
        int PlayBackCtrlWidgetY = PlayBackCarChoseWidgetY;
        ui->widget_BackPlayCtrl->move(PlayBackCtrlWidgetX,PlayBackCtrlWidgetY);
        ui->widget_BackPlayCtrl->resize(PlayBacklabelWidth,PLAYBACK_CARRIAGE_SELECT_WIDGET_HEIGHT_600P);

        //label
        int PlayBackPlayControlLabelX = (PlayBacklabelWidth-REALMONITOR_LABEL_WIDTH_600P)/2;
        int PlayBackPlayControlLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelPlayBackCarCtrl->move(PlayBackPlayControlLabelX,PlayBackPlayControlLabelY);
        ui->labelPlayBackCarCtrl->resize(REALMONITOR_LABEL_WIDTH_600P,LABEL_HEIGHT_600P);

        //playback control button
        int StartButtonX  = (PlayBacklabelWidth -BUTTON_WIDTH_600P*6-GRAPS_BETWEEN_CONTROLS*5)/2;
        int StartButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN_600P;
        ui->pushButton_play->move(StartButtonX,StartButtonY);
        ui->pushButton_play->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int SuspendButtonX  = StartButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int SuspendButtonY = StartButtonY;
        ui->pushButton_suspend->move(SuspendButtonX,SuspendButtonY);
        ui->pushButton_suspend->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);


        int BackButtonX = SuspendButtonX +  BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int BackButtonY = SuspendButtonY;
        ui->pushButton_back->move(BackButtonX,BackButtonY);
        ui->pushButton_back->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int ForwardButtonX = BackButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int ForwardButtonY = BackButtonY;
        ui->pushButton_forward->move(ForwardButtonX,ForwardButtonY);
        ui->pushButton_forward->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackpantographButtonX = ForwardButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int PlayBackpantographButtonY = ForwardButtonY;
        ui->pushButton_Backpantograph->move(PlayBackpantographButtonX,PlayBackpantographButtonY);
        ui->pushButton_Backpantograph->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackNextPageButtonX = PlayBackpantographButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int PlayBackNextPageButtonY = PlayBackpantographButtonY;
        ui->pushButton_BacknextPage->move(PlayBackNextPageButtonX,PlayBackNextPageButtonY);
        ui->pushButton_BacknextPage->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int DayCheckX = StartButtonX;
        int DayCheckY = StartButtonY+ BUTTON_HEIGHT_600P+ GRAPS_BETWEEN_CONTROLS;
        int DayCheckButtonWidth = BUTTON_WIDTH_600P*3+GRAPS_BETWEEN_CONTROLS*2;
        ui->pushButton_DayPlayBack->move(DayCheckX,DayCheckY);
        ui->pushButton_DayPlayBack->resize(DayCheckButtonWidth,BUTTON_HEIGHT_600P);

        int MonthCheckX = ForwardButtonX;
        int MonthCheckY = DayCheckY;
        ui->pushButton_MonthPlayBack->move(MonthCheckX,MonthCheckY);
        ui->pushButton_MonthPlayBack->resize(DayCheckButtonWidth,BUTTON_HEIGHT_600P);

        //video download

        //download set widget
        int setWidgetX = GRAPS_BETWEEN_CONTROLS;
        int setWidgetY  = GRAPS_BETWEEN_CONTROLS;
        int setWidgetHeight =stackHeight - BUTTON_HEIGHT_600P - GRAPS_BETWEEN_CONTROLS*3;
        ui->widget_DownloadSet->move(setWidgetX,setWidgetY);
        ui->widget_DownloadSet->resize(DOWNLOAD_SET_WIDGET_WIDTH_600P,setWidgetHeight);

     //   int videoWidgetX = GRAPS_BETWEEN_CONTROLS + DOWNLOAD_SET_WIDGET_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
      //  int videoWidgetY = GRAPS_BETWEEN_CONTROLS;
     //   int videoWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*3 - DOWNLOAD_SET_WIDGET_WIDTH_600P;
     //   int videoWidgetHeight = setWidgetHeight;
     //   ui->widget_VideoDownloadWin->move(videoWidgetX,videoWidgetY);
     //   ui->widget_VideoDownloadWin->resize(videoWidgetWidth,videoWidgetHeight);

          int videoWidgetX = GRAPS_BETWEEN_CONTROLS + DOWNLOAD_SET_WIDGET_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
           int videoWidgetY = GRAPS_BETWEEN_CONTROLS;
           int videoWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*3 - DOWNLOAD_SET_WIDGET_WIDTH_600P;
           int videoWidgetHeight = setWidgetHeight- PlayBackByFileSliderHeight_600P -GRAPS_BETWEEN_CONTROLS*2;
           ui->widget_VideoDownloadWin->move(videoWidgetX,videoWidgetY);
           ui->widget_VideoDownloadWin->resize(videoWidgetWidth,videoWidgetHeight);

        int CtrlWidgetX = GRAPS_BETWEEN_CONTROLS;
        int CtrlWidgetY = GRAPS_BETWEEN_CONTROLS + setWidgetHeight+ GRAPS_BETWEEN_CONTROLS;
        int CtrlWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*2;
        int CtrlHeight =stackHeight - GRAPS_BETWEEN_CONTROLS*2-setWidgetHeight;
        ui->widgetDownloadCtrl->move(CtrlWidgetX,CtrlWidgetY);
        ui->widgetDownloadCtrl->resize(CtrlWidth,CtrlHeight);

        //Download control button
        ui->pushButtonStartQuery->resize(FILE_SEARCH_BUTTON_WIDTH_600P,FILE_SEARCH_BUTTON_HEIGHT_600P);

        int DownloadButtonX = ( DOWNLOAD_SET_WIDGET_WIDTH_600P- FILE_DOWNLOAD_BUTTON_WIDTH_600P)/2;
        int DownloadButtonY = GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownload->move(DownloadButtonX,DownloadButtonY);
        ui->pushButtonDownload->resize(FILE_DOWNLOAD_BUTTON_WIDTH_600P,FILE_DOWNLOAD_BUTTON_HEIGHT_600P);

        int DownloadLastButtonX = videoWidgetX +(videoWidgetWidth-BUTTON_WIDTH_600P*6-GRAPS_BETWEEN_CONTROLS*5)/2;
        ui->pushButtonDownloadLastFile->move(DownloadLastButtonX,DownloadButtonY);
        ui->pushButtonDownloadLastFile->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int DownloadBackX = DownloadLastButtonX + BUTTON_WIDTH_600P+GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadBack->move(DownloadBackX,DownloadButtonY);
        ui->pushButtonDownloadBack->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int DownloadPlayX = DownloadBackX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadPlay->move(DownloadPlayX,DownloadButtonY);
        ui->pushButtonDownloadPlay->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int DownloadStopX = DownloadPlayX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadStop ->move(DownloadStopX,DownloadButtonY);
        ui->pushButtonDownloadStop->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int DownloadForwardX =DownloadStopX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadForward ->move(DownloadForwardX,DownloadButtonY);
        ui->pushButtonDownloadForward->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int DownloadNextX =DownloadForwardX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        ui->pushButtonDownloadNextFile ->move(DownloadNextX,DownloadButtonY);
        ui->pushButtonDownloadNextFile->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);


        int SliderX = GRAPS_BETWEEN_CONTROLS + DOWNLOAD_SET_WIDGET_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int SliderY = setWidgetHeight - PlayBackByFileSliderHeight_600P - GRAPS_BETWEEN_CONTROLS;
        int SliderWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*3 - DOWNLOAD_SET_WIDGET_WIDTH_600P;
        int SliderHeight = DOWNLOAD_SET_WIDGET_WIDTH_600P;
        ui->horizontalSlider->move(SliderX,SliderY);
        ui->horizontalSlider->resize(SliderWidth,SliderHeight);

        //-----------------------------------------------------------
        //device  status

        int DeviceStatusX = stackWidth - MAIN_FUNCTION_BUTTON_GRAPS_600P -DEVICE_STATUS_BUTTON_WIDTH_600P*3;
        int DeviceStatusY = MAIN_FUNCTION_BUTTON_GRAPS_600P;
//         ui->pushButtonDeviceStatus ->move(DeviceStatusX,DeviceStatusY);
//         ui->pushButtonDeviceStatus->resize(DEVICE_STATUS_BUTTON_WIDTH_600P,DEVICE_STATUS_BUTTON_HEIGHT_600P);
// 

        int SaveStatusX  = stackWidth - MAIN_FUNCTION_BUTTON_GRAPS_600P -DEVICE_STATUS_BUTTON_WIDTH*2;
        int SaveStatusY  = MAIN_FUNCTION_BUTTON_GRAPS_600P;
        ui->pushButtonSaveStatus ->move(SaveStatusX,SaveStatusY);
        ui->pushButtonSaveStatus->resize(DEVICE_STATUS_BUTTON_WIDTH_600P,DEVICE_STATUS_BUTTON_HEIGHT_600P);

        int AlarmListStatusX  = stackWidth - MAIN_FUNCTION_BUTTON_GRAPS_600P -DEVICE_STATUS_BUTTON_WIDTH;
        int AlarmListStatusY  = MAIN_FUNCTION_BUTTON_GRAPS_600P;
        ui->pushButtonAlarmList ->move(AlarmListStatusX,AlarmListStatusY);
        ui->pushButtonAlarmList->resize(DEVICE_STATUS_BUTTON_WIDTH_600P,DEVICE_STATUS_BUTTON_HEIGHT_600P);


        int DeviceStatusStackWidgetX   =  GRAPS_BETWEEN_CONTROLS;
        int DeviceStatusStackWidgetY  = MAIN_FUNCTION_BUTTON_GRAPS_600P + DEVICE_STATUS_BUTTON_HEIGHT_600P+ GRAPS_BETWEEN_CONTROLS;
        int DeviceStatusStackWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*2;
        int DeviceStatusStackWidgetHeight = stackHeight - DeviceStatusStackWidgetY - GRAPS_BETWEEN_CONTROLS;
        ui->stackedWidget2 ->move(DeviceStatusStackWidgetX,DeviceStatusStackWidgetY);
        ui->stackedWidget2->resize(DeviceStatusStackWidgetWidth,DeviceStatusStackWidgetHeight);


        int SaveStatusTableWidgetX  = GRAPS_BETWEEN_CONTROLS;
        int SaveStatusTableWidgetY  = GRAPS_BETWEEN_CONTROLS;
        int SaveStatusTableWidgetWidth = DeviceStatusStackWidgetWidth -GRAPS_BETWEEN_CONTROLS*2;
        int SaveStatusTableWidgetHeight =DeviceStatusStackWidgetHeight -GRAPS_BETWEEN_CONTROLS*2;
        ui->tableWidgetSaveStatus ->move(SaveStatusTableWidgetX,SaveStatusTableWidgetY);
        ui->tableWidgetSaveStatus->resize(SaveStatusTableWidgetWidth,SaveStatusTableWidgetHeight);
       //ui->tableWidgetSaveStatus->setMinimumSize(SaveStatusTableWidgetWidth,SaveStatusTableWidgetHeight);
       // ui->tableWidgetSaveStatus->setMinimumHeight(SaveStatusTableWidgetHeight-30);


        int AlarmListTableWidgetX  = GRAPS_BETWEEN_CONTROLS;
        int AlarmListTableWidgetY  = GRAPS_BETWEEN_CONTROLS;
        int AlarmListWidgetWidth = DeviceStatusStackWidgetWidth - GRAPS_BETWEEN_CONTROLS*2;
        int AlarmListWidgetHeight =DeviceStatusStackWidgetHeight - GRAPS_BETWEEN_CONTROLS*2;
        ui->tableWidgetAlarmList ->move(AlarmListTableWidgetX,AlarmListTableWidgetY);
        ui->tableWidgetAlarmList->resize(AlarmListWidgetWidth,AlarmListWidgetHeight);


        //**************************update*****************
        int setUpdateWidgetX = GRAPS_BETWEEN_CONTROLS;
        int setUpdateWidgetY  = GRAPS_BETWEEN_CONTROLS;
        int setUpdateWidgetHeight =stackHeight -STATCKWIDGET_LEFT_MARGIN_600P- GRAPS_BETWEEN_CONTROLS;
        ui->widgetUpdate->move(setUpdateWidgetX,setUpdateWidgetY);
        ui->widgetUpdate->resize(DOWNLOAD_SET_WIDGET_WIDTH_600P,setUpdateWidgetHeight);

        int setUpdateTableWidgetX = GRAPS_BETWEEN_CONTROLS + DOWNLOAD_SET_WIDGET_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int setUpdateTableWidgetY = GRAPS_BETWEEN_CONTROLS;
        int setUpdateTableWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*3 - DOWNLOAD_SET_WIDGET_WIDTH_600P;
        int setUpdateTableWidgetHeight = stackHeight -STATCKWIDGET_LEFT_MARGIN_600P- GRAPS_BETWEEN_CONTROLS;
        ui->tableWidgetPower->move(setUpdateTableWidgetX,setUpdateTableWidgetY);
        ui->tableWidgetPower->resize(setUpdateTableWidgetWidth,setUpdateTableWidgetHeight);


   }

}

//设置控件初始值参数
void UserMonitor::InitControl()
{
    int i = 0;
    if(g_iCarriageNum >1)
    {
		ui->widget_PlayCtrl->show();
	}
	for (int nIndex= 0;nIndex<8;nIndex++)
	{
		auto pBtn = m_RealCarButton[nIndex];
		if (pBtn&&nIndex<g_iCarriageNum)
		{
			pBtn->show();
		}
		else if (pBtn)
		{
			pBtn->hide();
		}
	}

    //("<font color=white size=100><b>车厢选择</b></font>");
    ui->labelCarChose->setText(FL8T("<font color=white>车厢选择</b></font>"));
    ui->labelPlayCtrl->setText(FL8T("<font color=white>播放控制</b></font>"));
    ui->labelPlayBackCarChose->setText(FL8T("<font color=white>回放车厢选择</b></font>"));
    ui->labelPlayBackCarCtrl->setText(FL8T("<font color=white>播放控制</b></font>"));

    ui->dateEditStartTime->setCalendarPopup(true);
    ui->timeEditEndTime->setCalendarPopup(true);

    ui->dateEditStartTime->setDisplayFormat("yyyy-MM-dd");
    ui->dateEditEndTime->setDisplayFormat("yyyy-MM-dd");
    ui->timeEditStartTime->setDisplayFormat("hh:mm:ss");
    ui->timeEditEndTime->setDisplayFormat("hh:mm:ss");



    ui->dateEditStartTime->setDate(QDate::currentDate());
    ui->dateEditEndTime->setDate(QDate::currentDate());
    ui->timeEditStartTime->setTime(QTime::currentTime().addSecs(0-1800));
    ui->timeEditEndTime->setTime(QTime::currentTime());
    ui->dateEditDownloadStartTime->setTime(QTime::currentTime().addSecs(0-1800));
    ui->dateEditDownloadStartTime->setDate(QDate::currentDate());
    ui->dateEditDownloadEndTime->setDate(QDate::currentDate());
    ui->dateEditDownloadEndTime->setTime(QTime::currentTime());


    ui->lineEditCar->setValidator(new QIntValidator(1, 9999, this));
    ui->lineEditPollTIme->setValidator(new QIntValidator(1, 9999, this));

    //下载项车厢相机选择控件设置
    ui->comboBoxCameraSelect->setStyleSheet("selection-background-color:white;selection-color:black");
    ui->comboBoxCarriageSelect->setStyleSheet("selection-background-color:white;selection-color:black");
    for(i=0;i<g_iCarriageNum;i++)
    {
       ui->comboBoxCarriageSelect->addItem(FL8T("车厢%1").arg(i+1));
    }
   // ui->comboBoxCarriageSelect->setFocusPolicy(Qt::NoFocus);


    QTableWidgetItem *headerItem = NULL;
    QStringList headerText;
    headerText<<FL8T("文件名")<<FL8T("车厢")<<FL8T("相机");
    ui->tableWidgetFileList->setColumnCount(headerText.count());
    ui->tableWidgetFileList->setHorizontalHeaderLabels(headerText);
    ui->tableWidgetFileList->setColumnWidth(0,280);
	ui->tableWidgetFileList->verticalHeader()->setVisible(false);
    headerText.clear();
    headerText<<FL8T("设备名")<<FL8T("设备IP")<<FL8T("硬盘容量")<<FL8T("硬盘使用量")<<FL8T("设备状态")<<" ";
	ui->tableWidgetSaveStatus->verticalHeader()->setVisible(false);
	ui->tableWidgetSaveStatus->setStyleSheet("background:rgb(5,23,89);");
	ui->tableWidgetSaveStatus->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
	ui->tableWidgetSaveStatus->setStyleSheet("QHeaderView::section{background:rgb(5,23,89);}");
	ui->tableWidgetSaveStatus->setFocusPolicy(Qt::NoFocus);
    ui->tableWidgetSaveStatus->setColumnCount(headerText.count());
    ui->tableWidgetSaveStatus->setHorizontalHeaderLabels(headerText);
    ui->tableWidgetSaveStatus->setColumnWidth(0,150);
    ui->tableWidgetSaveStatus->setColumnWidth(1,150);
    ui->tableWidgetSaveStatus->setColumnWidth(2,150);
    ui->tableWidgetSaveStatus->setColumnWidth(3,150);
    ui->tableWidgetSaveStatus->setColumnWidth(4,150);
    int leftWidth =stackWidth - GRAPS_BETWEEN_CONTROLS*8-150*5;
    ui->tableWidgetSaveStatus->setColumnWidth(5,leftWidth);
	

    headerText.clear();
    headerText<<FL8T("报警时间")<<FL8T("报警内容")<<FL8T("设备信息")<<"";
	ui->tableWidgetAlarmList->verticalHeader()->setVisible(false);
	ui->tableWidgetAlarmList->setStyleSheet("alternate-background-color:rgb(232,232,232);background:white;color:black");
	ui->tableWidgetAlarmList->setStyleSheet("background:rgb(5,23,89);");
	ui->tableWidgetAlarmList->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
	ui->tableWidgetAlarmList->setStyleSheet("QHeaderView::section{background:rgb(5,23,89);}");
    ui->tableWidgetAlarmList->setColumnCount(headerText.count());
    ui->tableWidgetAlarmList->setHorizontalHeaderLabels(headerText);
    ui->tableWidgetAlarmList->setColumnWidth(0,200);
    ui->tableWidgetAlarmList->setColumnWidth(1,200);
    ui->tableWidgetAlarmList->setColumnWidth(2,200);
    leftWidth =stackWidth - GRAPS_BETWEEN_CONTROLS*6 -200*3;
    ui->tableWidgetAlarmList->setColumnWidth(3,leftWidth);
	ui->tableWidgetAlarmList->setFocusPolicy(Qt::NoFocus);


    headerText.clear();
    headerText<<FL8T("用户名")<<FL8T("权限")<<FL8T("操作");
    ui->tableWidgetPower->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableWidgetPower->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidgetPower->setAlternatingRowColors(true);
    ui->tableWidgetPower->setStyleSheet("alternate-background-color:rgb(232,232,232);background:white;color:black");
    // 去除网格线
    ui->tableWidgetPower->setShowGrid(false);
    ui->tableWidgetPower->setColumnCount(headerText.count());
    ui->tableWidgetPower->setHorizontalHeaderLabels(headerText);

    int perColumnWidth =ui->tableWidgetPower->width() /3;
    ui->tableWidgetPower->setColumnWidth(0,perColumnWidth);
    ui->tableWidgetPower->setColumnWidth(1,perColumnWidth);
    ui->tableWidgetPower->setColumnWidth(2,ui->tableWidgetPower->width()-perColumnWidth*2-20);
    QHeaderView*header =  ui->tableWidgetPower->verticalHeader();
    header->setHidden(true);// 隐藏行号

    ui->radioButton10s->setChecked(true);
    ui->lineEditPollTIme->setEnabled(false);

	if (m_enumPollTimeType==USER)
	{
		ui->radioButtonCustomDefine->setChecked(true);
		QString strTime= QString::number(m_iPollTimerSeconds);
		ui->lineEditPollTIme->setText(strTime);
	}
	else
	{
		if (m_iPollTimerSeconds==30)
		{
			ui->radioButton10s->setChecked(true);
		}
		else if (m_iPollTimerSeconds==20)
		{
			ui->radioButton20s->setChecked(true);
		}
		else
		{
			ui->radioButton30s->setChecked(true);
		}
	}
}

void  UserMonitor::InitCarNVRButton()
{
	if (m_nNVRNumber==1)
	{
		
	}
	else
	{
		m_btnGroupNVRCar = new QButtonGroup();//按钮组
		connect(m_btnGroupNVRCar, SIGNAL(buttonClicked(int)),this, SLOT(buttonNumslot(int)));
		QHBoxLayout *pLayoutQNVRCar = new QHBoxLayout;
		for (int nNVRNum=0;nNVRNum<4;nNVRNum++)
		{
			QGroupBox *pGroupNVR=createNVRGroup(nNVRNum);
			pLayoutQNVRCar->addWidget(pGroupNVR);
		}
		ui->widget_CarChose->setLayout(pLayoutQNVRCar);
		ui->labelCarChose->hide();
	}

}

QGroupBox *UserMonitor::createNVRGroup(int nIndex,int nCarNum)
{
	QString strFormatNVR=QString("NVR%1").arg(nIndex+1);
	QGroupBox *pGroupBox = new QGroupBox(strFormatNVR);
	QHBoxLayout *phLayoutBox = new QHBoxLayout;
	int nCarPos=0;
	if (nIndex==1)
	{
		nCarPos=2;
	}
	for ( nCarPos;nCarPos<4;nCarPos++)
	{
		QPushButton *pCarBtn = new QPushButton();
		QString   strSheetInfo ="",strImageName=""; 
		if (nCarPos==0&&nIndex==0)
		{
			strImageName=QString("%1_on").arg(nCarPos+1);
		}
		else
		{
			strImageName=QString("%1").arg(nCarPos+1);
		}
		///carImag/carImage/
		strSheetInfo=QString("QPushButton{border-image:url(:/carImag/carImage/%1.png)}").arg(strImageName);
		pCarBtn->setStyleSheet(strSheetInfo);
		pCarBtn->setFixedSize(BUTTON_WIDTH,BUTTON_HEIGHT);
		
		phLayoutBox->addWidget(pCarBtn);
	}
	pGroupBox->setLayout(phLayoutBox);

	return pGroupBox;
}

 void UserMonitor::InitParams()
 {
     m_RealCarButton[0]= ui->pushButton_Car1;
     m_RealCarButton[1]= ui->pushButton_Car2;
     m_RealCarButton[2]= ui->pushButton_Car3;
     m_RealCarButton[3]= ui->pushButton_Car4;
     m_RealCarButton[4]= ui->pushButton_Car5;
     m_RealCarButton[5]= ui->pushButton_Car6;
     m_RealCarButton[6]= ui->pushButton_Car7;
     m_RealCarButton[7]= ui->pushButton_Car8;

     m_BackCarButton[0]= ui->pushButton_BackCar1;
     m_BackCarButton[1]= ui->pushButton_BackCar2;
     m_BackCarButton[2]= ui->pushButton_BackCar3;
     m_BackCarButton[3]= ui->pushButton_BackCar4;
     m_BackCarButton[4]= ui->pushButton_BackCar5;
     m_BackCarButton[5]= ui->pushButton_BackCar6;
     m_BackCarButton[6]= ui->pushButton_BackCar7;
     m_BackCarButton[7]= ui->pushButton_BackCar8;

     if(g_iCarriageNum == 6)
     {
         CarNVR[0]= ui->pushButtonCar1NVRAlarm;
         CarNVR[1]=ui->pushButtonCar2NVRAlarm;
         CarNVR[2]= ui->pushButtonCar3NVRAlarm;
         CarNVR[3]=ui->pushButtonCar4NVRAlarm;
         CarNVR[4]= ui->pushButtonCar5NVRAlarm;
         CarNVR[5]=ui->pushButtonCar6NVRAlarm;


         CarCamera[0][0] =ui->pushButtonCar1Alarm1;
         CarCamera[0][1] =ui->pushButtonCar1Alarm2;
         CarCamera[0][2] =ui->pushButtonCar1Alarm3;
         CarCamera[0][3] =ui->pushButtonCar1DriverAlarm;

         CarCamera[1][0] =ui->pushButtonCar2Alarm1;
         CarCamera[1][1] =ui->pushButtonCar2Alarm2;
         CarCamera[1][2] =ui->pushButtonCar2Alarm3;
         CarCamera[1][3] =ui->pushButtonCar2PantoAlarm;

         CarCamera[2][0] =ui->pushButtonCar3Alarm1;
         CarCamera[2][1] =ui->pushButtonCar3Alarm2;
         CarCamera[2][2] =ui->pushButtonCar3Alarm3;
         CarCamera[2][3] =ui->pushButtonCar3PantoAlarm;

         CarCamera[3][0] =ui->pushButtonCar4Alarm1;
         CarCamera[3][1] =ui->pushButtonCar4Alarm2;
         CarCamera[3][2] =ui->pushButtonCar4Alarm3;
         CarCamera[3][3] =ui->pushButtonCar4PantoAlarm;

         CarCamera[4][0] =ui->pushButtonCar5Alarm1;
         CarCamera[4][1] =ui->pushButtonCar5Alarm2;
         CarCamera[4][2] =ui->pushButtonCar5Alarm3;
         CarCamera[4][3] =ui->pushButtonCar5PantoAlarm;

         CarCamera[5][0] =ui->pushButtonCar6Alarm1;
         CarCamera[5][1] =ui->pushButtonCar6Alarm2;
         CarCamera[5][2] =ui->pushButtonCar6Alarm3;
         CarCamera[5][3] =ui->pushButtonCar6DriverAlarm;
     }
     else
     {
         CarNVR[0]= ui->pushButton8Car1NVRAlarm;
         CarNVR[1]=ui->pushButton8Car2NVRAlarm;
         CarNVR[2]= ui->pushButton8Car3NVRAlarm;
         CarNVR[3]=ui->pushButton8Car4NVRAlarm;
         CarNVR[4]= ui->pushButton8Car5NVRAlarm;
         CarNVR[5]=ui->pushButton8Car6NVRAlarm;
         CarNVR[6]= ui->pushButton8Car7NVRAlarm;
         CarNVR[7]=ui->pushButton8Car8NVRAlarm;


         CarCamera[0][0] =ui->pushButton8Car1Alarm1;
         CarCamera[0][1] =ui->pushButton8Car1Alarm2;
         CarCamera[0][2] =ui->pushButton8Car1Alarm3;
         CarCamera[0][3] =ui->pushButton8Car1DriverAlarm;

         CarCamera[1][0] =ui->pushButton8Car2Alarm1;
         CarCamera[1][1] =ui->pushButton8Car2Alarm2;
         CarCamera[1][2] =ui->pushButton8Car2Alarm3;
         CarCamera[1][3] =ui->pushButton8Car2PantoAlarm;

         CarCamera[2][0] =ui->pushButton8Car3Alarm1;
         CarCamera[2][1] =ui->pushButton8Car3Alarm2;
         CarCamera[2][2] =ui->pushButton8Car3Alarm3;
         CarCamera[2][3] =ui->pushButton8Car3PantoAlarm;

         CarCamera[3][0] =ui->pushButton8Car4Alarm1;
         CarCamera[3][1] =ui->pushButton8Car4Alarm2;
         CarCamera[3][2] =ui->pushButton8Car4Alarm3;
         CarCamera[3][3] =ui->pushButton8Car4PantoAlarm;

         CarCamera[4][0] =ui->pushButton8Car5Alarm1;
         CarCamera[4][1] =ui->pushButton8Car5Alarm2;
         CarCamera[4][2] =ui->pushButton8Car5Alarm3;
         CarCamera[4][3] =ui->pushButton8Car5PantoAlarm;

         CarCamera[5][0] =ui->pushButton8Car6Alarm1;
         CarCamera[5][1] =ui->pushButton8Car6Alarm2;
         CarCamera[5][2] =ui->pushButton8Car6Alarm3;
         CarCamera[5][3] =ui->pushButton8Car6PantoAlarm;

         CarCamera[6][0] =ui->pushButton8Car7Alarm1;
         CarCamera[6][1] =ui->pushButton8Car7Alarm2;
         CarCamera[6][2] =ui->pushButton8Car7Alarm3;
         CarCamera[6][3] =ui->pushButton8Car7PantoAlarm;

         CarCamera[7][0] =ui->pushButton8Car8Alarm1;
         CarCamera[7][1] =ui->pushButton8Car8Alarm2;
         CarCamera[7][2] =ui->pushButton8Car8Alarm3;
         CarCamera[7][3] =ui->pushButton8Car8DriverAlarm;
     }

     QSqlDatabase  mydb  = QSqlDatabase::addDatabase("QSQLITE");
    //新建数据库（已有的情况下也可以用）
      mydb.setDatabaseName("MyDB");
    //已有数据库
     mydb.database("MyDB");
     mydb.open();
	 
    // 带检查功能，如果已有数据表则不创建
      QString sqlCreatTab = QString("create table if not exists tb_power("
                       "serial_id serial primary key, "
                       "User_name varchar(32) ,"
                       "User_operator varchar(32) ,"
                       "User_password varchar(32));");


     QSqlQuery query(mydb);
     bool isok=query.exec(sqlCreatTab);


     QString sqlSelect = QString("select * from tb_power;");
          isok=query.exec(sqlSelect);

     bool bInitUser=false;
     while (query.next())
     {
          QString name = query.value("User_name").toString();
          int iOperator = query.value("User_operator").toInt();
          if(name == "admin" && iOperator == E_USER_INFO_SUPER_ADMIN)
          {
              bInitUser=true;
              break;
          }
     }
     if(bInitUser ==false)
     {
         QString sqlInsert = QString("insert into tb_power(User_name, User_operator, User_password) Values('%1', '%2', '%3');").arg("admin").arg(E_USER_INFO_SUPER_ADMIN).arg("cftc123456");
         query.exec(sqlInsert);
     }
	 // 带检查功能，如果已有数据表则不创建
	 QString sqlCreateParamTable = QString("create table if not exists tb_param("
		 "serial_id serial primary key, "
		 "poll_type varchar(32), "
		 "video_poll_time varchar(32)) ;"
		 );

	  isok = query.exec(sqlCreateParamTable);

	  sqlSelect = QString("select * from tb_param;");
	 isok=query.exec(sqlSelect);

	 int count =0;
	 bool bFindUser =0;
	 int iOperator = 0;
	 QString strVideoPollTime,strPollTimeType;
	 bool bInitParam=false;
	 if (query.next())
	 {
		 strVideoPollTime = query.value("video_poll_time").toString();
		 m_iPollTimerSeconds = atoi(strVideoPollTime.toStdString().c_str());
		 strPollTimeType =query.value("poll_type").toString();
		 m_enumPollTimeType = strPollTimeType.compare("user")==0?USER:FORMAT;
		 bInitParam = true;
	 }
	 if (bInitParam==false)
	 {
		 QString sqlInsert = QString("insert into tb_param(poll_type,video_poll_time) Values('%1','%2');").arg("format").arg("30");
		 query.exec(sqlInsert);
	 }
	
     mydb.close();
 }


 void UserMonitor::InitRender()
 {
     QRect rt;
     QLabel *pWnds[VIDEO_WINDOWS_COUNT] = {ui->widget_MonitorWin1,ui->widget_MonitorWin2,ui->widget_MonitorWin3,ui->widget_MonitorWin4};
     for(int i = 0; i < VIDEO_WINDOWS_COUNT; i++)
     {
         rt = pWnds[i]->geometry();
         m_RealMonitorVideos[i].nVideoWidth = 0;
         m_RealMonitorVideos[i].nVideoHeight = 0;
         m_RealMonitorVideos[i].pRenderHandle = NULL;
         m_RealMonitorVideos[i].nX = rt.x();
         m_RealMonitorVideos[i].nY = rt.y();
         m_RealMonitorVideos[i].nWidth = rt.width();
         m_RealMonitorVideos[i].nHeight = rt.height();
         m_RealMonitorVideos[i].hWnd = (HWND)pWnds[i]->winId();
#if 0
         if(m_RealMonitorVideos[i].pRenderHandle == NULL)
         {
             m_RealMonitorVideos[i].nVideoWidth = 1920;
             m_RealMonitorVideos[i].nVideoHeight = 1080;
             m_RealMonitorVideos[i].pRenderHandle = (void*)RENDER_Create(&m_RealMonitorVideos[i]);
         }
#endif
     }
     rt = ui->widget_VideoDownloadWin->geometry();
     m_tPlayBackVideo.nVideoWidth = 0;
     m_tPlayBackVideo.nVideoHeight = 0;
     m_tPlayBackVideo.pRenderHandle = NULL;
     m_tPlayBackVideo.nX = rt.x();
     m_tPlayBackVideo.nY = rt.y();
     m_tPlayBackVideo.nWidth = rt.width();
     m_tPlayBackVideo.nHeight = rt.height();
     m_tPlayBackVideo.hWnd = (HWND)ui->widget_VideoDownloadWin->winId();

#if 0
     if(m_tPlayBackVideo.pRenderHandle == NULL)
     {
         m_tPlayBackVideo.nVideoWidth = 1920;
         m_tPlayBackVideo.nVideoHeight = 1080;
         m_tPlayBackVideo.pRenderHandle = (void*)RENDER_Create(&m_tPlayBackVideo);
     }
#endif
 }
void UserMonitor::UnInitRender()
{
	
    for(int i = 0; i < VIDEO_WINDOWS_COUNT; i++)
    {
        if(m_RealMonitorVideos[i].pRenderHandle != NULL)
        {
			//todo
            //RENDER_Destroy(m_RealMonitorVideos[i].pRenderHandle);
        }
    }
    if(m_tPlayBackVideo.pRenderHandle != NULL)
    {
        //RENDER_Destroy(m_tPlayBackVideo.pRenderHandle);
    }
}
 /*************************************信号和槽*********************************************/
//设置受电弓按钮风格
 void UserMonitor::SetPantoStyleSheet(int iCarriageIndex)
 {

 }

 //左上角实时时间
 void UserMonitor::ProcessTimerOut()
 {
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy-MM-dd  hh:mm:ss");
    ui->labelRealTime->setText(current_date);
    ui->labelSystemTime->setText(current_date);
 }

 void UserMonitor::ProcessConnectTimerOut()
 {
     int iNvrNo = -1;
     for(int i = 0; i < m_pSdk->GetNvrNum(); i++)
     {
         iNvrNo = i;
         if(E_SERV_STATUS_CONNECT == NVR_GetConnectStatus(iNvrNo))
         {    
			
             if(0 == m_iConnectStates[i])
             { 
				 GetDeviceInfoOfCarriage(true);
                 m_iConnectStates[i] = 1;
                 UpdateSaveStatusTimer->stop();
                 m_bUpdateDevStatus = false;
                 UpdateSaveStatusTimer->setInterval(2000);
                 UpdateSaveStatusTimer->start();
                 if(m_iCurSelectCarriageIdx <= 0)
                 {
                     SetCarrriage(1);
                 }
                 else
                 {
                     SetCarrriage(m_iCurSelectCarriageIdx, true);
                 }
             }
             for(int j = 0; j < MAX_CARRIAGE_PRE_CAMERA_NUM; j++)
             {
                 if(1 != m_pSdk->getMonitorState(j))
                 {
                    SetMoniitorDefaultStyleSheet(j);
                 }
             }
         }
         else
         {
             if(m_iConnectStates[i])
             {
                 m_iConnectStates[i] = 0;
                 m_pSdk->stopMonitor(m_iCurSelectCarriageIdx);
				 m_bInitPreview = false;
                 SetMoniitorDefaultStyleSheet();
                 GetDeviceInfoOfCarriage(true);
             }
         }
     }
 }

 //轮询定时器
void UserMonitor::ProcessPollTimerOut()
{
    GetAlarmList();
}

DWORD WINAPI UserMonitor::SetCarrriageThread(void *arg)
{
	return 0;
	THREAD_PARAM *pThreadParam = (THREAD_PARAM*) arg;
	if (pThreadParam)
	{
		if (pThreadParam->m_pThis)
		{
			pThreadParam->m_pThis->emit SetCarNo(pThreadParam->m_nCarrriage,pThreadParam->m_bForcePlay);
			pThreadParam->m_pThis->SetCarrriageWorker(pThreadParam->m_nCarrriage,pThreadParam->m_bForcePlay);
		}
	}
	
	return 0;
}
void UserMonitor::SetCarrriageWorker(int iCarrriage, bool bForcePlay /* = false */)
{
	if(iCarrriage < 1 || iCarrriage > MAX_CARRIAGE_NUM)
	{
		return;
	}
	if(!bForcePlay && iCarrriage == m_iCurSelectCarriageIdx)
	{
		return;
	}

	m_pSdk->stopMonitor(m_iCurSelectCarriageIdx);
	m_bInitPreview = false;
	m_iCurSelectCarriageIdx = iCarrriage;

	SWitchVideoWinStyleSheet();
	if(iVideoFormat == VIDEO_1024_768)
	{
		if(m_iLastSelectCarriageIdx != m_iCurSelectCarriageIdx)
		{
			QString strStyleSheet = QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(m_iLastSelectCarriageIdx).arg(".png)}");
			m_RealCarButton[m_iLastSelectCarriageIdx-1]->setStyleSheet(strStyleSheet);
			m_iLastSelectCarriageIdx = m_iCurSelectCarriageIdx;
		}

		QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(m_iCurSelectCarriageIdx).arg("_on.png)}");
		m_RealCarButton[m_iCurSelectCarriageIdx-1]->setStyleSheet(strStyleSheet1);
	}
	else
	{
		if(m_iLastSelectCarriageIdx != m_iCurSelectCarriageIdx)
		{
			QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(m_iLastSelectCarriageIdx).arg(".png)}");
			m_RealCarButton[m_iLastSelectCarriageIdx-1]->setStyleSheet(strStyleSheet);
			m_iLastSelectCarriageIdx = m_iCurSelectCarriageIdx;
		}

		QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(m_iCurSelectCarriageIdx).arg("_on.png)}");
		m_RealCarButton[m_iCurSelectCarriageIdx-1]->setStyleSheet(strStyleSheet1);
	}

	//InitRender();

	m_pSdk->initPreview(m_RealMonitorVideos, VIDEO_WINDOWS_COUNT);
	m_bInitPreview = true;
	m_pSdk->startMonitor(iCarrriage);
}
void UserMonitor::slotSetCarNo(int iCarNo,bool bForcePlay)
{
	//SetCarrriageWorker(iCarNo,bForcePlay);
		
}
void UserMonitor::SetCarrriage(int iCarrriage, bool bForcePlay)
{
	//emit SetCarNo(iCarrriage,bForcePlay);
	SetCarrriageWorker(iCarrriage,bForcePlay);
// 	THREAD_PARAM stuTempThreadParam;
// 	stuTempThreadParam.m_pThis = this;
// 	stuTempThreadParam.m_nCarrriage = iCarrriage;
// 	stuTempThreadParam.m_bForcePlay = bForcePlay;
// 
// #if _WIN32
// 	HANDLE ThreadHandle = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SetCarrriageThread, &stuTempThreadParam, 0, NULL);
// 	::CloseHandle(ThreadHandle);
// #else
// 	iRet = pthread_create(&ptPmsgConnInfo->ThreadHandle, NULL, CliProcessThread, (void *)ptPmsgConnInfo);
// 	if (iRet < 0)
// 	{
// 		free(ptPmsgConnInfo);
// 		ptPmsgConnInfo = NULL;
// 		return 0;	
// 	}
// #endif
	
	
}


//获取告警列表
void UserMonitor::GetAlarmList()
{
    int iRet = -1;
    int iCurRow = 0;
    int iCarriageNo = 0;
    QString strText;
    T_NVR_WARN_STATE *pNvrWarnState = NULL;
    T_IPC_WARN_STATE *pIpcWarnState = NULL;
    QTableWidgetItem *pItem = NULL;
    ui->tableWidgetAlarmList->clearContents();
    ui->tableWidgetAlarmList->setRowCount(0);
    T_NVR_INFO tNvrInfo;
    QDateTime dt;
    //headerText<<"报警时间"<<"报警内容"<<"设备信息"<<"";
    for (int iNvrNo = 0; iNvrNo < m_pSdk->GetNvrNum(); iNvrNo++)
    {
        iRet = STATE_GetNvrInfo(iNvrNo, &tNvrInfo);
        if(iRet < 0)
        {
            continue;
        }

        T_DATA_LIST *pWarnList = &tNvrInfo.tWarnStateList;
        while(pWarnList != NULL)
        {
            pNvrWarnState = (T_NVR_WARN_STATE *)pWarnList->pData;
            if(pNvrWarnState == NULL)
            {
                break;
            }

            iCurRow = ui->tableWidgetAlarmList->rowCount();
            ui->tableWidgetAlarmList->insertRow(iCurRow);
            dt.setTime_t(pNvrWarnState->nTime);
            pItem = new QTableWidgetItem(dt.toString("yyyy-MM-dd hh:mm:ss"));
            ui->tableWidgetAlarmList->setItem(iCurRow, 0, pItem);

            strText = "";
            //bit0:硬盘1,bit1:硬盘2, ... , 0：无丢失；1：丢失
            for(int j = 0; j < 8; j++)
            {
                if(0x01 & (pNvrWarnState->cHdiskLost>>j))
                {
                    strText += FL8T("硬盘%1丢失 ").arg(j+1);
                }
                else
                {
                    //strText = FL8T("无丢失 ");
                }
                if(0x01 & (pNvrWarnState->cHdiskBad>>j))
                {
                    strText += FL8T("硬盘%1损坏").arg(j+1);
                }
                else
                {
                    //strText += FL8T("无损坏");
                }
            }

            pItem = new QTableWidgetItem(strText);
            ui->tableWidgetAlarmList->setItem(iCurRow, 1, pItem);


            strText = FL8T("NVR%1").arg(iNvrNo+1);
            pItem = new QTableWidgetItem(strText);
            ui->tableWidgetAlarmList->setItem(iCurRow, 2, pItem);


            pWarnList = pWarnList->pLast;
        }

        //相机
        for (int iCamNo = 0; iCamNo < tNvrInfo.iIPCNum; iCamNo++)
        {
            if(iCamNo >= MAX_CARRIAGE_PRE_CAMERA_NUM)
            {
                break;
            }

            iCarriageNo = tNvrInfo.atIpcInfo[iCamNo].cCarriageNo;

            T_DATA_LIST *pWarnList = &tNvrInfo.atIpcInfo[iCamNo].tWarnStateList;
            while(pWarnList != NULL)
            {
                pIpcWarnState = (T_IPC_WARN_STATE *)pWarnList->pData;
                if(pIpcWarnState == NULL)
                {
                    break;
                }

                iCurRow = ui->tableWidgetAlarmList->rowCount();
                ui->tableWidgetAlarmList->insertRow(iCurRow);
                dt.setTime_t(pIpcWarnState->nTime);
                pItem = new QTableWidgetItem(dt.toString("yyyy-MM-dd hh:mm:ss"));
                ui->tableWidgetAlarmList->setItem(iCurRow, 0, pItem);

                strText = "";
                if(pIpcWarnState->cShelter)
                {
                    strText = FL8T("遮挡 ");
                }
                else
                {
                    //strText = FL8T("无遮挡 ");
                }
                if(pIpcWarnState->cLost)
                {
                    strText += FL8T("丢失");
                }
                else
                {
                    //strText += FL8T("无丢失");
                }
                pItem = new QTableWidgetItem(strText);
                ui->tableWidgetAlarmList->setItem(iCurRow, 1, pItem);


                strText = tNvrInfo.atIpcInfo[iCamNo].acChannelName;
                if(strText.isEmpty())
                {
                   strText = FL8T("%1车厢%2号相机").arg(iCarriageNo).arg((int)tNvrInfo.atIpcInfo[iCamNo].cPos);
                }
                pItem = new QTableWidgetItem(strText);
                ui->tableWidgetAlarmList->setItem(iCurRow, 2, pItem);


                pWarnList = pWarnList->pLast;
            }
        }
    }//end for

 	if (ui->tableWidgetAlarmList!=NULL&&ui->tableWidgetAlarmList->rowCount()>0)
 	{
 		ui->tableWidgetAlarmList->sortItems(0);
 	}

}


//设置车厢状态图位置
void UserMonitor::SetCarrriageStatus()
{
    if(iVideoFormat == VIDEO_1024_768)
    {
        int DeviceStatusStackWidgetX   =  GRAPS_BETWEEN_CONTROLS;
        int DeviceStatusStackWidgetY  = MAIN_FUNCTION_BUTTON_GRAPS + DEVICE_STATUS_BUTTON_HEIGHT+ GRAPS_BETWEEN_CONTROLS;
        int DeviceStatusStackWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*2;
        int DeviceStatusStackWidgetHeight = stackHeight - DeviceStatusStackWidgetY - GRAPS_BETWEEN_CONTROLS;
        ui->stackedWidget2 ->move(DeviceStatusStackWidgetX,DeviceStatusStackWidgetY);
        ui->stackedWidget2->resize(DeviceStatusStackWidgetWidth,DeviceStatusStackWidgetHeight);

//         ui->pushButtonDeviceStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/device_status_hot.png)}");
// 		ui->pushButtonDeviceStatus->hide();
		ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/device_status_hot.png)}");
        int CarDriverAlarmX ,CarNVRX,CarAlarm1X,CarPantoX;
        int CarDriverAlarmY,CarNVRY,CarAlarm1Y,CarPantoY;
        if(g_iCarriageNum ==8)
        {

            int CarriageStatusWidgetX  = (DeviceStatusStackWidgetWidth - CARRIAGE_STATUS_WIDGET_WIDTH)/2;
            int CarriageStatusWidgetY  = (DeviceStatusStackWidgetHeight - CARRIAGE_STATUS_WIDGET_HEIGHT)/2;
            int CarriageStatusWidgetWidth = CARRIAGE_STATUS_WIDGET_WIDTH;
            int CarriageStatusWidgetHeight = CARRIAGE_STATUS_WIDGET_HEIGHT;
            ui->Widget8CarriageStatus ->move(CarriageStatusWidgetX,CarriageStatusWidgetY);
            ui->Widget8CarriageStatus->resize(CarriageStatusWidgetWidth,CarriageStatusWidgetHeight);

            ui->Widget8CarriageStatus->setStyleSheet("QWidget{border-image: url(:/imag/image/carriage_status_8_bg.png)}");

            //car1
            CarAlarm1X =134 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car1Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car1Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car1Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =165 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car1Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car1Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car1Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =165 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =111+(131-111-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButton8Car1NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car1NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car1NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            CarAlarm1X =196 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car1Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car1Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car1Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //car2
            CarAlarm1X =327 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car2Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car2Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car2Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            CarAlarm1X =358 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car2Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car2Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car2Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =358 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =111+(131-111-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButton8Car2NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car2NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car2NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            //panto
            CarPantoX  = 358 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =50- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButton8Car2PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButton8Car2PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car2PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");

            CarAlarm1X =389 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car2Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car2Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car2Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //car3
            CarAlarm1X =517 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car3Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car3Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car3Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =547- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car3Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car3Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car3Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =547 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =111+(131-111-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButton8Car3NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car3NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car3NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            //panto
            CarPantoX  = 547 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =50- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButton8Car3PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButton8Car3PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car3PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");

            CarAlarm1X =578- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car3Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car3Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car3Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //car4
            CarAlarm1X =711- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car4Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car4Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car4Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =742 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car4Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car4Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car4Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =742 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =111+(131-111-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButton8Car4NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car4NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car4NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            //panto
            CarPantoX  = 742 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =50- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButton8Car4PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButton8Car4PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car4PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");

            CarAlarm1X =773- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =149;
            ui->pushButton8Car4Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car4Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car4Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //car5
            CarAlarm1X =39 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car5Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car5Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car5Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =69 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car5Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car5Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car5Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =69 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =393+(413-393-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButton8Car5NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car5NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car5NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            //panto
            CarPantoX  = 69 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =332- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButton8Car5PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButton8Car5PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car5PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");

            CarAlarm1X =100 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car5Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car5Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car5Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //car6
            CarAlarm1X =230 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car6Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car6Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car6Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =260 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car6Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car6Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car6Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =260- DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =393+(413-393-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButton8Car6NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car6NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car6NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            //panto
            CarPantoX  = 260 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =332- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButton8Car6PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButton8Car6PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car6PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");

            CarAlarm1X =292- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car6Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car6Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car6Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //car7
            CarNVRX =419 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarNVRY =429;
            ui->pushButton8Car7Alarm1->move(CarNVRX,CarNVRY);
            ui->pushButton8Car7Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car7Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =450 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car7Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car7Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car7Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =450 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =393+(413-393-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;;
            ui->pushButton8Car7NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car7NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car7NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");


            //panto
            CarPantoX  =450- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =332- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButton8Car7PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButton8Car7PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car7PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");


            CarAlarm1X =481- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car7Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car7Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car7Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //car8
            CarNVRX =613- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarNVRY =429;
            ui->pushButton8Car8Alarm1->move(CarNVRX,CarNVRY);
            ui->pushButton8Car8Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car8Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =644 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car8Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car8Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car8Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =644- DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =393+(413-393-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;;
            ui->pushButton8Car8NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car8NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car8NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");


            CarAlarm1X =675 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =429;
            ui->pushButton8Car8Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car8Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButton8Car8Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //driver
            CarDriverAlarmX = 74 - DEVICE_DRIVER_BUTTON_WIDTH/2;
            CarDriverAlarmY =131;
            ui->pushButton8Car1DriverAlarm->move(CarDriverAlarmX,CarDriverAlarmY);
            ui->pushButton8Car1DriverAlarm->resize(DEVICE_DRIVER_BUTTON_WIDTH,DEVICE_DRIVER_BUTTON_HEIGHT);
            ui->pushButton8Car1DriverAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/driver_alarm.png)}");


            CarDriverAlarmX = 807 - DEVICE_DRIVER_BUTTON_WIDTH/2;
            CarDriverAlarmY =413;
            ui->pushButton8Car8DriverAlarm->move(CarDriverAlarmX,CarDriverAlarmY);
            ui->pushButton8Car8DriverAlarm->resize(DEVICE_DRIVER_BUTTON_WIDTH,DEVICE_DRIVER_BUTTON_HEIGHT);
            ui->pushButton8Car8DriverAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/driver_alarm.png)}");
        }

        else if(g_iCarriageNum ==6)
        {
     
            int CarriageStatusWidgetX  = (DeviceStatusStackWidgetWidth - CARRIAGE_STATUS_WIDGET_WIDTH)/2;
            int CarriageStatusWidgetY  = (DeviceStatusStackWidgetHeight - CARRIAGE_STATUS_WIDGET_HEIGHT)/2;
            int CarriageStatusWidgetWidth = CARRIAGE_STATUS_WIDGET_WIDTH;
            int CarriageStatusWidgetHeight = CARRIAGE_STATUS_WIDGET_HEIGHT;
            ui->Widget6CarriageStatus ->move(CarriageStatusWidgetX,CarriageStatusWidgetY);
            ui->Widget6CarriageStatus->resize(CarriageStatusWidgetWidth,CarriageStatusWidgetHeight);

            ui->Widget6CarriageStatus->setStyleSheet("QWidget{border-image: url(:/imag/image/carriage_status_6_bg.png)}");

            //car1
            CarAlarm1X =230 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =153;
            ui->pushButtonCar1Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar1Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar1Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =260 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =153;
            ui->pushButtonCar1Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar1Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar1Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =260 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =117+(136-117-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButtonCar1NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar1NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar1NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            CarAlarm1X =293 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =153;
            ui->pushButtonCar1Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar1Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar1Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //car2
            CarAlarm1X =423 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =153;
            ui->pushButtonCar2Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar2Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar2Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            CarAlarm1X =454 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =153;
            ui->pushButtonCar2Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar2Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar2Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =454 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =117+(136-117-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButtonCar2NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar2NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar2NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            //panto
            CarPantoX  = 454- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =56- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButtonCar2PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButtonCar2PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar2PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");

            CarAlarm1X =485 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =153;
            ui->pushButtonCar2Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar2Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar2Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //car3
            CarAlarm1X =613 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =153;
            ui->pushButtonCar3Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar3Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar3Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =643 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =153;
            ui->pushButtonCar3Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar3Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar3Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =643- DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =117+(136-117-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButtonCar3NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar3NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar3NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            //panto
            CarPantoX  = 643- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =56- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButtonCar3PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButtonCar3PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar3PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");

            CarAlarm1X =674- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =153;
            ui->pushButtonCar3Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar3Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar3Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");



            //car4
            CarAlarm1X =127 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =435;
            ui->pushButtonCar4Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar4Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar4Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =156 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =435;
            ui->pushButtonCar4Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar4Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar4Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =156- DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =398+(418-398-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButtonCar4NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar4NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar4NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            //panto
            CarPantoX  = 156 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =336- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButtonCar4PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButtonCar4PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar4PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");


            CarAlarm1X =188 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =435;
            ui->pushButtonCar4Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar4Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar4Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //car5
            CarAlarm1X =315 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =435;
            ui->pushButtonCar5Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar5Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar5Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =345 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =435;
            ui->pushButtonCar5Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar5Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar5Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //nvr
            CarNVRX =345 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =398+(418-398-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButtonCar5NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar5NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar5NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            //panto
            CarPantoX  = 345 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarPantoY  =336- DEVICE_PANTO_ALARM_BUTTON_HEIGHT;
            ui->pushButtonCar5PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButtonCar5PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_PANTO_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar5PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/panto_alarm.png)}");

            CarAlarm1X =377 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =435;
            ui->pushButtonCar5Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar5Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar5Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //car6
            CarNVRX =510 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarNVRY =435;
            ui->pushButtonCar6Alarm1->move(CarNVRX,CarNVRY);
            ui->pushButtonCar6Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar6Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            CarAlarm1X =540 - DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =435;
            ui->pushButtonCar6Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar6Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar6Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");

            //nvr
            CarNVRX =540 - DEVICE_NVR_ALARM_BUTTON_WIDTH;
            CarNVRY =398+(418-398-DEVICE_NVR_ALARM_BUTTON_HEIGHT)/2;
            ui->pushButtonCar6NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar6NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH,DEVICE_NVR_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar6NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");

            CarAlarm1X =571- DEVICE_ALARM_BUTTON_WIDTH/2;
            CarAlarm1Y =435;
            ui->pushButtonCar6Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar6Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH,DEVICE_ALARM_BUTTON_HEIGHT);
            ui->pushButtonCar6Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");


            //driver
            CarDriverAlarmX = 170 - DEVICE_DRIVER_BUTTON_WIDTH/2;
            CarDriverAlarmY =137;
            ui->pushButtonCar1DriverAlarm->move(CarDriverAlarmX,CarDriverAlarmY);
            ui->pushButtonCar1DriverAlarm->resize(DEVICE_DRIVER_BUTTON_WIDTH,DEVICE_DRIVER_BUTTON_HEIGHT);
            ui->pushButtonCar1DriverAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/driver_alarm.png)}");


            CarDriverAlarmX = 703 - DEVICE_DRIVER_BUTTON_WIDTH/2;
            CarDriverAlarmY =418;
            ui->pushButtonCar6DriverAlarm->move(CarDriverAlarmX,CarDriverAlarmY);
            ui->pushButtonCar6DriverAlarm->resize(DEVICE_DRIVER_BUTTON_WIDTH,DEVICE_DRIVER_BUTTON_HEIGHT);
            ui->pushButtonCar6DriverAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image/driver_alarm.png)}");
        }

    }
    else
    {
        int DeviceStatusStackWidgetX   =  GRAPS_BETWEEN_CONTROLS;
        int DeviceStatusStackWidgetY  = MAIN_FUNCTION_BUTTON_GRAPS_600P + DEVICE_STATUS_BUTTON_HEIGHT_600P+ GRAPS_BETWEEN_CONTROLS;
        int DeviceStatusStackWidgetWidth = stackWidth - GRAPS_BETWEEN_CONTROLS*2;
        int DeviceStatusStackWidgetHeight = stackHeight - DeviceStatusStackWidgetY;
        ui->stackedWidget2 ->move(DeviceStatusStackWidgetX,DeviceStatusStackWidgetY);
        ui->stackedWidget2->resize(DeviceStatusStackWidgetWidth,DeviceStatusStackWidgetHeight);


		ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/device_status_hot.png)}");
        int CarDriverAlarmX ,CarNVRX,CarAlarm1X,CarPantoX;
        int CarDriverAlarmY,CarNVRY,CarAlarm1Y,CarPantoY;
        if(g_iCarriageNum ==8)
        {

            int CarriageStatusWidgetX  = (DeviceStatusStackWidgetWidth - CARRIAGE_STATUS_WIDGET_WIDTH_600P)/2;
            int CarriageStatusWidgetY  =GRAPS_BETWEEN_CONTROLS;
            int CarriageStatusWidgetWidth = CARRIAGE_STATUS_WIDGET_WIDTH_600P;
            int CarriageStatusWidgetHeight = CARRIAGE_STATUS_WIDGET_HEIGHT_600P;
            ui->Widget8CarriageStatus ->move(CarriageStatusWidgetX,CarriageStatusWidgetY);
            ui->Widget8CarriageStatus->resize(CarriageStatusWidgetWidth,CarriageStatusWidgetHeight);
            ui->Widget8CarriageStatus->setStyleSheet("QWidget{border-image: url(:/imag/image_800/carriage_status_8_bg.png)}");

            //car1
            CarAlarm1X =111 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car1Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car1Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car1Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =136 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car1Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car1Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car1Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =136 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =93;
            ui->pushButton8Car1NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car1NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car1NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            CarAlarm1X =161 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car1Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car1Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car1Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //car2
            CarAlarm1X =268 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car2Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car2Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car2Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            CarAlarm1X =293 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car2Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car2Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car2Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =293 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =93;
            ui->pushButton8Car2NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car2NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car2NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            //panto
            CarPantoX  = 293 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarPantoY  =40- DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P;
            ui->pushButton8Car2PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButton8Car2PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car2PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/panto_alarm.png)}");

            CarAlarm1X =318 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car2Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car2Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car2Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //car3
            CarAlarm1X =422 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car3Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car3Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car3Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =448- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car3Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car3Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car3Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =448 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =93;
            ui->pushButton8Car3NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car3NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car3NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            CarAlarm1X =472- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car3Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car3Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car3Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //car4
            CarAlarm1X =581- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car4Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car4Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car4Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =606 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car4Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car4Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car4Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =606 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =93;
            ui->pushButton8Car4NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car4NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car4NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            CarAlarm1X =631- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =121;
            ui->pushButton8Car4Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car4Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car4Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //car5
            CarAlarm1X =33 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =352;
            ui->pushButton8Car5Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car5Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car5Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =58 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =352;
            ui->pushButton8Car5Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car5Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car5Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =58 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =323;
            ui->pushButton8Car5NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car5NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car5NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            //panto
            CarPantoX  = 58 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarPantoY  =270-DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P;
            ui->pushButton8Car5PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButton8Car5PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car5PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/panto_alarm.png)}");

            CarAlarm1X =83 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =352;
            ui->pushButton8Car5Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car5Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car5Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //car6
            CarAlarm1X =189 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =352;
            ui->pushButton8Car6Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car6Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car6Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =213 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =352;
            ui->pushButton8Car6Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car6Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car6Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =213- DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =323;
            ui->pushButton8Car6NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car6NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car6NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            CarAlarm1X =239- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =352;
            ui->pushButton8Car6Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car6Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car6Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //car7
            CarNVRX =343 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarNVRY =352;
            ui->pushButton8Car7Alarm1->move(CarNVRX,CarNVRY);
            ui->pushButton8Car7Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car7Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =368 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =352;
            ui->pushButton8Car7Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car7Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car7Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =368 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =323;
            ui->pushButton8Car7NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car7NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car7NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");


            //panto
            CarPantoX  =368- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarPantoY  =270- DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P;
            ui->pushButton8Car7PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButton8Car7PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car7PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/panto_alarm.png)}");


            CarAlarm1X =393- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =353;
            ui->pushButton8Car7Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car7Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car7Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //car8
            CarNVRX =501- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarNVRY =353;
            ui->pushButton8Car8Alarm1->move(CarNVRX,CarNVRY);
            ui->pushButton8Car8Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car8Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =526 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =353;
            ui->pushButton8Car8Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car8Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car8Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =526- DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =323;
            ui->pushButton8Car8NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButton8Car8NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car8NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");


            CarAlarm1X =552 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =352;
            ui->pushButton8Car8Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButton8Car8Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButton8Car8Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //driver
            CarDriverAlarmX = 61 - DEVICE_DRIVER_BUTTON_WIDTH_600P/2;
            CarDriverAlarmY =109;
            ui->pushButton8Car1DriverAlarm->move(CarDriverAlarmX,CarDriverAlarmY);
            ui->pushButton8Car1DriverAlarm->resize(DEVICE_DRIVER_BUTTON_WIDTH_600P,DEVICE_DRIVER_BUTTON_HEIGHT_600P);
            ui->pushButton8Car1DriverAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/driver_alarm.png)}");


            CarDriverAlarmX = 659 - DEVICE_DRIVER_BUTTON_WIDTH_600P/2;
            CarDriverAlarmY =339;
            ui->pushButton8Car8DriverAlarm->move(CarDriverAlarmX,CarDriverAlarmY);
            ui->pushButton8Car8DriverAlarm->resize(DEVICE_DRIVER_BUTTON_WIDTH_600P,DEVICE_DRIVER_BUTTON_HEIGHT_600P);
            ui->pushButton8Car8DriverAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/driver_alarm.png)}");
        }


        else if(g_iCarriageNum ==6)
        {

            int CarriageStatusWidgetX  = (DeviceStatusStackWidgetWidth - CARRIAGE_STATUS_WIDGET_WIDTH_600P)/2;
            int CarriageStatusWidgetY  = GRAPS_BETWEEN_CONTROLS;
            int CarriageStatusWidgetWidth = CARRIAGE_STATUS_WIDGET_WIDTH_600P;
            int CarriageStatusWidgetHeight = CARRIAGE_STATUS_WIDGET_HEIGHT_600P;
            ui->Widget6CarriageStatus ->move(CarriageStatusWidgetX,CarriageStatusWidgetY);
            ui->Widget6CarriageStatus->resize(CarriageStatusWidgetWidth,CarriageStatusWidgetHeight);

            ui->Widget6CarriageStatus->setStyleSheet("QWidget{border-image: url(:/imag/image_800/carriage_status_6_bg.png)}");

            //car1
            CarAlarm1X =189 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =119;
            ui->pushButtonCar1Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar1Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar1Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =214 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =119;
            ui->pushButtonCar1Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar1Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar1Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =214 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =91;
            ui->pushButtonCar1NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar1NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar1NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            CarAlarm1X =239 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =119;
            ui->pushButtonCar1Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar1Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar1Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //car2
            CarAlarm1X =346 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =119;
            ui->pushButtonCar2Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar2Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar2Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            CarAlarm1X =371 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =119;
            ui->pushButtonCar2Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar2Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar2Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =371 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =91;
            ui->pushButtonCar2NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar2NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar2NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            //panto
            CarPantoX  = 371- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarPantoY  =38- DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P;
            ui->pushButtonCar2PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButtonCar2PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar2PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/panto_alarm.png)}");

            CarAlarm1X =396 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =119;
            ui->pushButtonCar2Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar2Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar2Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //car3
            CarAlarm1X =500 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =119;
            ui->pushButtonCar3Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar3Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar3Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =525 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =119;
            ui->pushButtonCar3Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar3Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar3Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =525- DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =91;
            ui->pushButtonCar3NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar3NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar3NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            CarAlarm1X =551- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =119;
            ui->pushButtonCar3Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar3Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar3Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");



            //car4
            CarAlarm1X =104 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =349;
            ui->pushButtonCar4Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar4Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar4Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =128 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =349;
            ui->pushButtonCar4Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar4Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar4Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =128- DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =321;
            ui->pushButtonCar4NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar4NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar4NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");


            CarAlarm1X =154 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =349;
            ui->pushButtonCar4Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar4Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar4Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //car5
            CarAlarm1X =257 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =349;
            ui->pushButtonCar5Alarm1->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar5Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar5Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =282 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =349;
            ui->pushButtonCar5Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar5Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar5Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //nvr
            CarNVRX =282 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =321;
            ui->pushButtonCar5NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar5NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar5NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            //panto
            CarPantoX  = 282 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarPantoY  =268- DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P;
            ui->pushButtonCar5PantoAlarm->move(CarPantoX,CarPantoY);
            ui->pushButtonCar5PantoAlarm->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_PANTO_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar5PantoAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/panto_alarm.png)}");

            CarAlarm1X =308 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =349;
            ui->pushButtonCar5Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar5Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar5Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //car6
            CarNVRX =416 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarNVRY =349;
            ui->pushButtonCar6Alarm1->move(CarNVRX,CarNVRY);
            ui->pushButtonCar6Alarm1->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar6Alarm1->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            CarAlarm1X =441 - DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =349;
            ui->pushButtonCar6Alarm2->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar6Alarm2->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar6Alarm2->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");

            //nvr
            CarNVRX =441 - DEVICE_NVR_ALARM_BUTTON_WIDTH_600P;
            CarNVRY =321;
            ui->pushButtonCar6NVRAlarm->move(CarNVRX,CarNVRY);
            ui->pushButtonCar6NVRAlarm->resize(DEVICE_NVR_ALARM_BUTTON_WIDTH_600P,DEVICE_NVR_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar6NVRAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");

            CarAlarm1X =467- DEVICE_ALARM_BUTTON_WIDTH_600P/2;
            CarAlarm1Y =349;
            ui->pushButtonCar6Alarm3->move(CarAlarm1X,CarAlarm1Y);
            ui->pushButtonCar6Alarm3->resize(DEVICE_ALARM_BUTTON_WIDTH_600P,DEVICE_ALARM_BUTTON_HEIGHT_600P);
            ui->pushButtonCar6Alarm3->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");


            //driver
            CarDriverAlarmX = 139 - DEVICE_DRIVER_BUTTON_WIDTH_600P/2;
            CarDriverAlarmY =108;
            ui->pushButtonCar1DriverAlarm->move(CarDriverAlarmX,CarDriverAlarmY);
            ui->pushButtonCar1DriverAlarm->resize(DEVICE_DRIVER_BUTTON_WIDTH_600P,DEVICE_DRIVER_BUTTON_HEIGHT_600P);
            ui->pushButtonCar1DriverAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/driver_alarm.png)}");


            CarDriverAlarmX = 574 - DEVICE_DRIVER_BUTTON_WIDTH_600P/2;
            CarDriverAlarmY =337;
            ui->pushButtonCar6DriverAlarm->move(CarDriverAlarmX,CarDriverAlarmY);
            ui->pushButtonCar6DriverAlarm->resize(DEVICE_DRIVER_BUTTON_WIDTH_600P,DEVICE_DRIVER_BUTTON_HEIGHT_600P);
            ui->pushButtonCar6DriverAlarm->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/driver_alarm.png)}");
        }
    }
	ui->stackedWidget2->setCurrentIndex(2);
}

//退出
 void UserMonitor::on_pushButtonExit_clicked()
 {

     QMessageBox::StandardButton rb =QMessageBox::question( this, FL8T("提示"), FL8T("视频监控系统, 是否退出该监控系统"));
     if(rb == QMessageBox::Yes)
     {
#if WIN32
		 exit(0);
#else
         QApplication::exit();
#endif
     }
     else
     {
         this->showFullScreen();
     }
 }

/********************************主键控制*****************************************************************************/

void UserMonitor::SetMainButtonControlState(int LastSelectState,int CurSelectState)
{
   switch(LastSelectState)
   {
   case VIDEO_REALMONITOR:
        if(iVideoFormat == VIDEO_1024_768)
        {
            ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image/VideoMonitor.png);"
                                        "background-color:rgb(5,23,89)"
                                                       "}");
        }
        else
        {
            ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/VideoMonitor.png);"
                                        "background-color:rgb(5,23,89)"
                                                       "}");
        }

       break;
    case VIDEO_PLAYBACK:
       if(iVideoFormat == VIDEO_1024_768)
       {
           ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image/Playback.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }
       else
       {
           ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Playback.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }

       break;

     case VIDEO_DOWNLOAD:
       if(iVideoFormat == VIDEO_1024_768)
       {
           ui->pushButton_VideoDownload->setStyleSheet("QPushButton{border-image: url(:/imag/image/Download.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }
       else
       {
           ui->pushButton_VideoDownload->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Download.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }

       break;


   case DEVICE_STATE:
     if(iVideoFormat == VIDEO_1024_768)
     {
         ui->pushButton_DeviceStaus->setStyleSheet("QPushButton{border-image: url(:/imag/image/DeviceState.png);"
                                      "background-color:rgb(5,23,89)"
                                      "}");
     }
     else
     {
         ui->pushButton_DeviceStaus->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/DeviceState.png);"
                                      "background-color:rgb(5,23,89)"
                                      "}");
     }

     break;

   case SOFTWARE_UPDATE:
       if(iVideoFormat == VIDEO_1024_768)
       {
           ui->pushButton_Update->setStyleSheet("QPushButton{border-image: url(:/imag/image/Update.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }
       else
       {
           ui->pushButton_Update->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Update.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }
       break;

   default:
       break;
   }

   switch(CurSelectState)
   {
   case VIDEO_REALMONITOR:
       if(iVideoFormat == VIDEO_1024_768)
       {
           ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image/VideoMonitor_On.png);"
                                       "background-color:rgb(5,23,89)"
                                                    "}");
		    if (m_enumPoll ==REALMONITOR_POLLOFF )
		    {
				ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollStart_on.png)}");
				ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollSuspend.png)}");
		    }
			else
			{
				ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollStart.png)}");
				ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollSuspend_on.png)}");
			}
		    
          
       }
       else
       {
           ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/VideoMonitor_On.png);"
                                       "background-color:rgb(5,23,89)"
                                                      "}");
		    if (m_enumPoll ==REALMONITOR_POLLOFF )
			{
				ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollStart_on.png)}");
				ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollSuspend.png)}");
			}
			else
			{
				ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollStart.png)}");
				ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollSuspend_on.png)}");
			}

       }
       break;
    case VIDEO_PLAYBACK:
       if(iVideoFormat == VIDEO_1024_768)
       {
           ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image/Playback_On.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }
       else
       {
           ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Playback_On.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }
       break;

     case VIDEO_DOWNLOAD:
      if(iVideoFormat == VIDEO_1024_768)
      {
          ui->pushButton_VideoDownload->setStyleSheet("QPushButton{border-image: url(:/imag/image/Download_On.png);"
                                       "background-color:rgb(5,23,89)"
                                       "}");
      }
      else
      {
          ui->pushButton_VideoDownload->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Download_On.png);"
                                       "background-color:rgb(5,23,89)"
                                       "}");
      }

       break;


   case DEVICE_STATE:
    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->pushButton_DeviceStaus->setStyleSheet("QPushButton{border-image: url(:/imag/image/DeviceState_On.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");
    }
    else
    {
        ui->pushButton_DeviceStaus->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/DeviceState_On.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");
    }
    break;

   case SOFTWARE_UPDATE:
       if(iVideoFormat == VIDEO_1024_768)
       {
           ui->pushButton_Update->setStyleSheet("QPushButton{border-image: url(:/imag/image/Update_On.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }
       else
       {
           ui->pushButton_Update->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Update_On.png);"
                                        "background-color:rgb(5,23,89)"
                                        "}");
       }
       break;

   default:
       break;
   }
}

void UserMonitor::StopPlayVideo()
{
}

 //实时监控
void UserMonitor::on_pushButton_VideoMonitor_clicked()
{
    if(MainState == VIDEO_REALMONITOR)
        return ;

    StopPlayVideo();
    SetMainButtonControlState(MainState,VIDEO_REALMONITOR);

    MainState = VIDEO_REALMONITOR;
    ui->stackedWidget->setCurrentIndex(0);
    SubState = PLAYBACK_STOP;


    int iCurPlay = m_pSdk->getCurrentMonitoriCarr();
    if(iCurPlay <= 0)
    {
        SetCarrriage(m_iCurSelectCarriageIdx, true);
    }

}

//回放
void UserMonitor::on_pushButton_VideoPlayback_clicked()
{
    if(MainState == VIDEO_PLAYBACK)
        return ;

    StopPlayVideo();
    SetMainButtonControlState(MainState,VIDEO_PLAYBACK);

    MainState = VIDEO_PLAYBACK;
    ui->stackedWidget->setCurrentIndex(2);
    SubState = PLAYBACK_STOP;

}

//下载
void UserMonitor::on_pushButton_VideoDownload_clicked()
{
    if(MainState == VIDEO_DOWNLOAD)
        return ;

    StopPlayVideo();
    SetMainButtonControlState(MainState,VIDEO_DOWNLOAD);
    ui->stackedWidget->setCurrentIndex(2);

    MainState = VIDEO_DOWNLOAD;
    SubState = PLAYBACK_STOP;

}

//设备状态
void UserMonitor::on_pushButton_DeviceStaus_clicked()
{
    if(MainState == DEVICE_STATE)
        return ;

    SetMainButtonControlState(MainState,DEVICE_STATE);
    ui->stackedWidget->setCurrentIndex(3);

    MainState = DEVICE_STATE;


}

//更新
void UserMonitor::on_pushButton_Update_clicked()
{
    if(MainState == SOFTWARE_UPDATE)
        return ;

    SetMainButtonControlState(MainState,SOFTWARE_UPDATE);
    ui->stackedWidget->setCurrentIndex(4);

    MainState = SOFTWARE_UPDATE;
}

/******************************实时监控************************************************/

void UserMonitor::StopRealMonitorSlots(int iCameraIdx,int iSingleVideo)
{
    SWitchVideoWinStyleSheet();

}
void UserMonitor::on_pushButton_Car1_clicked()
{
    SetCarrriage(1);
}

void UserMonitor::on_pushButton_Car2_clicked()
{
    SetCarrriage(2);
}

void UserMonitor::on_pushButton_Car3_clicked()
{
    SetCarrriage(3);
}

void UserMonitor::on_pushButton_Car4_clicked()
{
    SetCarrriage(4);
}

void UserMonitor::on_pushButton_Car5_clicked()
{
    SetCarrriage(5);
}

void UserMonitor::on_pushButton_Car6_clicked()
{
    SetCarrriage(6);
}

void UserMonitor::on_pushButton_Car7_clicked()
{
    SetCarrriage(7);
}

void UserMonitor::on_pushButton_Car8_clicked()
{
    SetCarrriage(8);
}

//上一车厢
void UserMonitor::on_pushButton_prevCar_clicked()
{
    int iCurSelectCarriageIdx = m_iCurSelectCarriageIdx-1;
    if(iCurSelectCarriageIdx <= 0)
    {
        return;
    }
    SetCarrriage(iCurSelectCarriageIdx);
}

//下一车厢
void UserMonitor::on_pushButton_nextCar_clicked()
{
    int iCurSelectCarriageIdx = m_iCurSelectCarriageIdx+1;
    if(iCurSelectCarriageIdx >= MAX_CARRIAGE_NUM)
    {
        return;
    }
	if ( ui->pushButton_Car7->isHidden())
	{
		if(iCurSelectCarriageIdx > MAX_CARRIAGE_NUM-2)
		{
			return;
		}
	}
    SetCarrriage(iCurSelectCarriageIdx);
}


//轮询开始
void UserMonitor::on_pushButton_pollstart_clicked()
{
    if(m_enumPoll == REALMONITOR_POLLON)
        return ;

    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollStart.png)}");
        ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollSuspend_on.png)}");

    }
    else
    {
        ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollStart.png)}");
        ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollSuspend_on.png)}");
    }

    m_enumPoll = REALMONITOR_POLLON;
	m_pVideoPollTimer->setInterval(m_iPollTimerSeconds*1000);
	m_pVideoPollTimer->start();
}

//轮询暂停
void UserMonitor::on_pushButton_pollsuspend_clicked()
{
    if(m_enumPoll == REALMONITOR_POLLOFF)
        return ;

    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollSuspend.png)}");
        ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollStart_on.png)}");

    }
    else
    {
        ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollSuspend.png)}");
        ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollStart_on.png)}");
    }

    m_enumPoll =REALMONITOR_POLLOFF ;
	m_pVideoPollTimer->stop();
}


void UserMonitor::on_pushButton_nextPage_clicked()
{

}

/*-------------------------录像回放-------------------------------*/
void UserMonitor::StopPlayBackSlots(int iCameraIdx,int bSingleVideo)
{
    SWitchVideoWinStyleSheet();
}

void UserMonitor::on_pushButton_BackCar1_clicked()
{
    if(CurBackSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE1 || MMS_DEVICE_TYPE_CARRIAGE1 >=g_iCarriageNum)
        return ;

    StopPlayVideo();

    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE1;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }
    else
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE1;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
    }

    SetPantoStyleSheet(CurBackSelectCarriageIdx);
    SubState = PLAYBACK_STOP;

}

void UserMonitor::on_pushButton_BackCar2_clicked()
{
    if(CurBackSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE2 || MMS_DEVICE_TYPE_CARRIAGE2 >=g_iCarriageNum)
        return ;

    StopPlayVideo();
    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE2;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
    }
    else
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE2;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }

    SetPantoStyleSheet(CurBackSelectCarriageIdx);
    SubState = PLAYBACK_STOP;


}

void UserMonitor::on_pushButton_BackCar3_clicked()
{
    if(CurBackSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE3 || MMS_DEVICE_TYPE_CARRIAGE3 >=g_iCarriageNum)
        return ;

    StopPlayVideo();
    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE3;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
    }
    else
    {

        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE3;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }

    SetPantoStyleSheet(CurBackSelectCarriageIdx);
    SubState = PLAYBACK_STOP;

}

void UserMonitor::on_pushButton_BackCar4_clicked()
{
    if(CurBackSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE4 || MMS_DEVICE_TYPE_CARRIAGE4 >=g_iCarriageNum)
        return ;

   // m_Devices[CurBackSelectCarriageIdx]->PlayBackThread->StopPlayBack();
    StopPlayVideo();

    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE4;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }
    else
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE4;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
    }

    SetPantoStyleSheet(CurBackSelectCarriageIdx);
    SubState = PLAYBACK_STOP;

}

void UserMonitor::on_pushButton_BackCar5_clicked()
{
    if(CurBackSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE5 || MMS_DEVICE_TYPE_CARRIAGE5 >=g_iCarriageNum)
        return ;

   // m_Devices[CurBackSelectCarriageIdx]->PlayBackThread->StopPlayBack();
    StopPlayVideo();
    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE5;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }
    else
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE5;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }

    SetPantoStyleSheet(CurBackSelectCarriageIdx);
    SubState = PLAYBACK_STOP;

}

void UserMonitor::on_pushButton_BackCar6_clicked()
{
    if(CurBackSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE6 || MMS_DEVICE_TYPE_CARRIAGE6 >=g_iCarriageNum)
        return ;

   // m_Devices[CurBackSelectCarriageIdx]->PlayBackThread->StopPlayBack();
    StopPlayVideo();

    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE6;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }
    else
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE6;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }

    SetPantoStyleSheet(CurBackSelectCarriageIdx);
    SubState = PLAYBACK_STOP;

}

void UserMonitor::on_pushButton_BackCar7_clicked()
{
    if(CurBackSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE7 || MMS_DEVICE_TYPE_CARRIAGE7 >=g_iCarriageNum)
        return ;

   // m_Devices[CurBackSelectCarriageIdx]->PlayBackThread->StopPlayBack();
    StopPlayVideo();

    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE7;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }
    else
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE7;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }

    SetPantoStyleSheet(CurBackSelectCarriageIdx);
    SubState = PLAYBACK_STOP;

}

void UserMonitor::on_pushButton_BackCar8_clicked()
{
    if(CurBackSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE8 || MMS_DEVICE_TYPE_CARRIAGE8 >=g_iCarriageNum)
        return ;

    //m_Devices[CurBackSelectCarriageIdx]->PlayBackThread->StopPlayBack();
    StopPlayVideo();

    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE8;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }
    else
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg(".png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet);


        CurBackSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE8;
        QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurBackSelectCarriageIdx+1).arg("_on.png)}");
        m_BackCarButton[CurBackSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }

    SetPantoStyleSheet(CurBackSelectCarriageIdx);
    SubState = PLAYBACK_STOP;
}

//开始回放
void UserMonitor::on_pushButton_play_clicked()
{

    if(SubState != PLAYBACK_STOP)
        return ;

    QDate  startData =ui->dateEditStartTime->date();
    QDate  endData =ui->dateEditEndTime->date();
    QTime  startTime =ui->timeEditStartTime->time();
    QTime  endTime =ui->timeEditEndTime->time();
    QDateTime StartDataTime(startData,startTime);
    QDateTime StopDataTime(endData,endTime);


    //m_Devices[CurBackSelectCarriageIdx]->PlayBackThread->SetPlayBackTime(StartDataTime,StopDataTime,true,m_iPlayBackCurrentPage);

    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->pushButton_play->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play.png)}");
        ui->pushButton_suspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop_on.png)}");
        ui->pushButton_back->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back_on.png)}");
        ui->pushButton_forward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward_on.png)}");
    }
    else
    {
        ui->pushButton_play->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play.png)}");
        ui->pushButton_suspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop_on.png)}");
        ui->pushButton_back->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back_on.png)}");
        ui->pushButton_forward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward_on.png)}");

    }

    SubState =PLAYBACK_START;
}


//停止回放
void UserMonitor::on_pushButton_suspend_clicked()
{
    if(SubState == PLAYBACK_STOP)
        return ;

    StopPlayVideo();

    SubState = PLAYBACK_STOP;

}

//后退
void UserMonitor::on_pushButton_back_clicked()
{

    if(SubState == PLAYBACK_STOP)
        return ;

    //m_Devices[CurBackSelectCarriageIdx]->PlayBackThread->SetPlayBackStatus(NET_DVR_PLAYSLOW);

    SubState = PLAYBACK_BACKOFF;
}

//快进
void UserMonitor::on_pushButton_forward_clicked()
{

    if(SubState == PLAYBACK_STOP)
        return ;

    //m_Devices[CurBackSelectCarriageIdx]->PlayBackThread->SetPlayBackStatus(NET_DVR_PLAYFAST);

    SubState = PLAYBACK_FASTFORWARD;
}

void UserMonitor::on_pushButton_BacknextPage_clicked()
{
     StopPlayVideo();
     m_iPlayBackCurrentPage = (++m_iPlayBackCurrentPage%MAX_CARRIAGE_PAGE_NUM);
    SubState = PLAYBACK_STOP;
}

//日回放
void UserMonitor::on_pushButton_DayPlayBack_clicked()
{

   // if(SubState != PLAYBACK_STOP)
     //   return ;


    QTime CurTime =QTime::currentTime();
  //  QTime m_StartTime =CurTime
  //  QTime m_StopTime =CurTime;
    QDate CurData =QDate::currentDate();
    QDate StartData =CurData.addDays(0-1);
    QDate EndData =CurData;

    QDateTime StartDataTime(StartData,CurTime);
    QDateTime StopDataTime(EndData,CurTime);

    //m_Devices[CurBackSelectCarriageIdx]->PlayBackThread->SetPlayBackTime(StartDataTime,StopDataTime,true,m_iPlayBackCurrentPage);

    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->pushButton_play->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play.png)}");
        ui->pushButton_suspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop_on.png)}");
        ui->pushButton_back->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back_on.png)}");
        ui->pushButton_forward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward_on.png)}");

    }
    else
    {
        ui->pushButton_play->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play.png)}");
        ui->pushButton_suspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop_on.png)}");
        ui->pushButton_back->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back_on.png)}");
        ui->pushButton_forward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward_on.png)}");
    }

    SubState = PLAYBACK_DAYPLAY;

}

//月回放
void UserMonitor::on_pushButton_MonthPlayBack_clicked()
{


}


/**************************************录像下载*****************************************************/
void UserMonitor::on_comboBoxCarriageSelect_currentIndexChanged(int index)
{
    if(index < 0 || index >= g_iCarriageNum)
    {
        return;
    }
    QString str;
    ui->comboBoxCameraSelect->clear();
    int nCam = 0;
    int i=0;
    for(i=0; i<g_atCarriages[index].cIpcNum; i++)
    {
        nCam = g_atCarriages[index].acIpcPos[i];
        str = FL8T("相机%1").arg(nCam);
        //qDebug() << str;
        ui->comboBoxCameraSelect->addItem(str);
        ui->comboBoxCameraSelect->setItemData(i, QVariant(nCam));
    }
    ui->comboBoxCameraSelect->addItem(FL8T("全部相机"));
    ui->comboBoxCameraSelect->setItemData(i, QVariant(-1));
    //ui->comboBoxCameraSelect->setFocusPolicy(Qt::NoFocus);
}

//全选槽函数
void UserMonitor::on_checkBoxSelectAllfiles_stateChanged(int arg1)
{
    //qDebug()<<"arg1:"<<arg1<<endl;
    QTableWidgetItem *item;
    for(int i=0;i<ui->tableWidgetFileList->rowCount();i++)
    {
        item =ui->tableWidgetFileList->item(i,0);
        if(item ==NULL)  continue;
        if(arg1 ==0)
        {
            item->setCheckState(Qt::Unchecked);
        }
        else if(arg1 ==2)
        {
             item->setCheckState(Qt::Checked);
        }
    }
}

//搜索结果槽函数
void UserMonitor::SearchFileResultSlot(QVariant variant)
{


}
//搜索文件的完成函数
void UserMonitor::SearchFileOver()
{
	int iNvrNo = g_atCarriages[m_iSearchCarriageIndex-1].cNvrNo;
	int iFileCount  = NVR_GetFileQueueSize(iNvrNo);
	if(iFileCount >= 0)
	{
		int iSearchCamCount = m_iSelSearchCameraIndexs.size();
		if(iSearchCamCount <= 0)
		{
			m_pSearchTimer->stop();
		}

		STATE_SetFileSearchState(E_FILE_IDLE,0);
		int iDataLen = 0;
		char *pBefore = NULL;
		char *pAfter = NULL;
		T_CMD_PACKET tPkt;
		int iLeaveDataLen = 0;
		char acData[2048] = {0};
		char acData_BK[2048] = {0};

		tPkt.iDataLen = 0;
		tPkt.pData = NULL;

		T_SEARCH_NODE tSearchNode;
		tSearchNode.iCam = m_iSearchCameraIndex;
		while(NVR_GetFileInfo(iNvrNo, &tPkt))
		{
			if(tPkt.iDataLen + iLeaveDataLen < 2048)
			{
				memcpy(acData+iLeaveDataLen,tPkt.pData,tPkt.iDataLen);
				iDataLen = tPkt.iDataLen + iLeaveDataLen;
			}
			else
			{
				memcpy(acData,tPkt.pData,tPkt.iDataLen);
				iDataLen = tPkt.iDataLen + iLeaveDataLen;
			}
			iLeaveDataLen = iDataLen;

			pBefore = acData;
			pAfter = pBefore;
			while (*pAfter != 0 && iLeaveDataLen >0)
			{
				tSearchNode.sFile = "";
				pAfter= strstr(pBefore,".MP4");
				if(pAfter)
				{
					tSearchNode.sFile.append(pBefore,pAfter-pBefore +4);
					m_aSearchFileList.push_back(tSearchNode);
					QString str(tSearchNode.sFile.c_str());
					qDebug()<<str;
					iLeaveDataLen -= pAfter-pBefore +4;
					pAfter += 4;
					pBefore = pAfter;
				}
				else
				{
					iLeaveDataLen=0;
					break;
				}
			}
			if (tPkt.pData)
			{
				free(tPkt.pData);
				tPkt.pData = NULL;
				tPkt.iDataLen = 0;
			}
			if(iLeaveDataLen >0)
			{
				while((iLeaveDataLen >0) && (0 == acData[iDataLen-iLeaveDataLen]))
				{
					iLeaveDataLen--;
				}
				if(iLeaveDataLen > 0)
				{
					memcpy(acData_BK,&acData[iDataLen-iLeaveDataLen],iLeaveDataLen);
					memset(acData,0,sizeof(acData));
					memcpy(acData,acData_BK,iLeaveDataLen);
				}
				else
				{
					iLeaveDataLen = 0;
				}
			}
			else
			{
				iLeaveDataLen = 0;
			}
		}
		if(iSearchCamCount > 0)
		{
			m_iSearchCameraIndex = m_iSelSearchCameraIndexs.at(m_iSelSearchCameraIndexs.size()-1);
			m_iSelSearchCameraIndexs.pop_back();
			searchFile();
		}
		else
		{
			if(m_aSearchFileList.size() > 0)
			{
				updateSearchFile();
			}
			if(iSearchCamCount <= 0)
			{
				m_pSearchTimer->stop();
				STATE_SetFileSearchState(E_FILE_IDLE,0);
				if(m_aSearchFileList.size() <= 0)
				{
					emit ShowWarnSignals(this, FL8T("提示"), FL8T("搜索完成"));
				}	
			}
		}

	}
	else
	{
		
			int iSearchCamCount = m_iSelSearchCameraIndexs.size();
			if(iSearchCamCount <= 0)
			{
				m_pSearchTimer->stop();
				STATE_SetFileSearchState(E_FILE_IDLE,0);
				if(m_aSearchFileList.size() <= 0)
				{
					emit ShowWarnSignals(this, FL8T("提示"), FL8T("搜索完成"));
				}	
			}
			
	}
}

//搜索文件的槽函数
void UserMonitor::SearchFileTimerSlots()
{
    static int iWaitCnt = 0;
    int iNvrNo = g_atCarriages[m_iSearchCarriageIndex-1].cNvrNo;
    int iFileCount = NVR_GetFileQueueSize(iNvrNo);
    if(iFileCount >= 0)
    {
        int iSearchCamCount = m_iSelSearchCameraIndexs.size();
        if(iSearchCamCount <= 0)
        {
            m_pSearchTimer->stop();
        }

        STATE_SetFileSearchState(E_FILE_IDLE,0);
        int iDataLen = 0;
        char *pBefore = NULL;
        char *pAfter = NULL;
        T_CMD_PACKET tPkt;
        int iLeaveDataLen = 0;
        char acData[2048] = {0};
        char acData_BK[2048] = {0};

        tPkt.iDataLen = 0;
        tPkt.pData = NULL;

        T_SEARCH_NODE tSearchNode;
        tSearchNode.iCam = m_iSearchCameraIndex;
        while(NVR_GetFileInfo(iNvrNo, &tPkt))
        {
            if(tPkt.iDataLen + iLeaveDataLen < 2048)
            {
                memcpy(acData+iLeaveDataLen,tPkt.pData,tPkt.iDataLen);
                iDataLen = tPkt.iDataLen + iLeaveDataLen;
            }
            else
            {
                memcpy(acData,tPkt.pData,tPkt.iDataLen);
                iDataLen = tPkt.iDataLen + iLeaveDataLen;
            }
            iLeaveDataLen = iDataLen;

            pBefore = acData;
            pAfter = pBefore;
            while (*pAfter != 0 && iLeaveDataLen >0)
            {
                tSearchNode.sFile = "";
                pAfter= strstr(pBefore,".MP4");
                if(pAfter)
                {
                    tSearchNode.sFile.append(pBefore,pAfter-pBefore +4);
                    m_aSearchFileList.push_back(tSearchNode);
					QString str(tSearchNode.sFile.c_str());
					qDebug()<<str;
                    iLeaveDataLen -= pAfter-pBefore +4;
                    pAfter += 4;
                    pBefore = pAfter;
                }
				else
				{
					iLeaveDataLen=0;
					break;
				}
            }
            if (tPkt.pData)
            {
                free(tPkt.pData);
                tPkt.pData = NULL;
                tPkt.iDataLen = 0;
            }
            if(iLeaveDataLen >0)
            {
                while((iLeaveDataLen >0) && (0 == acData[iDataLen-iLeaveDataLen]))
                {
                    iLeaveDataLen--;
                }
                if(iLeaveDataLen > 0)
                {
                    memcpy(acData_BK,&acData[iDataLen-iLeaveDataLen],iLeaveDataLen);
                    memset(acData,0,sizeof(acData));
                    memcpy(acData,acData_BK,iLeaveDataLen);
                }
                else
                {
                    iLeaveDataLen = 0;
                }
            }
            else
            {
                iLeaveDataLen = 0;
            }
        }
        if(iSearchCamCount > 0)
        {
            m_iSearchCameraIndex = m_iSelSearchCameraIndexs.at(m_iSelSearchCameraIndexs.size()-1);
            m_iSelSearchCameraIndexs.pop_back();
            searchFile();
        }
        else
        {
            if(m_aSearchFileList.size() > 0)
            {
                updateSearchFile();
            }
			if(iSearchCamCount <= 0)
			{
				m_pSearchTimer->stop();
				STATE_SetFileSearchState(E_FILE_IDLE,0);
				if(m_aSearchFileList.size() <= 0)
				{
					emit ShowWarnSignals(this, FL8T("提示"), FL8T("搜索完成"));
				}	
			}
        }

    }
    else
    {
        iWaitCnt++;
        if(iWaitCnt > 1)
        {
            iWaitCnt = 0;
            int iSearchCamCount = m_iSelSearchCameraIndexs.size();
            if(iSearchCamCount <= 0)
            {
                m_pSearchTimer->stop();
                STATE_SetFileSearchState(E_FILE_IDLE,0);
                if(m_aSearchFileList.size() <= 0)
                {
                    showWarn(this, FL8T(""), FL8T("搜索完成"));
                }
                else
                {
                    updateSearchFile();
                }
            }
            else
            {
                m_iSearchCameraIndex = m_iSelSearchCameraIndexs.at(m_iSelSearchCameraIndexs.size()-1);
                m_iSelSearchCameraIndexs.pop_back();
                searchFile();
            }
        }
    }
}

int UserMonitor::searchFile()
{
    T_NVR_SEARCH_RECORD tSearchInfo;
    tSearchInfo.iCarriageNo = m_iSearchCarriageIndex;
    tSearchInfo.iIpcPos = m_iSearchCameraIndex;

    QDate stDate = ui->dateEditDownloadStartTime->date();
    QTime stTime = ui->dateEditDownloadStartTime->time();
    QDate endDate = ui->dateEditDownloadEndTime->date();
    QTime endTime = ui->dateEditDownloadEndTime->time();
    tSearchInfo.tStartTime.year = htons(stDate.year());
    tSearchInfo.tStartTime.mon = stDate.month();
    tSearchInfo.tStartTime.day = stDate.day();
    tSearchInfo.tStartTime.hour = stTime.hour();
    tSearchInfo.tStartTime.min = stTime.minute();
    tSearchInfo.tStartTime.sec = stTime.second();

    tSearchInfo.tEndTime.year = htons(endDate.year());
    tSearchInfo.tEndTime.mon = endDate.month();
    tSearchInfo.tEndTime.day = endDate.day();
    tSearchInfo.tEndTime.hour = endTime.hour();
    tSearchInfo.tEndTime.min = endTime.minute();
    tSearchInfo.tEndTime.sec = endTime.second();

    return m_pSdk->searchRecordFiles(&tSearchInfo);
}

//开始查询
void UserMonitor::on_pushButtonStartQuery_clicked()
{
    int iRet = -1;
    int iCarriageIndex = ui->comboBoxCarriageSelect->currentIndex();
    int iCamIndex = ui->comboBoxCameraSelect->currentIndex();
    if(iCarriageIndex < 0 || iCamIndex < 0)
    {
        return;
    }
    if(m_pSearchTimer->isActive())
    {
        return;
    }

    QDate stDate = ui->dateEditDownloadStartTime->date();
    QTime stTime = ui->dateEditDownloadStartTime->time();
    QDate endDate = ui->dateEditDownloadEndTime->date();
    QTime endTime = ui->dateEditDownloadEndTime->time();
    QDateTime sdt(stDate, stTime);
    QDateTime edt(endDate, endTime);
    if(sdt >= edt)
    {
        showWarn(this, FL8T("提示"), FL8T("搜索时间无效,请选择正确的开始/结束时间"));
        return;
    }

    m_iSelSearchCameraIndexs.clear();
    m_iSearchCarriageIndex = iCarriageIndex+1;
    m_iSearchCameraIndex = ui->comboBoxCameraSelect->itemData(iCamIndex).toInt();
    if(m_iSearchCameraIndex < 0)
    {
        m_iSearchCameraIndex = g_atCarriages[iCarriageIndex].acIpcPos[0];
        for(char i = g_atCarriages[iCarriageIndex].cIpcNum-1; i > 0; i--)
        {
            m_iSelSearchCameraIndexs.push_back(g_atCarriages[iCarriageIndex].acIpcPos[i]);
        }
    }
    //else
    {
        iRet = searchFile();
        if(iRet == 0)
        {
            ui->tableWidgetFileList->clearContents();
            m_aSearchFileList.clear();
            m_iSearchFileCnt = -1;
            m_pSearchTimer->start();
        }
        else
        {
            showWarn(this, FL8T(""), FL8T("搜索失败"));
        }
    }

}

void UserMonitor::updateSearchFile()
{
    // headerText<<"文件名"<<"车厢"<<"相机"<<"";
    int iCount = m_aSearchFileList.size();
    ui->tableWidgetFileList->clearContents();
    ui->tableWidgetFileList->setRowCount(iCount);
    QTableWidgetItem *pItem = NULL;

    bool bCheck = ui->checkBoxSelectAllfiles->isChecked();
    int nIndex = -1;
    QString sPath, sFile;
    for(int i = 0; i < iCount; i++)
    {
        const T_SEARCH_NODE &tNode = m_aSearchFileList[i];
        sPath = QString::fromStdString(tNode.sFile);
        qDebug() << sPath;
        nIndex = sPath.lastIndexOf("/");
        if(nIndex > 0)
        {
            sFile = sPath.mid(nIndex+1);
            sPath = sPath.left(nIndex+1);
        }
        else
        {
            sFile = sPath;
        }
        pItem = new QTableWidgetItem(sFile);
        if(bCheck)
        {
            pItem->setCheckState(Qt::Checked);
        }
        else
        {
            pItem->setCheckState(Qt::Unchecked);
        }
        ui->tableWidgetFileList->setItem(i, 0, pItem);

        pItem = new QTableWidgetItem(FL8T("车厢%1").arg(m_iSearchCarriageIndex));
        ui->tableWidgetFileList->setItem(i, 1, pItem);

        pItem = new QTableWidgetItem(FL8T("相机%1").arg(tNode.iCam));
        ui->tableWidgetFileList->setItem(i, 2, pItem);
    }
	ui->tableWidgetFileList->show();
}

void UserMonitor::VideoPollTimeSlots()
{
	if (REALMONITOR_POLLON==m_enumPoll)
	{
		int iCurSelectCarriageIdx = m_iCurSelectCarriageIdx+1;
		if(iCurSelectCarriageIdx > MAX_CARRIAGE_NUM)
		{
			iCurSelectCarriageIdx=m_iCurSelectCarriageIdx = 0;
		}
		if ( ui->pushButton_Car7->isHidden())
		{
			if(iCurSelectCarriageIdx > MAX_CARRIAGE_NUM-2)
			{
				iCurSelectCarriageIdx=m_iCurSelectCarriageIdx = 0;
			}
		}
		SetCarrriage(iCurSelectCarriageIdx);
	}
}
void UserMonitor::DownloadFileTimerSlots()
{
    int iProgress = STATE_GetFileDownProgress();
    int iState = STATE_GetFileDownState();
    ui->progressBar_downLoad->setValue(iProgress);
    if(iProgress < 100)
    {
        //ui->pushButtonDownload->setText(FL8T("下载..."));
    }
   
	else if (E_FILE_DOWNING!=iState)
	{
        //ui->pushButtonDownload->setText(FL8T("下载"));
        m_pSdk->UninitDownload();
        m_pDownloadtimer->stop();
	
        if(iState == E_FILE_DOWN_SUCC)
        {
            showWarn(this, FL8T("提示"), FL8T("文件下载成功"));
			ui->progressBar_downLoad->setValue(0);
        }
        else
        {
            showWarn(this, FL8T("提示"), FL8T("文件下载失败"));
        }
        STATE_SetFileDownState(E_FILE_DOWN_IDLE);


    }
	/*TO DO
	QString strFtpInfo= m_pSdk->getFtpInfo();
	if (!strFtpInfo.isEmpty())
	{
		qDebug()<<"info about qto: "<<strFtpInfo;
	}*/

}
bool UserMonitor::winIsUsb(char* volumePath) {
	HANDLE deviceHandle = CreateFileA(volumePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	STORAGE_PROPERTY_QUERY query;
	memset(&query, 0, sizeof(query));

	DWORD bytes;
	STORAGE_DEVICE_DESCRIPTOR devd;

	//STORAGE_BUS_TYPE用于记录结构，类型要初始化
	STORAGE_BUS_TYPE busType=BusTypeUnknown;

	if (DeviceIoControl(deviceHandle, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query), &devd, sizeof(devd), &bytes, NULL)) {
		busType = devd.BusType;

	}
	CloseHandle(deviceHandle);
	return busType == BusTypeUsb;
}

QString UserMonitor::usbWindPath()
{
	QString sFilePath;
	bool bRet= false;
	char szLogicalDriveStrings[256]= {0},szInputInfo[128] = {0};
	ZeroMemory(szLogicalDriveStrings, 256);
	GetLogicalDriveStringsA(256 - 1, szLogicalDriveStrings);
	PCHAR psz;
	psz = (PCHAR)szLogicalDriveStrings;
	do{

		_snprintf(szInputInfo,128,"\\\\.\\%c:",psz[0]);
		bRet = winIsUsb(szInputInfo);
		if(bRet)
		{
			sFilePath.append(psz[0]);
			sFilePath+="://";
			break;
		}
		psz += (strlen(psz) + 1);
	} while ((*psz) != '\0');
	 return sFilePath.trimmed();
}
QString UserMonitor::usbPath()
{
    char acBuf[1024] = {0};
    ExecSysCmd("cat /proc/mounts |grep /media", acBuf, 1023);
    QString sFilePath(acBuf);
    int nIndex = sFilePath.indexOf("/media");
    if(nIndex <= 0)
    {
        return "";
    }

    sFilePath = sFilePath.mid(nIndex);
    nIndex = sFilePath.indexOf(" ");
    if(nIndex > 0)
    {
        sFilePath = sFilePath.left(nIndex);
    }
    return sFilePath.trimmed();
}
//下载
void UserMonitor::on_pushButtonDownload_clicked()
{
    int iRet = -1;
    QTableWidgetItem *pItem = NULL;
    std::vector<std::string> aFiles;
    int iRows = ui->tableWidgetFileList->rowCount();
    for(int i = 0; i < iRows; i++)
    {
        pItem = ui->tableWidgetFileList->item(i, 0);
        if(pItem == NULL)
        {
            continue;
        }
        if(Qt::Checked == pItem->checkState())
        {
            aFiles.push_back(m_aSearchFileList.at(i).sFile);
        }
	}
#if _WIN32
	QString sFilePath = usbWindPath();
#else
	QString sFilePath = usbPath();
#endif
	
	if(sFilePath.isEmpty())
	{
		QString file_path = QFileDialog::getExistingDirectory(this,FL8T("请选择模板保存路径..."),"./");  
		if(file_path.isEmpty())  
		{  
			return;  
		}
		else
		{  
			qDebug() << file_path << endl;  
		}
		sFilePath = file_path;
		//showWarn(this, FL8T("警告"), FL8T(file_path));
		//return;
	}

	QFile file(sFilePath+"/_1_.txt");
    if(file.open(QFile::WriteOnly))
    {
        file.close();
        file.remove();
    }
    else
    {
        showWarn(this, FL8T("警告"), FL8T("U盘不可写!"));
        return;
    }

    ui->progressBar_downLoad->setValue(0);
    iRet = m_pSdk->downloadFile(m_iSearchCarriageIndex, aFiles, sFilePath.toStdString());
    if(iRet == 0)
    {
        m_pDownloadtimer->start();
    }
    else
    {
        showWarn(this, FL8T("警告"), FL8T(m_pSdk->GetErrorString()));
    }
}

//上一文件
void UserMonitor::on_pushButtonDownloadLastFile_clicked()
{
    if(m_iCurPlayFileIndex < 0 || m_iCurPlayFileIndex >= ui->tableWidgetFileList->rowCount())
    {
        return;
    }
    QTableWidgetItem *pItem = NULL;
    pItem = ui->tableWidgetFileList->item(m_iCurPlayFileIndex, 0);
    if(pItem)
    {
        pItem->setCheckState(Qt::Unchecked);
    }
    ui->tableWidgetFileList->setCurrentCell(-1, 0, QItemSelectionModel::SelectCurrent);
    m_iCurPlayFileIndex--;
    if(m_iCurPlayFileIndex < 0)
    {
        m_iCurPlayFileIndex =  ui->tableWidgetFileList->rowCount()-1;
    }
    ui->tableWidgetFileList->setCurrentCell(m_iCurPlayFileIndex, 0);
    pItem = ui->tableWidgetFileList->item(m_iCurPlayFileIndex, 0);
    if(pItem)
    {
        pItem->setCheckState(Qt::Checked);
    }
    on_pushButtonDownloadPlay_clicked();
}

//后退
void UserMonitor::on_pushButtonDownloadBack_clicked()
{
    if(m_iCurPos > 10)
    {
        m_pSdk->setPlayPos(m_iCurPos - 10);
    }
    //m_pSdk->setPlayBackSpeed(0.5);
}

//播放
void UserMonitor::on_pushButtonDownloadPlay_clicked()
{
    int iRet = -1;

    if(m_iSearchCameraIndex < 1 || m_iSearchCarriageIndex < 1)
    {
        return;
    }
    int nSel = ui->tableWidgetFileList->currentRow();
    if(nSel < 0)
    {
        QTableWidgetItem *pItem = NULL;
        int iRows = ui->tableWidgetFileList->rowCount();
        for(int i = 0; i < iRows; i++)
        {
            pItem = ui->tableWidgetFileList->item(i, 0);
            if(pItem == NULL)
            {
                continue;
            }
            if(Qt::Checked == pItem->checkState())
            {
                nSel = i;
                break;
            }
        }
    }
    if(nSel < 0 || nSel >= m_aSearchFileList.size())
    {
        return;
    }
    std::string sFile = m_aSearchFileList.at(nSel).sFile;
    if(sFile.length() > 4)
    {
		
        m_pSdk->stopPlayBack(0);
        m_pSdk->initPlayBack(&m_tPlayBackVideo);
		TIMER_START(SdkPlayFile);
        iRet = m_pSdk->startPlayBack(m_iSearchCarriageIndex, sFile.c_str());
		TIMER_STOP(SdkPlayFile);
		std::string  strFormatInfo;
		qDebug() << " startPlayBack Cost  " << TIMER_MSEC(SdkPlayFile) << "ms.\n";
        if(iRet < 0)
        {
            return;
        }
        m_iCurPlayFileIndex = nSel;
        m_iPlayState = CMP_STATE_STOP;
        m_iOpenMediaState = CMP_OPEN_MEDIA_UNKOWN;
        m_iCurPos = 0;
        m_iPlayRange = 0;
		
        m_pUpdatePlayBackTimer->start();
    }
}

//停止播放
void UserMonitor::on_pushButtonDownloadStop_clicked()
{
    if(m_iSearchCameraIndex < 1 || m_iSearchCarriageIndex < 1)
    {
        return;
    }
    m_pSdk->stopPlayBack(m_iSearchCameraIndex);
    m_pUpdatePlayBackTimer->stop();
    m_iCurPos = 0;
    ui->horizontalSlider->setValue(m_iCurPos);
    m_iCurPlayFileIndex = -1;
    m_iPlayState = CMP_STATE_STOP;
    m_iOpenMediaState = CMP_OPEN_MEDIA_UNKOWN;
    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->widget_VideoDownloadWin->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->pushButtonDownloadLastFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/pre_file_hot.png)}");
        ui->pushButtonDownloadBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/fast_back.png)}");
        ui->pushButtonDownloadPlay->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_hot.png)}");
        ui->pushButtonDownloadStop->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
        ui->pushButtonDownloadForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/fast_head.png)}");
        ui->pushButtonDownloadNextFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/next_file_hot.png)}");
    }
    else
    {
        ui->widget_VideoDownloadWin->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:2px solid rgb(11,78,193);}");//图片在资源文件中
        ui->pushButtonDownloadLastFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/pre_file_hot.png)}");
        ui->pushButtonDownloadBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/fast_back.png)}");
        ui->pushButtonDownloadPlay->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_hot.png)}");
        ui->pushButtonDownloadStop->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
        ui->pushButtonDownloadForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/fast_head.png)}");
        ui->pushButtonDownloadNextFile->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/next_file_hot.png)}");
    }
}

//快进
void UserMonitor::on_pushButtonDownloadForward_clicked()
{
    if(m_iPlayRange <= 0)
    {
        return;
    }
    if(m_iCurPos < m_iPlayRange -10)
    {
        m_pSdk->setPlayPos(m_iCurPos + 10);
    }
}

//下一文件
void UserMonitor::on_pushButtonDownloadNextFile_clicked()
{
    if(m_iCurPlayFileIndex < 0 || m_iCurPlayFileIndex >= ui->tableWidgetFileList->rowCount())
    {
        return;
    }
    QTableWidgetItem *pItem = NULL;
    pItem = ui->tableWidgetFileList->item(m_iCurPlayFileIndex, 0);
    if(pItem)
    {
        pItem->setCheckState(Qt::Unchecked);
    }
    ui->tableWidgetFileList->setCurrentCell(-1, 0, QItemSelectionModel::SelectCurrent);
    m_iCurPlayFileIndex++;
    if(m_iCurPlayFileIndex >= ui->tableWidgetFileList->rowCount())
    {
        m_iCurPlayFileIndex = 0;
    }
    ui->tableWidgetFileList->setCurrentCell(m_iCurPlayFileIndex, 0);
    pItem = ui->tableWidgetFileList->item(m_iCurPlayFileIndex, 0);
    if(pItem)
    {
        pItem->setCheckState(Qt::Checked);
    }

    on_pushButtonDownloadPlay_clicked();
}


/************************************设备状态*******************************************************/
//更新设备信息
void UserMonitor::GetDeviceInfoOfCarriage(bool bUpdate)
{
    int iRet = -1;
    int iCurRow = 0;
    int iCarriageNo = 0;
    QString strText;
    QTableWidgetItem *pItem = NULL;
    if(bUpdate)
    {
        ui->tableWidgetSaveStatus->clearContents();
        ui->tableWidgetSaveStatus->setRowCount(0);
    }
    T_NVR_INFO tNvrInfo;
    int iNvrNo = 0;
    for (iNvrNo = 0; iNvrNo < m_pSdk->GetNvrNum(); iNvrNo++)
    {
        NVR_SendCmdInfo(iNvrNo, CLI_SERV_MSG_TYPE_GET_NVR_STATUS, NULL, 0);

        NVR_SendCmdInfo(iNvrNo, CLI_SERV_MSG_TYPE_GET_IPC_STATUS, NULL, 0);

        if(!bUpdate)
        {
            continue;
        }
        iRet = STATE_GetNvrInfo(iNvrNo, &tNvrInfo);
        if(iRet < 0)
        {
            continue;
        }
        //状态列表更新
        iCurRow = ui->tableWidgetSaveStatus->rowCount();
        ui->tableWidgetSaveStatus->insertRow(iCurRow);
		string strOriValue = tNvrInfo.acNVRName;
		std::wstring strTempValue =PortSipUtility::string2WString(strOriValue);
		strText = PortSipUtility::Unicode2Utf8_ND(strTempValue).c_str();
		if(strText.isEmpty())
		{
			   strText = FL8T("NVR%1").arg(iNvrNo+1);;
		}
     
        pItem = new QTableWidgetItem(strText);
        pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
        ui->tableWidgetSaveStatus->setItem(iCurRow, 0, pItem);

        pItem = new QTableWidgetItem(tNvrInfo.acIp);
        pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
        ui->tableWidgetSaveStatus->setItem(iCurRow, 1, pItem);

        if(tNvrInfo.tState.sHdiskSize <= 0)
        {
            strText = FL8T("未知");
        }
        else
        {
            strText = FL8T("%1 G").arg(tNvrInfo.tState.sHdiskSize);
        }
        pItem = new QTableWidgetItem(strText);
        pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
        ui->tableWidgetSaveStatus->setItem(iCurRow,2,pItem);


        if(tNvrInfo.tState.sHdiskUseSize <= 0)
        {
            strText = FL8T("未知");
        }
        else
        {
            strText = FL8T("%1 G").arg(tNvrInfo.tState.sHdiskUseSize);
        }
        pItem = new QTableWidgetItem(strText);
        pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
        ui->tableWidgetSaveStatus->setItem(iCurRow,3,pItem);



        //NVR图形状态更新
        if (E_SERV_STATUS_CONNECT == NVR_GetConnectStatus(iNvrNo))
        {
            pItem = new QTableWidgetItem(FL8T("在线"));
            pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
            ui->tableWidgetSaveStatus->setItem(iCurRow,4,pItem);
            for(int n = 0; n < g_iCarriageNum; n++)
            {
                if(iNvrNo == g_atCarriages[n].cNvrNo)
                {
                    if(iVideoFormat == VIDEO_1024_768)
                    {
                        CarNVR[n]->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_normal.png)}");
                    }
                    else
                    {
                        CarNVR[n]->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_normal.png)}");
                    }
                }
            }
        }
        else
        {
            pItem = new QTableWidgetItem(FL8T("不在线"));
            pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
            ui->tableWidgetSaveStatus->setItem(iCurRow,4,pItem);

            for(int n = 0; n < g_iCarriageNum; n++)
            {
                if(iNvrNo == g_atCarriages[n].cNvrNo)
                {
                    if(iVideoFormat == VIDEO_1024_768)
                    {
                        CarNVR[n]->setStyleSheet("QPushButton{border-image: url(:/imag/image/nvr_alarm.png)}");
                    }
                    else
                    {
                        CarNVR[n]->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/nvr_alarm.png)}");
                    }
                }
            }
        }

        //相机
        for (int iCamIndex = 0; iCamIndex < tNvrInfo.iIPCNum; iCamIndex++)
        {

            iCarriageNo = tNvrInfo.atIpcInfo[iCamIndex].cCarriageNo;
            if(iCarriageNo < 1 || iCarriageNo > g_iCarriageNum)
            {
                break;
            }

            //相机状态列表更新
            iCurRow = ui->tableWidgetSaveStatus->rowCount();
            ui->tableWidgetSaveStatus->insertRow(iCurRow);
            strText = tNvrInfo.atIpcInfo[iCamIndex].acChannelName;
			string strOriValue = tNvrInfo.atIpcInfo[iCamIndex].acChannelName;
			std::wstring strTempValue =PortSipUtility::string2WString(strOriValue);
			strText = PortSipUtility::Unicode2Utf8_ND(strTempValue).c_str();
            if(strText.isEmpty())
            {
               strText = FL8T("%1车厢%2号相机").arg(iCarriageNo).arg((int)tNvrInfo.atIpcInfo[iCamIndex].cPos);
            }
            pItem = new QTableWidgetItem(strText);
            pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
            ui->tableWidgetSaveStatus->setItem(iCurRow,0,pItem);

            strText = tNvrInfo.atIpcInfo[iCamIndex].acIp;
            if(strText.isEmpty())
            {
                //strText = tNvrInfo.acIp;
            }
            pItem = new QTableWidgetItem(strText);
            pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
            ui->tableWidgetSaveStatus->setItem(iCurRow,1,pItem);


            strText = FL8T("");
            pItem = new QTableWidgetItem(strText);
            pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
            ui->tableWidgetSaveStatus->setItem(iCurRow,2,pItem);

            pItem = new QTableWidgetItem(strText);
            pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
            ui->tableWidgetSaveStatus->setItem(iCurRow,3,pItem);

            //相机图形状态更新
            if(E_SERV_STATUS_CONNECT == NVR_GetConnectStatus(iNvrNo) && tNvrInfo.atIpcInfo[iCamIndex].tState.cOnlineState == STATE_ONLINE)
            {
                pItem = new QTableWidgetItem(FL8T("在线"));
                pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
                ui->tableWidgetSaveStatus->setItem(iCurRow,4,pItem);

                if(iVideoFormat == VIDEO_1024_768)
                {
                   CarCamera[iCarriageNo-1][tNvrInfo.atIpcInfo[iCamIndex].cPos-1]->setStyleSheet("QPushButton{border-image: url(:/imag/image/normal.png)}");
                }
                else
                {
                   CarCamera[iCarriageNo-1][tNvrInfo.atIpcInfo[iCamIndex].cPos-1]->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/normal.png)}");
                }
            }
            else
            {
                pItem = new QTableWidgetItem(FL8T("不在线"));
                pItem->setTextAlignment(Qt::AlignCenter|Qt::AlignHCenter);
                ui->tableWidgetSaveStatus->setItem(iCurRow,4,pItem);

                if(iVideoFormat == VIDEO_1024_768)
                {
                   CarCamera[iCarriageNo-1][tNvrInfo.atIpcInfo[iCamIndex].cPos-1]->setStyleSheet("QPushButton{border-image: url(:/imag/image/alarm.png)}");
                }
                else
                {
                   CarCamera[iCarriageNo-1][tNvrInfo.atIpcInfo[iCamIndex].cPos-1]->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/alarm.png)}");
                }
            }

        }
    }//end for
}

//更新设备状态的定时器槽函数
void UserMonitor::ProcessUpdateSaveStatusTimerOut()
{
    if(this->MainState == DEVICE_STATE)
    {
        m_bUpdateDevStatus = !m_bUpdateDevStatus;
        GetDeviceInfoOfCarriage(m_bUpdateDevStatus);
    }
}

void UserMonitor::on_pushButtonDeviceStatus_clicked()
{
	return;
    if(g_iCarriageNum == 6)
    {
        ui->stackedWidget2->setCurrentIndex(0);
    }
    else if(g_iCarriageNum == 8)
    {
        ui->stackedWidget2->setCurrentIndex(1);
    }

    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/device_status.png)}");
        ui->pushButtonAlarmList->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/alarm_list.png)}");
    }
    else
    {
        ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/device_status.png)}");
        ui->pushButtonAlarmList->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/alarm_list.png)}");
    }
	GetDeviceInfoOfCarriage(m_bUpdateDevStatus);
}

void UserMonitor::on_pushButtonSaveStatus_clicked()
{
     ui->stackedWidget2->setCurrentIndex(2);
     if(iVideoFormat == VIDEO_1024_768)
     {
         ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/device_status_hot.png)}");
         ui->pushButtonAlarmList->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/alarm_list.png)}");
     }
     else
     {
         ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/device_status_hot.png)}");
         ui->pushButtonAlarmList->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/alarm_list.png)}");
     }
}

void UserMonitor::on_pushButtonAlarmList_clicked()
{
     ui->stackedWidget2->setCurrentIndex(3);
     if(iVideoFormat == VIDEO_1024_768)
     {
         ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/device_status.png)}");
         ui->pushButtonAlarmList->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/alarm_list_hot.png)}");

     }
     else
     {
         ui->pushButtonSaveStatus->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/device_status.png)}");
         ui->pushButtonAlarmList->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/alarm_list_hot.png)}");
     }

}

/*******************维护更新*******************************/
void UserMonitor::on_radioButton10s_clicked()
{
  ui->lineEditPollTIme->setEnabled(false);
}

void UserMonitor::on_radioButton20s_clicked()
{
    ui->lineEditPollTIme->setEnabled(false);
}

void UserMonitor::on_radioButton30s_clicked()
{
    ui->lineEditPollTIme->setEnabled(false);
}

void UserMonitor::on_radioButtonCustomDefine_clicked()
{
     ui->lineEditPollTIme->setEnabled(true);
}


void UserMonitor::on_pushButtonSetPollTIme_clicked()
{
    int polltime = 5;
	QString strPollTimeType="format";
    if(ui->radioButton10s->isChecked())
    {
        polltime =10;

    }
    else if(ui->radioButton20s->isChecked())
    {
        polltime =20;

    }
    else if(ui->radioButton30s->isChecked())
    {
        polltime =30;

    }
    else if(ui->radioButtonCustomDefine->isChecked())
    {
		strPollTimeType = "user";
        QString text =ui->lineEditPollTIme->text();
        polltime =text.toInt();
    }
    else
    {
        return;
    }
    
	QSqlDatabase mydb = QSqlDatabase::addDatabase("QSQLITE");
	//新建数据库（已有的情况下也可以用）
	mydb.setDatabaseName("MyDB");
	//已有数据库
	mydb.database("MyDB");
	mydb.open();
	
	
	QString sqldelete =QString("update  tb_param Set video_poll_time=('%1'),poll_type=('%2');").arg(polltime).arg(strPollTimeType);
	QSqlQuery query(mydb);
	bool isok=query.exec(sqldelete);
	mydb.close();
	m_iPollTimerSeconds = polltime;
	polltime *= 1000;
	if(polltime != pollTimer->interval())
	{
		pollTimer->stop();
		pollTimer->setInterval(polltime);
		pollTimer->start();
	}
	if (polltime!= m_pVideoPollTimer->interval())
	{
		m_pVideoPollTimer->stop();
		m_pVideoPollTimer->setInterval(polltime);
		m_pVideoPollTimer->start();
	}
}

void UserMonitor::OnPowerCancelBtnClickedSlots()
{
    QPushButton *senderObj=qobject_cast<QPushButton*>(sender());
    if(senderObj == nullptr)
    {
        return;
    }
    QModelIndex idx = ui->tableWidgetPower->indexAt(QPoint(senderObj->frameGeometry().x(),senderObj->frameGeometry().y()));
    int row = idx.row();
    if(row < 0)
    {
        return;
    }

    QTableWidgetItem *item;
    item = ui->tableWidgetPower->item(row,1);
    int iOperator = item->data(Qt::UserRole).toInt();
    int iLoginType = m_pSdk->GetUserStyle();
    if(iOperator == E_USER_INFO_SUPER_ADMIN)
    {
        return;
    }
    if(iLoginType == E_USER_INFO_ADMIN && iOperator == E_USER_INFO_ADMIN)
    {
        return;
    }

    item=ui->tableWidgetPower->item(row,0);
    QString UserName =item->text();
    QSqlDatabase mydb = QSqlDatabase::addDatabase("QSQLITE");
   //新建数据库（已有的情况下也可以用）
     mydb.setDatabaseName("MyDB");
   //已有数据库
    mydb.database("MyDB");
	mydb.open();
	
    QString sqldelete = QString("DELETE FROM tb_power WHERE User_name ='%1' AND User_operator = '%2';").arg(UserName).arg(iOperator);
    QSqlQuery query(mydb);
    bool isok=query.exec(sqldelete);
    mydb.close();
    if(isok)
    {
        ui->tableWidgetPower->removeRow(row);//清除已有的行列
        m_iUserCount--;
        updateUserTable();
    }
}

void UserMonitor::RegisterUserInfoSlots(QVariant variant,int nError)
{
    if(nError < 0)
    {
        if(nError == -1)
        {
            showWarn(this, FL8T(""), FL8T("输入参数无效"));
        }
        else if(nError == -2)
        {
            showWarn(this, FL8T(""), FL8T("密码不一致"));
        }
        else if(nError == -3) // cancel
        {
            ShowKeyboardSlots(0);
            this->showMaximized();
            this->showFullScreen();
        }
        return;
    }
    ShowKeyboardSlots(0);
    this->showMaximized();
    this->showFullScreen();

    int iRet =0;
    QTableWidgetItem *item;
    RegisterUserInfo UserInfo =variant.value<RegisterUserInfo>();

    QSqlDatabase  mydb  = QSqlDatabase::addDatabase("QSQLITE");
    //新建数据库（已有的情况下也可以用）
    mydb.setDatabaseName("MyDB");
    //已有数据库
    mydb.database("MyDB");
    bool ret =mydb.open();
	
    //qDebug()<<"data base open:"<<ret<<endl;
    // 带检查功能，如果已有数据表则不创建
    QString sqlCreatTab = QString("create table if not exists tb_power("
                 "serial_id serial primary key, "
                 "User_name varchar(32) ,"
                 "User_operator varchar(32) ,"
                 "User_password varchar(32));");


    QSqlQuery query(mydb);
    bool isok=query.exec(sqlCreatTab);
    //  qDebug()<<"creat tab:"<<isok<<endl;
    //  qDebug()<<query.lastError().text()<<endl;
	if (!isok)
	{
		showWarn(this, FL8T(""), FL8T("创建表失败"));
		return;
	}
    QString sqlSelect = QString("select * from tb_power;");
    isok=query.exec(sqlSelect);
	if (!isok)
	{
		showWarn(this, FL8T(""), FL8T("查询失败"));
		return;
	}
    QString name;
    int iOperator;
    while (query.next())
    {
        name = query.value("User_name").toString();
        iOperator = query.value("User_operator").toInt();
        if(name == UserInfo.UserName  && iOperator == UserInfo.iOperator)
        {
			showWarn(this, FL8T(""), FL8T("已存在当前用户"));
            iRet =-1;
            break;
        }
    }
    if(iRet < 0)
    {
        return;
    }
    QString sqlInsert = QString("insert into tb_power(User_name, User_operator, User_password) Values('%1', '%2', '%3');").arg(UserInfo.UserName).arg(UserInfo.iOperator).arg(UserInfo.PassWord);
    isok=query.exec(sqlInsert);
    mydb.close();
    if(!isok)
    {
		showWarn(this, FL8T(""), FL8T("插入数据失败"));
        return;
    }

    int RowCount =ui->tableWidgetPower->rowCount();
    if(m_iUserCount >= RowCount)
    {
        ui->tableWidgetPower->insertRow(RowCount);
    }
    item = new QTableWidgetItem(UserInfo.UserName);
    item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->tableWidgetPower->setItem(m_iUserCount,0,item);

    if(E_USER_INFO_ADMIN == UserInfo.iOperator)
    {
        item = new QTableWidgetItem(FL8T("管理员"));
    }
    else
    {
        item = new QTableWidgetItem(FL8T("操作员"));
    }
    item->setData(Qt::UserRole, QVariant(UserInfo.iOperator));
    item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui->tableWidgetPower->setItem(m_iUserCount,1,item);

    QPushButton *CancelButton = new QPushButton(ui->tableWidgetPower);
    CancelButton->setText(FL8T("删除"));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(OnPowerCancelBtnClickedSlots()));
    ui->tableWidgetPower->setCellWidget(m_iUserCount,2,CancelButton);


    QString rgbtext;
    QStringList qss;
    int ired = 0, igreen=0, iblue = 0;
    if(m_iUserCount % 2 ==0)
    {
        ired =255;
        igreen =255;
        iblue =255;
    }
    else
    {
        ired =232;
        igreen =232;
        iblue =232;
    }
    rgbtext=QString("rgb(%1,%2,%3)").arg(ired).arg(igreen).arg(iblue);
    qss.clear();
    qss.append(QString("QPushButton{border-style:none;border-radius:3px;color:%1;background-color:%2;%3}").arg("red").arg(rgbtext).arg(QString::fromLocal8Bit(" font-family:'微软雅黑';")));
    qss.append(QString("QPushButton:hover{color:%1;background-color:%2;}").arg("red").arg(rgbtext));
    qss.append(QString("QPushButton:pressed{color:%1;background-color:%2;}").arg("red").arg(rgbtext));

    CancelButton->setStyleSheet(qss.join(""));

    m_iUserCount++;
}

void UserMonitor::loadUserInfo()
{
    QSqlDatabase  mydb  = QSqlDatabase::addDatabase("QSQLITE");
    //新建数据库（已有的情况下也可以用）
    mydb.setDatabaseName("MyDB");
    //已有数据库
    mydb.database("MyDB");
	bool bRet = mydb.open();
    //qDebug()<<"data base open:"<<ret<<endl;

    // 带检查功能，如果已有数据表则不创建
    QString sqlCreatTab = QString("create table if not exists tb_power("
                 "serial_id serial primary key, "
                 "User_name varchar(32) ,"
                 "User_operator varchar(32) ,"
                 "User_password varchar(32));");


    QSqlQuery query(mydb);
    bool isok=query.exec(sqlCreatTab);

    QString sqlSelect = QString("select * from tb_power;");
    isok=query.exec(sqlSelect);

    QTableWidgetItem *item;
    QString name;
    int  iOperator;
    while (query.next())
    {
        name = query.value("User_name").toString();
        iOperator = query.value("User_operator").toInt();

        ui->tableWidgetPower->insertRow(m_iUserCount);
        item = new QTableWidgetItem(name);
        item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidgetPower->setItem(m_iUserCount,0,item);

        if(E_USER_INFO_SUPER_ADMIN == iOperator)
        {
            item = new QTableWidgetItem(FL8T("超级管理员"));
        }
        else if(E_USER_INFO_ADMIN == iOperator)
        {
            item = new QTableWidgetItem(FL8T("管理员"));
        }
        else
        {
            item = new QTableWidgetItem(FL8T("操作员"));
        }
        item->setData(Qt::UserRole, QVariant(iOperator));
        item->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
        ui->tableWidgetPower->setItem(m_iUserCount,1,item);

        QPushButton *CancelButton = new QPushButton(ui->tableWidgetPower);
        CancelButton->setText(FL8T("删除"));
        connect(CancelButton, SIGNAL(clicked()), this, SLOT(OnPowerCancelBtnClickedSlots()));
        ui->tableWidgetPower->setCellWidget(m_iUserCount,2,CancelButton);

        m_iUserCount++;
    }
    updateUserTable();
}

void UserMonitor::updateUserTable()
{
    QString rgbtext;
    QStringList qss;
    int ired = 0, igreen=0, iblue = 0;
    for(int i = 0; i < m_iUserCount; i++)
    {
        if(i % 2 ==0)
        {
            ired =255;
            igreen =255;
            iblue =255;
        }
        else
        {
            ired =232;
            igreen =232;
            iblue =232;
        }
        rgbtext=QString("rgb(%1,%2,%3)").arg(ired).arg(igreen).arg(iblue);
        qss.clear();
        qss.append(QString("QPushButton{border-style:none;border-radius:3px;color:%1;background-color:%2;%3}").arg("red").arg(rgbtext).arg(QString::fromLocal8Bit(" font-family:'微软雅黑';")));
        qss.append(QString("QPushButton:hover{color:%1;background-color:%2;}").arg("red").arg(rgbtext));
        qss.append(QString("QPushButton:pressed{color:%1;background-color:%2;}").arg("red").arg(rgbtext));

        QWidget *pWidget = ui->tableWidgetPower->cellWidget(i, 2);
        if(pWidget)
        {
            pWidget->setStyleSheet(qss.join(""));
        }
    }
}

void UserMonitor::on_pushButtonAddUser_clicked()
{
    int OffsetX = (this->width()-USER_REGISYER_WIN_WIDTH)/2;
    int OffsetY  =(this->height()-USER_REGISYER_WIN_HEIGHT)/2;
    RegisterDlg->move(this->x()+OffsetX,this->y()+OffsetY);
    RegisterDlg->show();
}

void UserMonitor::ShowKeyboardSlots(int nShow)
{
    if(0 == nShow)
    {
        KeyBoardDlg->hide();
        bShowKeyboard =0;
    }
    else
    {
        if(bShowKeyboard ==0)
        {
            KeyBoardDlg->show();
            bShowKeyboard =1;
        }
    }
}

void UserMonitor::KeyboardPressKeySlots(char key)
{

    if(key==BSPACE)
    {

        if(ui->lineEditPollTIme->hasFocus())//输入框1焦点
        {
            if(!ui->lineEditPollTIme->selectedText().isEmpty())
            {
                 ui->lineEditPollTIme->del();

            }
            else
            {
                ui->lineEditPollTIme->backspace();
            }

        }
        else if(ui->lineEditCar->hasFocus())//输入框2焦点
        {
            if(!ui->lineEditCar->selectedText().isEmpty())
            {
                 ui->lineEditCar->del();

            }
            else
            {
                ui->lineEditCar->backspace();
            }
        }
    }
    else
    {
        if(ui->lineEditPollTIme->hasFocus())//输入框1焦点
        {
            ui->lineEditPollTIme->insert(QString( key));
        }
        else if(ui->lineEditCar->hasFocus())//输入框2焦点
        {
            ui->lineEditCar->insert(QString( key));
        }
    }
}

bool UserMonitor::eventFilter(QObject *obj, QEvent *e)
{
    if(e->type() == QEvent::MouseButtonPress)
    {
        if(obj == ui->lineEditPollTIme)                    //判断是不是我创建的label触发了事件
        {
            if(ui->lineEditPollTIme->isEnabled())
            {
                ShowKeyboardSlots(1);
				//ui->lineEditPollTIme->setFocus();
               
            }
        }
        else if(obj == ui->lineEditCar)
        {
            ShowKeyboardSlots(1);
			//ui->lineEditCar->setFocus();
           
        }
    }
    else if(e->type() == QEvent::FocusOut)
    {
        if(obj == ui->lineEditPollTIme || obj == ui->lineEditCar)                    //判断是不是我创建的label触发了事件
        {
            ShowKeyboardSlots(0);     
        }
    }
    return QWidget::eventFilter(obj,e);        //返回默认的事件过滤器
}

//校时
void UserMonitor::on_pushButtonSetTime_clicked()
{
    if(m_nDateSetSelType == 0)
    {
        m_nDateSetSelType = 1;
        ui->labelSystemTime->hide();
        ui->dateTimeEdit_SetDT->setDateTime(QDateTime::currentDateTime());
        ui->dateTimeEdit_SetDT->show();
        ui->pushButtonSetTime->setText(FL8T("确定"));
        ui->pushButtonSetTimeCancel->show();

    }
    else if(m_nDateSetSelType == 1)
    {
        m_nDateSetSelType = 0;
        ui->dateTimeEdit_SetDT->hide();
        ui->pushButtonSetTimeCancel->hide();
        ui->labelSystemTime->show();
        ui->pushButtonSetTime->setText(FL8T("手工校时"));
        QDateTime curDateTime = ui->dateTimeEdit_SetDT->dateTime();
        QTime curTime = curDateTime.time();
        QDate curDate = curDateTime.date();

        QString sDt = "date  " + curDateTime.toString("yyyy-MM-dd");
        system(sDt.toStdString().c_str());
		QString sTime="time " +curDateTime.toString("hh:mm:ss");
        system(sTime.toStdString().c_str());

        T_TIME_INFO timeInfo;
        timeInfo.year = htons(curDate.year());
        timeInfo.mon = curDate.month();
        timeInfo.day = curDate.day();
        timeInfo.hour = curTime.hour();
        timeInfo.min = curTime.minute();
        timeInfo.sec = curTime.second();
        for(int i = 0; i < m_pSdk->GetNvrNum(); i++)
        {
            NVR_SendCmdInfo(i, CLI_SERV_MSG_TYPE_CHECK_TIME, (char*)&timeInfo, sizeof(T_TIME_INFO));
        }
    }
}

void UserMonitor::on_pushButtonSetTimeCancel_clicked()
{
    if(m_nDateSetSelType == 1)
    {
        m_nDateSetSelType = 0;
        ui->dateTimeEdit_SetDT->hide();
        ui->pushButtonSetTimeCancel->hide();
        ui->labelSystemTime->show();
        ui->pushButtonSetTime->setText(FL8T("手工校时"));
    }
}

void UserMonitor::on_pushButtonCarriageQuery_clicked()
{
    QString text = ui->lineEditCar->text();
    int nCarriageNo = text.toInt();
    for(int i = 0; i < m_pSdk->GetNvrNum(); i++)
    {
        NVR_SendCmdInfo(i, CLI_SERV_MSG_TYPE_SET_CARRIAGE_NUM, (char*)&nCarriageNo, 4);
    }
}

void UserMonitor::setLoginType(int iLoginType)
{
    if(E_USER_INFO_OTHER == iLoginType || E_USER_INFO_OPER == iLoginType)
    {
        //ui->pushButtonCancel->hide();
        ui->pushButton_DeviceStaus->hide();
        ui->pushButton_Update->hide();
        ui->pushButtonDownload->hide();
    }
    else// if(E_USER_INFO_SUPER_ADMIN == iLoginType)
    {
        //ui->pushButtonCancel->show();
        ui->pushButton_DeviceStaus->show();
        ui->pushButton_Update->show();
        ui->pushButtonDownload->show();
    }
}

void UserMonitor::on_pushButtonCancel_clicked()
{
    hide();
    on_pushButton_VideoMonitor_clicked();
    emit ChangetoLoginSignal();
}

void UserMonitor::on_horizontalSlider_valueChanged(int value)
{

}

void UserMonitor::on_horizontalSlider_sliderReleased()
{
    if(m_iOpenMediaState != CMP_OPEN_MEDIA_SUCC)
    {
        //return;
    }
    if(m_iPlayState == CMP_STATE_PLAY || m_iPlayState == CMP_STATE_FAST_FORWARD || m_iPlayState == CMP_STATE_SLOW_FORWARD)
    {
        int value  = ui->horizontalSlider->value();
        if(value >= 0 && value < m_iPlayRange)
        {
            m_pSdk->setPlayPos(value);
        }
    }
}

void UserMonitor::on_tableWidgetFileList_itemDoubleClicked(QTableWidgetItem *item)
{
    int iRet = -1;

    if(m_iSearchCameraIndex < 1 || m_iSearchCarriageIndex < 1)
    {
        return;
    }
    int nSel = item->row();
    if(nSel < 0)
    {
        return;
    }
    std::string sFile = m_aSearchFileList.at(nSel).sFile;
    if(sFile.length() > 4)
    {
        m_pSdk->stopPlayBack(0);
        m_pSdk->initPlayBack(&m_tPlayBackVideo);
        iRet = m_pSdk->startPlayBack(m_iSearchCarriageIndex, sFile.c_str());
        if(iRet < 0)
        {
            return;
        }
        m_iCurPlayFileIndex = nSel;
        m_iPlayState = CMP_STATE_STOP;
        m_iOpenMediaState = CMP_OPEN_MEDIA_UNKOWN;
        m_iCurPos = 0;
        m_iPlayRange = 0;
        m_pUpdatePlayBackTimer->start();
    }
}

unsigned int UserMonitor::UpdateThreadFunc(void *param)
{
    UserMonitor *pWidget = (UserMonitor*)param;
    pWidget->UpdateFunc();
	return 0;
}

unsigned int UserMonitor::UpdateFunc()
{
#if _WIN32
	QString sFilePath = usbWindPath();
#else
	QString sFilePath = usbPath();
#endif
    if(sFilePath.isEmpty())
    {
        emit sglUpdateState(-1, FL8T("未检测到U盘!"));
        return 0;
    }
    char acUsbPath[128] = {0};
    char UpgradeCfgFile[256] = {0};
    strncpy(acUsbPath, sFilePath.toStdString().c_str(), 127);
    acUsbPath[127] = 0;
    sprintf(UpgradeCfgFile, "%s/%s", acUsbPath, "UpgradeCfg.ini");

    qDebug() <<"usb cfg path:" << UpgradeCfgFile;
    int iRet = 0;
#ifdef WIN32
   QFileInfo file(UpgradeCfgFile);
   if(file.exists()==false)
   {
       emit sglUpdateState(-2, FL8T("未检测到升级文件!"));
       return 0;
   }
#else
     iRet = access(UpgradeCfgFile, F_OK);
    if(iRet != 0)
    {
        emit sglUpdateState(-2, FL8T("未检测到升级文件!"));
        return 0;
    }
#endif

    char acLocalPath[256] = {0};
    strncpy(acLocalPath,qApp->applicationDirPath().toStdString().c_str(), 255);
    acLocalPath[255] = 0;
    qDebug() <<"local path:" << acLocalPath;

    int iUpdateFiles = 0;
    int iFileNum = 0;
    char acBuf[256] = {0};
    char acGroup[128] = {0};
    char acValue[128] = {0};
    char acFileName[128] = {0};
    //iRet = ReadParam(UpgradeCfgFile,"[Attr]","FileNum",acBuf);
	iRet =GetPrivateProfileStringA("Attr", "FileNum", (""), acBuf, 256, UpgradeCfgFile);;
    if(iRet > 0)
    {
        iFileNum = atoi(acBuf);
    }
    for(int i=0; i<iFileNum; i++)
    {
        memset(acGroup, 0, sizeof(acGroup));
        memset(acValue, 0, sizeof(acValue));
        sprintf(acGroup,"File%d",i+1);
        //iRet = ReadParam(UpgradeCfgFile,acGroup,"FileName",acValue);
		iRet =GetPrivateProfileStringA(acGroup, "FileName", (""), acValue, 256, UpgradeCfgFile);;
        if(iRet <= 0)
        {
            continue;
        }
        memset(acFileName, 0, sizeof(acFileName));
        sprintf(acFileName, "%s%s", acUsbPath, acValue);

#ifdef WIN32
        QFileInfo file(acFileName);
        if(file.exists()==false)
        {
            emit sglUpdateState(-2, FL8T("未检测到升级文件!"));
            return 0;
        }
        iRet = 0;
#else
        iRet = access(acFileName, F_OK);
#endif
        if (0 == iRet)
        {
            emit sglUpdateState(1, FL8T("拷贝文件:%1!").arg(acValue));
#ifdef WIN32
            Sleep(1000);
#else
            usleep(100000);
#endif
            memset(acBuf, 0, sizeof(acBuf));
#ifdef WIN32
			QString strSrcFileName(acFileName);
			QString strLocalPath(acLocalPath);
			strSrcFileName.replace(("//"), "\\");
			strLocalPath.replace(("/"), "\\");
			sprintf(acBuf, "xcopy %s %s /A", strSrcFileName.toStdString().c_str(), strLocalPath.toStdString().c_str());
#else
			sprintf(acBuf, "cp %s %s", acFileName, acLocalPath);
			
			printf("%s ,ret:%d \n", acBuf, iRet);
#endif
			iRet = system(acBuf);
            iUpdateFiles ++;
        }
    }
    if(iUpdateFiles == iFileNum)
    {
        sprintf(acBuf, "echo 1 > %s/updateflag.ini", acLocalPath);

		system(acBuf);
        emit sglUpdateState(1, FL8T("拷贝文件成功，重启升级!"));
    }
    else
    {
        emit sglUpdateState(0, FL8T("未拷贝文件!"));
    }
    return 0;
}
void UserMonitor::slotSignalFormCmd(int nCmd)
{
    this->showFullScreen();
}
void UserMonitor::sltUpdateState(int iState, QString sText)
{
    ui->label_update->setText(sText);
    m_iUpdateState = -1;
    return ;
}
void UserMonitor::on_pushButtonSysUpdate_clicked()
{
    if(m_iUpdateState != -1)
    {
        showWarn(this, FL8T("警告"), FL8T("正在升级!"));
        return;
    }
    if(m_pUpdateThread == NULL)
    {
        m_pUpdateThread = new WorkerThread(this);
        m_pUpdateThread->setThreadFunc(UpdateThreadFunc, this);
        connect(this, SIGNAL(sglUpdateState(int,QString)), this, SLOT(sltUpdateState(int,QString)));
    }
    m_iUpdateState = 1;
    m_pUpdateThread->start();
}

void UserMonitor::on_pushButtonReboot_clicked()
{
#if _WIN32
	QApplication::exit();
	UpdateVersion();
	exit(0);
#else
	if(QMessageBox::Yes == QMessageBox::question(this, FL8T("提示"), FL8T("是否确定要重启系统?")))
	{
		system("reboot");
		return;
	}
	this->showFullScreen();
#endif
    
}

bool UserMonitor::UpdateVersion() {
	char szPath[MAX_PATH] = {0};
	::GetModuleFileNameA(NULL, szPath, MAX_PATH);

	std::string strFull = szPath;
	std::string strPath = strFull.substr(0, strFull.rfind("\\") + 1);
	std::string strUpdateFile = strPath + "ClientUpdate.exe";
	if(ShellExecuteA(NULL, "open", strUpdateFile.c_str(), NULL, NULL, SW_SHOW) >= (HANDLE) 32) {
		return true;
	}

	return false;
}
void UserMonitor::NVRDisconnect(int nDisconnect,int iNvrNo,void* pUserData)
{
	UserMonitor* pThis =(UserMonitor*) pUserData;
	if (pThis->m_pSdk!=NULL)
	{
		pThis->m_pSdk->NVRDisconnect(nDisconnect,iNvrNo);
	}
}
void UserMonitor::IPCOnlineStatusChange(int iNvrNo, void* ptId, void *ptState,void* pUserData)
{
	if (ptId==NULL||ptState==NULL||pUserData==NULL)
	{
		return;
	}
	PT_IPC_STATE ptIPCState =(PT_IPC_STATE) ptState;
	PT_IPC_ID pcIPCID =(PT_IPC_ID) ptId;
	UserMonitor* pThis =(UserMonitor*) pUserData;
	if (ptIPCState->cOnlineState==STATE_ONLINE)
	{
		int iCurSelectCarriageIdx = pThis->m_iCurSelectCarriageIdx;
		if(pcIPCID->cCarriageNo==iCurSelectCarriageIdx)
		{
			if (pThis->m_pSdk!=NULL&&pThis->m_enumPoll!=1)
			{
				//pThis->m_pSdk->stopMonitor(pcIPCID->cCarriageNo,pcIPCID->cPos);
				if (pThis->m_bInitPreview==false)
				{
					pThis->m_pSdk->initPreview(pThis->m_RealMonitorVideos, VIDEO_WINDOWS_COUNT);
				}
				pThis->m_pSdk->startMonitor(pcIPCID->cCarriageNo,pcIPCID->cPos);
			}
		}
	}
// 	else
// 	{
// 		if (pThis->m_pSdk!=NULL&&pThis->m_enumPoll!=1)
// 		{
// 			pThis->m_pSdk->stopMonitor(pcIPCID->cCarriageNo,pcIPCID->cPos);
// 		}
// 	}
}

void UserMonitor::SearchOver(void* pUserData)
{
	UserMonitor* pThis =(UserMonitor*) pUserData;
	if(pThis)
	{
		pThis->SearchFileOver();
	}
}

void UserMonitor::ShowWarnSlotsSDK( QString title,  QString content)
{
	 showWarn(this,title,content);
}