#include "VisitorMonitor.h"
#include "ui_VisitorMonitor.h"
#include <QDebug>
#include<QRect>

VisitorMonitor::VisitorMonitor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VisitorMonitor),
    m_iCurrentPage(0),
    iVideoFormat(0),
    LastSelectCarriageIdx(0),
    CurSelectCarriageIdx(0),
    MainState(VIDEO_REALMONITOR),
    SubState(REALMONITOR_POLLON),
    bSingleVideoWin(false),
    iSingleCameraIdx(-1)

{
    ui->setupUi(this);


    QDesktopWidget *desktop=QApplication::desktop();
   // QRect clientRect = desktop->availableGeometry();

    //获取设备可用区宽高
    //int width=clientRect.width();
   // int height=clientRect.height();

    //获取系统分辨率
    int width=desktop->width();
    int height=desktop->height();

    if(desktop->width()>=1024)
    {
       iVideoFormat = VIDEO_1024_768;
    }
    else
    {
        iVideoFormat = VIDEO_800_600;
    }
    resize(width,height);
    //setWindowFlags(Qt::FramelessWindowHint);//无边框
    setWindowFlags(Qt::FramelessWindowHint |Qt::Window);//无边框
   // showFullScreen();
   //  this->setCursor(Qt::BlankCursor);

    //setGeometry(WINDOW_OFFSETX,WINDOW_OFFSETY,SYSTEM_WIN_WIDTH,SYSTEM_WIN_HEIGHT);

    //QFont font;
    //font.setPointSize(MONITOR_FONT_SIZE);//字体大小
    //setFont(font);//其他控件一样
    QStringList qss;
    qss.append(QString("%1}").arg(QString::fromLocal8Bit(" font-family:'微软雅黑';")));
    setStyleSheet(qss.join(""));

    ResizeMainFunctionButton(width,height);
    ResizeControl(width,height);
    ResizeLabel( width,height);
    ResizeButton(width,height);
    SetControlStyleSheet();
    InitControl();
    CalVideoArea(width,height);


    ui->stackedWidget->setCurrentIndex(0);
    ui->stackedWidgetPlayCtrl->setCurrentIndex(0);

    timer =new QTimer(this);
    timer->stop();
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(ProcessTimerOut()));
    timer->start();

#if 0
    pollTimer =new QTimer(this);
    pollTimer->stop();
    pollTimer->setInterval(g_iTurnTime*1000);
    connect(pollTimer, SIGNAL(timeout()), this, SLOT(ProcessPollTimerOut()));
 #endif

    m_Videos[0]=(HWND)ui->widget_MonitorWin1->winId();
    m_Videos[1]=(HWND)ui->widget_MonitorWin2->winId();
    m_Videos[2]=(HWND)ui->widget_MonitorWin3->winId();
    m_Videos[3]=(HWND)ui->widget_MonitorWin4->winId();

    m_CarButton[0]= ui->pushButton_Car1;
    m_CarButton[1]= ui->pushButton_Car2;
    m_CarButton[2]= ui->pushButton_Car3;
    m_CarButton[3]= ui->pushButton_Car4;
    m_CarButton[4]= ui->pushButton_Car5;
    m_CarButton[5]= ui->pushButton_Car6;
    m_CarButton[6]= ui->pushButton_Car7;
    m_CarButton[7]= ui->pushButton_Car8;
}

VisitorMonitor::~VisitorMonitor()
{

    delete ui;
}

/*******************************全屏切换****************************/
void VisitorMonitor::CalVideoArea(int width,int height)
{
     if(iVideoFormat == VIDEO_1024_768)
     {
        int LengthToTopMargin = height *42/768 + MAIN_FUNCTION_BUTTON_HEIGHT+MAIN_FUNCTION_BUTTON_GRAPS*2;

        int stackWidth =width -STATCKWIDGET_LEFT_MARGIN *2;
        int stackHeight =height - LengthToTopMargin - STATCKWIDGET_BOTTOM_MARGIN;

        int VideoHeight = (stackHeight -GRAPS_BETWEEN_CONTROLS*4-CARRIAGE_SELECT_WIDGET_HEIGHT)/2;
        int VideoWidth = (stackWidth -GRAPS_BETWEEN_CONTROLS*3)/2;

         m_VideoArea[0].X = STATCKWIDGET_LEFT_MARGIN+GRAPS_BETWEEN_CONTROLS;
         m_VideoArea[0].Y = LengthToTopMargin+ GRAPS_BETWEEN_CONTROLS;
         m_VideoArea[0].Width = VideoWidth;
         m_VideoArea[0].Height = VideoHeight;

         m_VideoArea[1].X=  m_VideoArea[0].X+VideoWidth+GRAPS_BETWEEN_CONTROLS;
         m_VideoArea[1].Y = m_VideoArea[0].Y;
         m_VideoArea[1].Width = VideoWidth;
         m_VideoArea[1].Height = VideoHeight;

         m_VideoArea[2].X =  m_VideoArea[0].X;
         m_VideoArea[2].Y = m_VideoArea[0].Y+ VideoHeight+ GRAPS_BETWEEN_CONTROLS;
         m_VideoArea[2].Width = VideoWidth;
         m_VideoArea[2].Height = VideoHeight;

         m_VideoArea[3].X =  m_VideoArea[1].X;
         m_VideoArea[3].Y =  m_VideoArea[2].Y;
         m_VideoArea[3].Width = VideoWidth;
         m_VideoArea[3].Height = VideoHeight;


     }
     else
     {
        int LengthToTopMargin = height *39/600 + MAIN_FUNCTION_BUTTON_HEIGHT_600P+MAIN_FUNCTION_BUTTON_GRAPS_600P*2;

        int stackWidth =width -STATCKWIDGET_LEFT_MARGIN_600P *2;
        int stackHeight =height - LengthToTopMargin - STATCKWIDGET_BOTTOM_MARGIN_600P;

        int VideoHeight = (stackHeight -GRAPS_BETWEEN_CONTROLS*4-CARRIAGE_SELECT_WIDGET_HEIGHT_600P)/2;
        int VideoWidth = (stackWidth -GRAPS_BETWEEN_CONTROLS*3)/2;

         m_VideoArea[0].X=STATCKWIDGET_LEFT_MARGIN_600P+GRAPS_BETWEEN_CONTROLS;
         m_VideoArea[0].Y = LengthToTopMargin+ GRAPS_BETWEEN_CONTROLS;
         m_VideoArea[0].Width = VideoWidth;
         m_VideoArea[0].Height = VideoHeight;

         m_VideoArea[1].X=m_VideoArea[0].X+VideoWidth+GRAPS_BETWEEN_CONTROLS;
         m_VideoArea[1].Y = m_VideoArea[0].Y;
         m_VideoArea[1].Width = VideoWidth;
         m_VideoArea[1].Height = VideoHeight;

         m_VideoArea[2].X =  m_VideoArea[0].X;
         m_VideoArea[2].Y = m_VideoArea[0].Y+ VideoHeight+ GRAPS_BETWEEN_CONTROLS;
         m_VideoArea[2].Width = VideoWidth;
         m_VideoArea[2].Height = VideoHeight;

         m_VideoArea[3].X =  m_VideoArea[1].X;
         m_VideoArea[3].Y =  m_VideoArea[2].Y;
         m_VideoArea[3].Width = VideoWidth;
         m_VideoArea[3].Height = VideoHeight;
     }

}

void VisitorMonitor::mousePressEvent(QMouseEvent *event)
{
    int x = event->x();
    int y = event->y();
    if(x>m_VideoArea[0].X && x<  m_VideoArea[0].X+  m_VideoArea[0].Width && y> m_VideoArea[0].Y &&
            y<  m_VideoArea[0].Y+  m_VideoArea[0].Height)
    {
       SingleVideoParams              VideoParams;
      //  memset(&VideoParams,0x0,sizeof(SingleVideoParams));
       iSingleCameraIdx = m_iCurrentPage*VIDEO_WINDOWS_COUNT;
       if(MainState == VIDEO_REALMONITOR)
       {
          m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
       }
       else if(MainState == VIDEO_PLAYBACK)
       {
           QTime CurTime =QTime::currentTime();
           QTime m_StartTime =CurTime.addSecs(0-g_iPlaybackSecond);
           QTime m_StopTime =CurTime;
           QDate data =QDate::currentDate();

           QDateTime StartDataTime(data,m_StartTime);
           QDateTime StopDataTime(data,m_StopTime);

           VideoParams.StartDataTime = StartDataTime;
           VideoParams.StopDataTime = StopDataTime;
           m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();
       }
        VideoParams.iCurCarriageIdx =CurSelectCarriageIdx;
        VideoParams.iCurPage = m_iCurrentPage;
        VideoParams.iCameraIdx =m_iCurrentPage*VIDEO_WINDOWS_COUNT;
        VideoParams.iMainStatus = MainState;
        VideoParams.iPlayStatus = SubState;
        VideoParams.iPower = POWER_VISTOR;
        QVariant Variant;
        Variant.setValue(VideoParams);
        this->hide();
        emit ChangetoSingleVideoWinSignals(Variant);
        bSingleVideoWin =true;

    }
    else if(x>m_VideoArea[1].X  &&  x<  m_VideoArea[1].X+  m_VideoArea[1].Width && y> m_VideoArea[1].Y &&
            y<  m_VideoArea[1].Y+  m_VideoArea[1].Height)
    {
         SingleVideoParams  VideoParams;
       //  memset(&VideoParams,0x0,sizeof(SingleVideoParams));
         iSingleCameraIdx = m_iCurrentPage*VIDEO_WINDOWS_COUNT+1;
        if(MainState == VIDEO_REALMONITOR)
        {
           m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        }
        else if(MainState == VIDEO_PLAYBACK)
        {
            QTime CurTime =QTime::currentTime();
            QTime m_StartTime =CurTime.addSecs(0-g_iPlaybackSecond);
            QTime m_StopTime =CurTime;
            QDate data =QDate::currentDate();

            QDateTime StartDataTime(data,m_StartTime);
            QDateTime StopDataTime(data,m_StopTime);

            VideoParams.StartDataTime = StartDataTime;
            VideoParams.StopDataTime = StopDataTime;
            m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();
        }

        VideoParams.iCurCarriageIdx =CurSelectCarriageIdx;
        VideoParams.iCurPage = m_iCurrentPage;
        VideoParams.iCameraIdx =m_iCurrentPage*VIDEO_WINDOWS_COUNT+1;
        VideoParams.iMainStatus = MainState;
        VideoParams.iPlayStatus = SubState;
        VideoParams.iPower = POWER_VISTOR;
        QVariant Variant;
        Variant.setValue(VideoParams);
        this->hide();
        emit ChangetoSingleVideoWinSignals(Variant);
        bSingleVideoWin =true;
        //qDebug()<<"mouse press event"<<endl;

    }
    else if(x>m_VideoArea[2].X  &&  x<  m_VideoArea[2].X+  m_VideoArea[2].Width && y> m_VideoArea[2].Y &&
            y<  m_VideoArea[2].Y+  m_VideoArea[2].Height )
    {
        SingleVideoParams  VideoParams;
      //  memset(&VideoParams,0x0,sizeof(SingleVideoParams));
        iSingleCameraIdx = m_iCurrentPage*VIDEO_WINDOWS_COUNT+2;
       if(MainState == VIDEO_REALMONITOR)
       {
          m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
       }
       else if(MainState == VIDEO_PLAYBACK)
       {
           QTime CurTime =QTime::currentTime();
           QTime m_StartTime =CurTime.addSecs(0-g_iPlaybackSecond);
           QTime m_StopTime =CurTime;
           QDate data =QDate::currentDate();

           QDateTime StartDataTime(data,m_StartTime);
           QDateTime StopDataTime(data,m_StopTime);

           VideoParams.StartDataTime = StartDataTime;
           VideoParams.StopDataTime = StopDataTime;
           m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();
       }
        VideoParams.iCurCarriageIdx =CurSelectCarriageIdx;
        VideoParams.iCurPage = m_iCurrentPage;
        VideoParams.iCameraIdx =m_iCurrentPage*VIDEO_WINDOWS_COUNT+2;
        VideoParams.iMainStatus = MainState;
        VideoParams.iPlayStatus = SubState;
        VideoParams.iPower = POWER_VISTOR;
        QVariant Variant;
        Variant.setValue(VideoParams);
        this->hide();
        emit ChangetoSingleVideoWinSignals(Variant);
        bSingleVideoWin =true;
    }
    else if(x>m_VideoArea[3].X  &&  x<  m_VideoArea[3].X+  m_VideoArea[3].Width && y> m_VideoArea[3].Y &&
            y<  m_VideoArea[3].Y+  m_VideoArea[3].Height )
    {
        SingleVideoParams  VideoParams;
       // memset(&VideoParams,0x0,sizeof(SingleVideoParams));
       iSingleCameraIdx = m_iCurrentPage*VIDEO_WINDOWS_COUNT+3;
       if(MainState == VIDEO_REALMONITOR)
       {
          m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
       }
       else if(MainState == VIDEO_PLAYBACK)
       {
           QTime CurTime =QTime::currentTime();
           QTime m_StartTime =CurTime.addSecs(0-g_iPlaybackSecond);
           QTime m_StopTime =CurTime;
           QDate data =QDate::currentDate();

           QDateTime StartDataTime(data,m_StartTime);
           QDateTime StopDataTime(data,m_StopTime);

           VideoParams.StartDataTime = StartDataTime;
           VideoParams.StopDataTime = StopDataTime;
           m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();
       }
        VideoParams.iCurCarriageIdx =CurSelectCarriageIdx;
        VideoParams.iCurPage = m_iCurrentPage;
        VideoParams.iCameraIdx =m_iCurrentPage*VIDEO_WINDOWS_COUNT+3;
        VideoParams.iMainStatus = MainState;
        VideoParams.iPlayStatus = SubState;
        VideoParams.iPower = POWER_VISTOR;
        QVariant Variant;
        Variant.setValue(VideoParams);
        this->hide();
        emit ChangetoSingleVideoWinSignals(Variant);
        bSingleVideoWin =true;
    }

}

void VisitorMonitor::SingleVideoWinToVisorWinSlots()
{
    this->showFullScreen();
    bSingleVideoWin =false;
    iSingleCameraIdx =-1;
    for (int iIndex = 0; iIndex < g_iCarriageNum; iIndex++)
    {
        int iIndex3 = 0;
        for (int iIndex2 = 0; iIndex2 < MMS_MAX_CHANNUM; iIndex2++)
        {
            g_sCarriages[iIndex].sNVR.sChannelInfo[iIndex2].sPrivewInfo.hPlayWnd = m_Videos[iIndex3++%VIDEO_WINDOWS_COUNT];
            g_sCarriages[iIndex].sNVR.sChannelInfo[iIndex2].sVodPara.hWnd = g_sCarriages[iIndex].sNVR.sChannelInfo[iIndex2].sPrivewInfo.hPlayWnd;
        }
    }
    QThread::msleep(200);
    if(MainState == VIDEO_REALMONITOR)
    {
        //SWitchVideoWinStyleSheet();
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    }
    else if(MainState == VIDEO_PLAYBACK)
    {
        if(SubState !=PLAYBACK_STOP)
        {
           // qDebug()<<"play back***"<<endl;
            QTime CurTime =QTime::currentTime();
            QTime m_StartTime =CurTime.addSecs(0-g_iPlaybackSecond);
            QTime m_StopTime =CurTime;
            QDate data =QDate::currentDate();

            QDateTime StartDataTime(data,m_StartTime);
            QDateTime StopDataTime(data,m_StopTime);
            m_Devices[CurSelectCarriageIdx]->PlayBackThread->SetPlayBackTime(StartDataTime,StopDataTime,true,m_iCurrentPage);

        }

    }
}


/*********************************样式参数设置************************/
//设置控件样式表
void VisitorMonitor::SetControlStyleSheet()
{

    if(iVideoFormat == VIDEO_1024_768)
    {
        QPixmap pixmapWin = QPixmap(":/imag/image/Monitor_new.jpg").scaled(this->size());
        QPalette paletteWin(this->palette());
        paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
        this->setPalette(paletteWin);

        ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image/VideoMonitor_On.png);"
                                                   "background-color:rgb(5,23,89)"
                                                   "}");

        ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image/Playback.png);"
                                                    "background-color:rgb(5,23,89)"
                                                    "}");

        ui->widget_CarChose->setStyleSheet("border:2px solid rgb(11,78,193);");
        ui->stackedWidgetPlayCtrl->setStyleSheet("border:1px solid rgb(11,78,193);");


        //monitor window
        ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中

        //label
        ui->labelCarChose->setStyleSheet("border:0px solid rgb(5,23,89);");
        ui->labelPlayCtrl->setStyleSheet("border:0px solid rgb(5,23,89);");
        ui->labelPlayBackCtrl->setStyleSheet("border:0px solid rgb(5,23,89);");

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
        ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollStart.png)}");
        ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollSuspend_on.png)}");
        ui->pushButton_pantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Pantograph.png)}");
        ui->pushButton_nextPage->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/NextPage_on.png)}");


        //viedo play back
        ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
        ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
        ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
        ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");
        ui->pushButtonPlayBackpantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Pantograph.png)}");
        ui->pushButtonPlayBackNextPage->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/NextPage_on.png)}");

        //login  button
        ui->pushButton_Login->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/login.bmp)}");
        //time label
        ui->labelRealTime->setStyleSheet("color:white;"
                                         "background-color:rgb(5,23,89);");
    }
    else
    {
        QPixmap pixmapWin = QPixmap(":/imag/image_800/Monitor_new.jpg").scaled(this->size());
        QPalette paletteWin(this->palette());
        paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
        this->setPalette(paletteWin);


        ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/VideoMonitor_On.png);"
                                                   "background-color:rgb(5,23,89)"
                                                   "}");


        ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Playback.png);"
                                                    "background-color:rgb(5,23,89)"
                                                    "}");

        ui->widget_CarChose->setStyleSheet("border:2px solid rgb(11,78,193);");
        ui->stackedWidgetPlayCtrl->setStyleSheet("border:1px solid rgb(11,78,193);");


        //monitor window
        ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中

        //label
        ui->labelCarChose->setStyleSheet("border:0px solid rgb(5,23,89);");
        ui->labelPlayCtrl->setStyleSheet("border:0px solid rgb(5,23,89);");
        ui->labelPlayBackCtrl->setStyleSheet("border:0px solid rgb(5,23,89);");

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
        ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
        ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
        ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
        ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");
        ui->pushButtonPlayBackpantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Pantograph.png)}");
        ui->pushButtonPlayBackNextPage->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/NextPage_on.png)}");

        //login  button
        ui->pushButton_Login->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/login.bmp)}");
        //time label
        ui->labelRealTime->setStyleSheet("color:white;"
                                         "background-color:rgb(5,23,89);");

    }

}

void VisitorMonitor::ResizeButton(int width,int Height)
{

    if(iVideoFormat == VIDEO_1024_768)
    {
       int LoginButtonX =  width - STATCKWIDGET_LEFT_MARGIN-LOGIN_BUTTON_WIDTH;
       int LoginButtonY =9;
       ui->pushButton_Login->move(LoginButtonX,LoginButtonY);
       ui->pushButton_Login->resize(LOGIN_BUTTON_WIDTH,LOGIN_BUTTON_HEIGHT);
    }
    else
    {
        int LoginButtonX =  width - STATCKWIDGET_LEFT_MARGIN_600P-LOGIN_BUTTON_WIDTH_600P;
        int LoginButtonY =9;
        ui->pushButton_Login->move(LoginButtonX,LoginButtonY);
        ui->pushButton_Login->resize(LOGIN_BUTTON_WIDTH_600P,LOGIN_BUTTON_HEIGHT_600P);
    }

}


void VisitorMonitor::ResizeLabel(int width,int Height)
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

void VisitorMonitor::ResizeMainFunctionButton(int Width,int Height)
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
    }

}


void VisitorMonitor::ResizeControl(int Width,int Height)
{
    if(iVideoFormat == VIDEO_1024_768)
    {
        //statckwidget
        int LengthToTopMargin = Height *42/768 + MAIN_FUNCTION_BUTTON_HEIGHT+MAIN_FUNCTION_BUTTON_GRAPS*2;

        int stackWidth =Width -STATCKWIDGET_LEFT_MARGIN *2;
        int stackHeight =Height - LengthToTopMargin - STATCKWIDGET_BOTTOM_MARGIN;
        ui->stackedWidget->move(STATCKWIDGET_LEFT_MARGIN,LengthToTopMargin);
        ui->stackedWidget->resize(stackWidth,stackHeight);

        //show video label  labelHeight  stackHeight
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

        //play control stack widget
        int playCtrlWidgetX = carChoseWidgetX + labelWidth + GRAPS_BETWEEN_CONTROLS;
        int playCtrlWidgetY = carChoseWidgetY;
        ui->stackedWidgetPlayCtrl->move(playCtrlWidgetX,playCtrlWidgetY);
        ui->stackedWidgetPlayCtrl->resize(labelWidth,CARRIAGE_SELECT_WIDGET_HEIGHT);

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

        //playbacklabel
        //label
        int carPlayBackControlLabelX = (labelWidth-PLAYBACK_LABEL_WIDTH)/2;
        int carPlayBackControlLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelPlayBackCtrl->move(carControlLabelX,carControlLabelY);
        ui->labelPlayBackCtrl->resize(PLAYBACK_LABEL_WIDTH,LABEL_HEIGHT);

        //playback button
        int StartButtonX  = (labelWidth -BUTTON_WIDTH*6-GRAPS_BETWEEN_CONTROLS*5)/2;
        int StartButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN;
        ui->pushButtonPlayBackStart->move(StartButtonX,StartButtonY);
        ui->pushButtonPlayBackStart->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int SuspendButtonX  = StartButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int SuspendButtonY = StartButtonY;
        ui->pushButtonPlayBackSuspend->move(SuspendButtonX,SuspendButtonY);
        ui->pushButtonPlayBackSuspend->resize(BUTTON_WIDTH,BUTTON_HEIGHT);


        int BackButtonX = SuspendButtonX +  BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int BackButtonY = SuspendButtonY;
        ui->pushButtonPlayBackBack->move(BackButtonX,BackButtonY);
        ui->pushButtonPlayBackBack->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int ForwardButtonX = BackButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int ForwardButtonY = BackButtonY;
        ui->pushButtonPlayBackForward->move(ForwardButtonX,ForwardButtonY);
        ui->pushButtonPlayBackForward->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackpantographButtonX = ForwardButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int PlayBackpantographButtonY = ForwardButtonY;
        ui->pushButtonPlayBackpantograph->move(PlayBackpantographButtonX,PlayBackpantographButtonY);
        ui->pushButtonPlayBackpantograph->resize(BUTTON_WIDTH,BUTTON_HEIGHT);

        int PlayBackNextPageButtonX = PlayBackpantographButtonX + BUTTON_WIDTH + GRAPS_BETWEEN_CONTROLS;
        int PlayBackNextPageButtonY = PlayBackpantographButtonY;
        ui->pushButtonPlayBackNextPage->move(PlayBackNextPageButtonX,PlayBackNextPageButtonY);
        ui->pushButtonPlayBackNextPage->resize(BUTTON_WIDTH,BUTTON_HEIGHT);


    }
    else
    {
        //statckwidget
        int LengthToTopMargin = Height *39/600 + MAIN_FUNCTION_BUTTON_HEIGHT_600P+MAIN_FUNCTION_BUTTON_GRAPS_600P*2;

        int stackWidth =Width -STATCKWIDGET_LEFT_MARGIN_600P*2;
        int stackHeight =Height - LengthToTopMargin - STATCKWIDGET_BOTTOM_MARGIN_600P;
        ui->stackedWidget->move(STATCKWIDGET_LEFT_MARGIN_600P,LengthToTopMargin);
        ui->stackedWidget->resize(stackWidth,stackHeight);

        //show video label  labelHeight  stackHeight
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

        int car2ChoseButtonX = car1ChoseButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
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

        //play control stack widget
        int playCtrlWidgetX = carChoseWidgetX + labelWidth + GRAPS_BETWEEN_CONTROLS;
        int playCtrlWidgetY = carChoseWidgetY;
        ui->stackedWidgetPlayCtrl->move(playCtrlWidgetX,playCtrlWidgetY);
        ui->stackedWidgetPlayCtrl->resize(labelWidth,CARRIAGE_SELECT_WIDGET_HEIGHT_600P);

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

        //playbacklabel
        //label
        int carPlayBackControlLabelX = (labelWidth-PLAYBACK_LABEL_WIDTH_600P)/2;
        int carPlayBackControlLabelY = GRAPS_BETWEEN_CONTROLS;
        ui->labelPlayBackCtrl->move(carControlLabelX,carControlLabelY);
        ui->labelPlayBackCtrl->resize(PLAYBACK_LABEL_WIDTH_600P,LABEL_HEIGHT_600P);

        //playback button
        int StartButtonX  = (labelWidth -BUTTON_WIDTH_600P*6-GRAPS_BETWEEN_CONTROLS*5)/2;
        int StartButtonY = BUTTON_TO_CHOSEWIDGET_MARGIN_600P;
        ui->pushButtonPlayBackStart->move(StartButtonX,StartButtonY);
        ui->pushButtonPlayBackStart->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int SuspendButtonX  = StartButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int SuspendButtonY = StartButtonY;
        ui->pushButtonPlayBackSuspend->move(SuspendButtonX,SuspendButtonY);
        ui->pushButtonPlayBackSuspend->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);


        int BackButtonX = SuspendButtonX +  BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int BackButtonY = SuspendButtonY;
        ui->pushButtonPlayBackBack->move(BackButtonX,BackButtonY);
        ui->pushButtonPlayBackBack->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int ForwardButtonX = BackButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int ForwardButtonY = BackButtonY;
        ui->pushButtonPlayBackForward->move(ForwardButtonX,ForwardButtonY);
        ui->pushButtonPlayBackForward->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackpantographButtonX = ForwardButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int PlayBackpantographButtonY = ForwardButtonY;
        ui->pushButtonPlayBackpantograph->move(PlayBackpantographButtonX,PlayBackpantographButtonY);
        ui->pushButtonPlayBackpantograph->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);

        int PlayBackNextPageButtonX = PlayBackpantographButtonX + BUTTON_WIDTH_600P + GRAPS_BETWEEN_CONTROLS;
        int PlayBackNextPageButtonY = PlayBackpantographButtonY;
        ui->pushButtonPlayBackNextPage->move(PlayBackNextPageButtonX,PlayBackNextPageButtonY);
        ui->pushButtonPlayBackNextPage->resize(BUTTON_WIDTH_600P,BUTTON_HEIGHT_600P);
    }

}


void VisitorMonitor::SWitchVideoWinStyleSheet()
{
    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
    }
    else
    {
        ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.jpg);border:3px solid rgb(11,78,193);}");//图片在资源文件中

    }

}


 void VisitorMonitor::InitControl()
 {

     ui->labelCarChose->setText("<font color=white>车厢选择</b></font>");
     ui->labelPlayCtrl->setText("<font color=white>播放控制</b></font>");
     ui->labelPlayBackCtrl->setText("<font color=white>回放车厢选择</b></font>");
 }

/*************************************信号和槽*********************************************/
void VisitorMonitor::SetPantoStyleSheet(int iCarriageIndex)
{
    if(g_sCarriages[iCarriageIndex].sNVR.iPantoCarmeraNo >0)
    {
        if(MainState == VIDEO_REALMONITOR)
        {
            if(iVideoFormat == VIDEO_1024_768)
            {
                 ui->pushButton_pantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Pantograph_on.png)}");
            }
            else
            {
                 ui->pushButton_pantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Pantograph_on.png)}");
            }

        }
        else if(MainState == VIDEO_PLAYBACK)
        {
            if(iVideoFormat == VIDEO_1024_768)
            {
               ui->pushButtonPlayBackpantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Pantograph_on.png)}");
            }
            else
            {
                ui->pushButtonPlayBackpantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Pantograph_on.png)}");
            }

        }
    }
    else
    {

        if(MainState == VIDEO_REALMONITOR)
        {
            if(iVideoFormat == VIDEO_1024_768)
            {
                ui->pushButton_pantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Pantograph.png)}");
            }
            else
            {
                ui->pushButton_pantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Pantograph.png)}");
            }
        }
        else if(MainState == VIDEO_PLAYBACK)
        {
            if(iVideoFormat == VIDEO_1024_768)
            {
                ui->pushButtonPlayBackpantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Pantograph.png)}");
            }
            else
            {
                ui->pushButtonPlayBackpantograph->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Pantograph.png)}");
            }

        }
    }
}

void VisitorMonitor::toVistorWinSlot()
{
    for (int iIndex = 0; iIndex < g_iCarriageNum; iIndex++)
    {
        int iIndex3 = 0;
        for (int iIndex2 = 0; iIndex2 < MMS_MAX_CHANNUM; iIndex2++)
        {
            g_sCarriages[iIndex].sNVR.sChannelInfo[iIndex2].sPrivewInfo.hPlayWnd = m_Videos[iIndex3++%VIDEO_WINDOWS_COUNT];
            g_sCarriages[iIndex].sNVR.sChannelInfo[iIndex2].sVodPara.hWnd = g_sCarriages[iIndex].sNVR.sChannelInfo[iIndex2].sPrivewInfo.hPlayWnd;
        }

    }
    if(MainState == VIDEO_REALMONITOR)
    {
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
        pollTimer->start();
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
         SubState= REALMONITOR_POLLON;
    }
    if(g_iCarriageNum != MAX_CARRIAGE_NUM)
    {
        ui->pushButton_Car7->hide();
        ui->pushButton_Car8->hide();
    }

}

//real data time
void VisitorMonitor::ProcessTimerOut()
{
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy-MM-dd  hh:mm:ss");
    ui->labelRealTime->setText(current_date);
}


//poll timer
void VisitorMonitor::ProcessPollTimerOut()
{

    LastSelectCarriageIdx = CurSelectCarriageIdx;
    CurSelectCarriageIdx = (LastSelectCarriageIdx+1)%g_iCarriageNum;

    if(bSingleVideoWin ==true)
    {
        emit  SingleVideoWinPollSignals(CurSelectCarriageIdx);
    }
    else
    {
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();
        QThread::msleep(200);
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    }

    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
        m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

        QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
        m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
    }
    else
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
        m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

        QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
        m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }
    SetPantoStyleSheet(CurSelectCarriageIdx);

}

//创建设备
void VisitorMonitor::CreatDevice()
{
    pollTimer =new QTimer(this);
    pollTimer->stop();
    pollTimer->setInterval(g_iTurnTime*1000);
    connect(pollTimer, SIGNAL(timeout()), this, SLOT(ProcessPollTimerOut()));

    for (int iIndex = 0; iIndex < g_iCarriageNum; iIndex++)
    {
        int iIndex3 = 0;
        for (int iIndex2 = 0; iIndex2 < MMS_MAX_CHANNUM; iIndex2++)
        {
            g_sCarriages[iIndex].sNVR.sChannelInfo[iIndex2].sPrivewInfo.hPlayWnd = m_Videos[iIndex3++%VIDEO_WINDOWS_COUNT];
            g_sCarriages[iIndex].sNVR.sChannelInfo[iIndex2].sVodPara.hWnd = g_sCarriages[iIndex].sNVR.sChannelInfo[iIndex2].sPrivewInfo.hPlayWnd;
        }

        int iCarriageIndex = g_sCarriages[iIndex].iCarriageNo - 1;

        m_Devices[iCarriageIndex] = new CCarriageDevice(iCarriageIndex);
        connect(m_Devices[iCarriageIndex]->PlayBackThread,SIGNAL(StopPlayBackSignals(int,int)),this,SLOT(StopPlayBackSlots(int,int)));
        connect(m_Devices[iCarriageIndex]->RealTimeMonitorThread,SIGNAL(StopRealMonitorSignals(int,int)),this,SLOT(StopRealMonitorSlots(int,int)));
    }

    emit SucceedInitDeviceSignals(m_Devices);

}

void VisitorMonitor::InitDeviceSlots()
{
     CreatDevice();
}

/********************************主键控制*****************************************************************************/

void VisitorMonitor::StopPlayVideo()
{
    if(MainState == VIDEO_REALMONITOR)
    {
        //停止实时预览
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        if(iVideoFormat == VIDEO_1024_768)
        {
            ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollStart_on.png)}");
            ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollSuspend.png)}");

        }
        else
        {
            ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollStart_on.png)}");
            ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollSuspend.png)}");
        }
        SWitchVideoWinStyleSheet();
        pollTimer->stop();

    }
    else if(MainState==VIDEO_PLAYBACK)
    {
        //停止回放
        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();
        if(iVideoFormat == VIDEO_1024_768)
        {
            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");
        }
        else
        {
            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");
        }
        SWitchVideoWinStyleSheet();
        SubState= PLAYBACK_STOP;
    }

}


// Login
void VisitorMonitor::on_pushButton_Login_clicked()
{
    StopPlayVideo();
    emit  ChangetoLoginWinSignal();
}

//实时监控按钮
void VisitorMonitor::on_pushButton_VideoMonitor_clicked()
{
    if(MainState == VIDEO_REALMONITOR)
        return ;

    //停止回放
    StopPlayVideo();

    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image/VideoMonitor_On.png);"
                                                   "background-color:rgb(5,23,89)"
                                                   "}");

        ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image/Playback.png);"
                                                    "background-color:rgb(5,23,89)"
                                                    "}");


        ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollStart.png)}");
        ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/PollSuspend_on.png)}");

    }
    else
    {
        ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/VideoMonitor_On.png);"
                                                   "background-color:rgb(5,23,89)"
                                                   "}");

        ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Playback.png);"
                                                    "background-color:rgb(5,23,89)"
                                                    "}");

        ui->pushButton_pollstart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollStart.png)}");
        ui->pushButton_pollsuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/PollSuspend_on.png)}");
    }

    m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    ui->stackedWidgetPlayCtrl->setCurrentIndex(0);
    MainState = VIDEO_REALMONITOR;
    pollTimer->start();
    SubState= REALMONITOR_POLLON;
}

//回放按钮
void VisitorMonitor::on_pushButton_VideoPlayback_clicked()
{
    if(MainState == VIDEO_PLAYBACK)
        return ;


    //停止实时预览
    StopPlayVideo();

    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image/VideoMonitor.png);"
                                    "background-color:rgb(5,23,89)"
                                                   "}");

        ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image/Playback_On.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");

    }
    else
    {
        ui->pushButton_VideoMonitor->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/VideoMonitor.png);"
                                    "background-color:rgb(5,23,89)"
                                                   "}");

        ui->pushButton_VideoPlayback->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/Playback_On.png);"
                                     "background-color:rgb(5,23,89)"
                                     "}");
    }

    MainState =VIDEO_PLAYBACK;
    ui->stackedWidgetPlayCtrl->setCurrentIndex(1);
    SubState= PLAYBACK_STOP;
}


/*----------------------------------------实时预览-------------------------------------*/
void VisitorMonitor::StopRealMonitorSlots(int iCameraIdx,int iSingleVideo)
{
   // qDebug()<<"stop realmonitor:"<<iCameraIdx<<"iSingleVideo:"<< iSingleVideo<<endl;
    SWitchVideoWinStyleSheet();
    if(iSingleVideo && iSingleCameraIdx == iCameraIdx)
    {
        if(MainState == VIDEO_REALMONITOR)
        {
            emit SingleVideoPlaySignals();
        }
    }

}
//上一车
void VisitorMonitor::on_pushButton_prevCar_clicked()
{
    LastSelectCarriageIdx = CurSelectCarriageIdx;
    CurSelectCarriageIdx = LastSelectCarriageIdx-1;
    if(CurSelectCarriageIdx <0)
        CurSelectCarriageIdx = g_iCarriageNum - 1;

    m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
    SWitchVideoWinStyleSheet();

    if(iVideoFormat == VIDEO_1024_768)
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
        m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

        QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
        m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }
    else
    {
        QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
        m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

        QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
        m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

    }

    m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    SetPantoStyleSheet(CurSelectCarriageIdx);

}

//下一车
void VisitorMonitor::on_pushButton_nextCar_clicked()
{
    LastSelectCarriageIdx = CurSelectCarriageIdx;
    CurSelectCarriageIdx = (LastSelectCarriageIdx+1)%g_iCarriageNum;

    m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
    SWitchVideoWinStyleSheet();

    //QThread::usleep(1000);
    //qDebug()<<"set style sheet next car"<<endl;
     if(iVideoFormat == VIDEO_1024_768)
     {
         QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
         m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

         QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
         m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

     }
     else
     {
         QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
         m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

         QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
         m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
     }


    m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    SetPantoStyleSheet(CurSelectCarriageIdx);
}


//开始轮询
void VisitorMonitor::on_pushButton_pollstart_clicked()
{
    if(SubState == REALMONITOR_POLLON)
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
    pollTimer->start();
    SubState = REALMONITOR_POLLON;
}

//停止轮询
void VisitorMonitor::on_pushButton_pollsuspend_clicked()
{
    if(SubState == REALMONITOR_POLLOFF)
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

    pollTimer->stop();
    SubState =REALMONITOR_POLLOFF ;
}

//下一页
void VisitorMonitor::on_pushButton_nextPage_clicked()
{
     m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);

     SWitchVideoWinStyleSheet();
     m_iCurrentPage = (++m_iCurrentPage%MAX_CARRIAGE_PAGE_NUM);
     QThread::usleep(100);
     m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
}



/*-------------------------回放-------------------------------*/
//注：实时监控和回放用的车厢按钮是同一组，这里一个函数判断

void VisitorMonitor::StopPlayBackSlots(int iCameraIdx,int bSingleVideo)
{
    SWitchVideoWinStyleSheet();
    if(bSingleVideo && iSingleCameraIdx == iCameraIdx)
    {

        if(MainState == VIDEO_PLAYBACK && SubState !=PLAYBACK_STOP)
        {
            emit SingleVideoPlaySignals();

        }
    }
}



//车厢1
void VisitorMonitor::on_pushButton_Car1_clicked()
{
    if(CurSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE1 || MMS_DEVICE_TYPE_CARRIAGE1 >=g_iCarriageNum)
        return ;

    if( MainState == VIDEO_REALMONITOR)
    {
        LastSelectCarriageIdx = CurSelectCarriageIdx;
        CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE1;
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);

    }
    else if (MainState == VIDEO_PLAYBACK)
    {
        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE1;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");
        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

           //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE1;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");
        }
        SubState = PLAYBACK_STOP;

    }

    SetPantoStyleSheet(CurSelectCarriageIdx);
}

void VisitorMonitor::on_pushButton_Car2_clicked()
{
    if(CurSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE2 || MMS_DEVICE_TYPE_CARRIAGE2 >=g_iCarriageNum)
        return ;

    if( MainState == VIDEO_REALMONITOR)
    {
        LastSelectCarriageIdx = CurSelectCarriageIdx;
        CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE2;
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    }
    else if (MainState == VIDEO_PLAYBACK)
    {
        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE2;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE2;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");
        }
        SubState = PLAYBACK_STOP;
    }

    SetPantoStyleSheet(CurSelectCarriageIdx);

}

void VisitorMonitor::on_pushButton_Car3_clicked()
{
    if(CurSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE3 ||  MMS_DEVICE_TYPE_CARRIAGE3 >=g_iCarriageNum)
        return ;

    if( MainState == VIDEO_REALMONITOR)
    {
        LastSelectCarriageIdx = CurSelectCarriageIdx;
        CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE3;
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
        }
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    }

    else if (MainState == VIDEO_PLAYBACK)
    {
        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();
        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE3;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE3;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");

        }
        SubState = PLAYBACK_STOP;
    }

    SetPantoStyleSheet(CurSelectCarriageIdx);
}

void VisitorMonitor::on_pushButton_Car4_clicked()
{
    if(CurSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE4 ||  MMS_DEVICE_TYPE_CARRIAGE4 >=g_iCarriageNum)
        return ;

    if( MainState == VIDEO_REALMONITOR)
    {
        LastSelectCarriageIdx = CurSelectCarriageIdx;
        CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE4;
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
        }
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    }

    else if (MainState == VIDEO_PLAYBACK)
    {
        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE4;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE4;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");

        }
        SubState = PLAYBACK_STOP;
    }
    SetPantoStyleSheet(CurSelectCarriageIdx);

}

void VisitorMonitor::on_pushButton_Car5_clicked()
{
    if(CurSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE5 || MMS_DEVICE_TYPE_CARRIAGE5 >=g_iCarriageNum)
        return ;


    if( MainState == VIDEO_REALMONITOR)
    {
        LastSelectCarriageIdx = CurSelectCarriageIdx;
        CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE5;
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
        }
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    }

    else if (MainState == VIDEO_PLAYBACK)
    {
        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();
        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE5;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE5;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");

        }
        SubState = PLAYBACK_STOP;

    }
    SetPantoStyleSheet(CurSelectCarriageIdx);

}

void VisitorMonitor::on_pushButton_Car6_clicked()
{
    if(CurSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE6 || MMS_DEVICE_TYPE_CARRIAGE6 >=g_iCarriageNum)
        return ;

    if( MainState == VIDEO_REALMONITOR)
    {
        LastSelectCarriageIdx = CurSelectCarriageIdx;
        CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE6;
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
        }
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    }

    else if (MainState == VIDEO_PLAYBACK)
    {
        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();

         if(iVideoFormat == VIDEO_1024_768)
         {

             QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg(".png)}");
             m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

             //SWitchVideoWinStyleSheet();
             CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE6;
             QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
             m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

             ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
             ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
             ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
             ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");

         }
         else
         {
             QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg(".png)}");
             m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

             //SWitchVideoWinStyleSheet();
             CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE6;
             QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
             m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

             ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
             ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
             ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
             ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");
         }

          SubState = PLAYBACK_STOP;
    }
    SetPantoStyleSheet(CurSelectCarriageIdx);

}

void VisitorMonitor::on_pushButton_Car7_clicked()
{
    if(CurSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE7 || MMS_DEVICE_TYPE_CARRIAGE7 >=g_iCarriageNum)
        return ;

    if( MainState == VIDEO_REALMONITOR)
    {
        LastSelectCarriageIdx = CurSelectCarriageIdx;
        CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE7;
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
        }
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    }

    else if (MainState == VIDEO_PLAYBACK)
    {
        if(iVideoFormat == VIDEO_1024_768)
        {

            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

           // SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE7;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");
            SetPantoStyleSheet(CurSelectCarriageIdx);
        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE7;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");
            SetPantoStyleSheet(CurSelectCarriageIdx);
        }
        SubState = PLAYBACK_STOP;
    }
    SetPantoStyleSheet(CurSelectCarriageIdx);

}

void VisitorMonitor::on_pushButton_Car8_clicked()
{
    if(CurSelectCarriageIdx  == MMS_DEVICE_TYPE_CARRIAGE8|| MMS_DEVICE_TYPE_CARRIAGE8 >=g_iCarriageNum)
        return ;

    if( MainState == VIDEO_REALMONITOR)
    {
        LastSelectCarriageIdx = CurSelectCarriageIdx;
        CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE8;
        m_Devices[LastSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(0,-1);
        SWitchVideoWinStyleSheet();

        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(LastSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[LastSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            QString strStyleSheet1 =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);
        }
        m_Devices[CurSelectCarriageIdx]->RealTimeMonitorThread->SetPreviewStatus(1,m_iCurrentPage);
    }

    else if (MainState == VIDEO_PLAYBACK)
    {

        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();
        if(iVideoFormat == VIDEO_1024_768)
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

           // SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE8;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");
        }
        else
        {
            QString strStyleSheet =QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg(".png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet);

            //SWitchVideoWinStyleSheet();
            CurSelectCarriageIdx =  MMS_DEVICE_TYPE_CARRIAGE8;
            QString strStyleSheet1= QString("%1%2%3").arg("QPushButton{border-image: url(:/carImag/carImage_800/").arg(CurSelectCarriageIdx+1).arg("_on.png)}");
            m_CarButton[CurSelectCarriageIdx]->setStyleSheet(strStyleSheet1);

            ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
            ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
            ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
            ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");
        }
        SubState = PLAYBACK_STOP;
    }
    SetPantoStyleSheet(CurSelectCarriageIdx);

}


//开始回放
void VisitorMonitor::on_pushButtonPlayBackStart_clicked()
{   
   if(SubState !=PLAYBACK_STOP)
       return ;

    QTime CurTime =QTime::currentTime();
    QTime m_StartTime =CurTime.addSecs(0-g_iPlaybackSecond);
    QTime m_StopTime =CurTime;
    QDate data =QDate::currentDate();

    QDateTime StartDataTime(data,m_StartTime);
    QDateTime StopDataTime(data,m_StopTime);


    m_Devices[CurSelectCarriageIdx]->PlayBackThread->SetPlayBackTime(StartDataTime,StopDataTime,true,m_iCurrentPage);
    if(iVideoFormat == VIDEO_1024_768)
    {

        ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play.png)}");
        ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop_on.png)}");
        ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back_on.png)}");
        ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward_on.png)}");

    }
    else
    {
        ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play.png)}");
        ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop_on.png)}");
        ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back_on.png)}");
        ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward_on.png)}");

    }
 //   qDebug()<<"play111111111111"<<" "<<m_iCurrentPage<<endl;
    SubState =PLAYBACK_START;
}



//回放暂停
void VisitorMonitor::on_pushButtonPlayBackSuspend_clicked()
{
    if(SubState == PLAYBACK_STOP)
        return ;

    m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();

    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
        ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
        ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
        ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");
    }
    else
    {
        ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
        ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
        ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
        ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");

    }
    SubState = PLAYBACK_STOP;
}

//回放后退
void VisitorMonitor::on_pushButtonPlayBackBack_clicked()
{
    if(SubState == PLAYBACK_STOP)
        return ;

    m_Devices[CurSelectCarriageIdx]->PlayBackThread->SetPlayBackStatus(NET_DVR_PLAYSLOW);

    SubState = PLAYBACK_BACKOFF;

}

//回放快进
void VisitorMonitor::on_pushButtonPlayBackForward_clicked()
{
    if(SubState == PLAYBACK_STOP)
        return ;

    m_Devices[CurSelectCarriageIdx]->PlayBackThread->SetPlayBackStatus(NET_DVR_PLAYFAST);

    SubState = PLAYBACK_FASTFORWARD;

}

//下一页
void VisitorMonitor::on_pushButtonPlayBackNextPage_clicked()
{
    m_iCurrentPage = (++m_iCurrentPage%MAX_CARRIAGE_PAGE_NUM);
    if(iVideoFormat == VIDEO_1024_768)
    {
        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();

        ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Play_on.png)}");
        ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Stop.png)}");
        ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Back.png)}");
        ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control/Forward.png)}");

    }
    else
    {
        m_Devices[CurSelectCarriageIdx]->PlayBackThread->StopPlayBack();

        ui->pushButtonPlayBackStart->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Play_on.png)}");
        ui->pushButtonPlayBackSuspend->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Stop.png)}");
        ui->pushButtonPlayBackBack->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Back.png)}");
        ui->pushButtonPlayBackForward->setStyleSheet("QPushButton{border-image: url(:/ctrl/Control_800/Forward.png)}");

    }
    SubState = PLAYBACK_STOP;
}

