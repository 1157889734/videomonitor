#include "SingleVideo.h"
#include "ui_SingleVideo.h"

CSingleVideo::CSingleVideo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CSingleVideo)
{
    ui->setupUi(this);
    QDesktopWidget *desktop=QApplication::desktop();
    //QRect clientRect = desktop->availableGeometry();
    //获取设备可用区宽高
    //int width=clientRect.width();
    //int height=clientRect.height();
	int width=(desktop)->width();
	int height=(desktop)->height();

    if(desktop->width()>=1024)
    {
       iVideoFormat = VIDEO_1024_768;
    }
    else
    {
        iVideoFormat = VIDEO_800_600;
    }

    resize(width,height);
    setWindowFlags(Qt::FramelessWindowHint);
    setWindowFlags(Qt::FramelessWindowHint |Qt::Window);//无边框
    //showFullScreen();

    ResizeControl(width,height);
    SetControlStyleSheet();

    m_VideoArea.X=0;
    m_VideoArea.Y=0;
    m_VideoArea.Width = desktop->width();
    m_VideoArea.Height = desktop->height();

    m_Videos=(HWND)ui->widget_MonitorWin->winId();

}

CSingleVideo::~CSingleVideo()
{
    delete ui;
}

void CSingleVideo::ResizeControl(int Width,int Height)
{

      int WidgetWidth = Width-GRAPS_BETWEEN_CONTROLS*2;
      int WidgetHeight = Height-GRAPS_BETWEEN_CONTROLS*2;
      ui->widget_MonitorWin->move(GRAPS_BETWEEN_CONTROLS,GRAPS_BETWEEN_CONTROLS);
      ui->widget_MonitorWin->resize(WidgetWidth,WidgetHeight);
}

void CSingleVideo::SetControlStyleSheet()
{
    if(iVideoFormat == VIDEO_1024_768)
    {
        ui->widget_MonitorWin->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中
    }
    else
    {
        ui->widget_MonitorWin->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中
    }
}

//鼠标事件 单视频切到主视频
void CSingleVideo::mousePressEvent(QMouseEvent *event)
{
    int x = event->x();
    int y = event->y();
    if(x>m_VideoArea.X && x<  m_VideoArea.X+  m_VideoArea.Width && y> m_VideoArea.Y &&
            y<  m_VideoArea.Y+  m_VideoArea.Height)
    {

        if(VideoParmas.iMainStatus == VIDEO_REALMONITOR)
        {

        }
        else if(VideoParmas.iMainStatus == VIDEO_PLAYBACK ||  VideoParmas.iMainStatus == VIDEO_DOWNLOAD )
        {

        }

        if(VideoParmas.iPower == POWER_VISTOR)
        {
            emit SingleVideoWintoVistorWinSignals();
        }
        else if(VideoParmas.iPower == POWER_USER)
        {
            emit SingleVideoWintoUserWinSignals();

        }
        this->hide();
    }

}

void CSingleVideo::SingleVideoPlaySlots()
{
    if(VideoParmas.iMainStatus == VIDEO_REALMONITOR)
    {

    }
    else if(VideoParmas.iMainStatus == VIDEO_PLAYBACK)
    {
        if(VideoParmas.iPlayStatus != PLAYBACK_STOP)
        {

        }
    }
    else if(VideoParmas.iMainStatus == VIDEO_DOWNLOAD)
    {
        if(VideoParmas.iPlayStatus != PLAYBACK_STOP)
        {

        }
    }
}

//主视频切到单视频槽函数
void CSingleVideo::ChangetoSingleVideoWinSlots(QVariant variant)
{
    this->showFullScreen();
    VideoParmas =variant.value<SingleVideoParams>();
    if(VideoParmas.iMainStatus == VIDEO_REALMONITOR)
    {

    }
    else if(VideoParmas.iMainStatus == VIDEO_PLAYBACK)
    {

    }
    else if(VideoParmas.iMainStatus == VIDEO_DOWNLOAD && !VideoParmas.PlayBackFile.isEmpty())
    {

    }
}
