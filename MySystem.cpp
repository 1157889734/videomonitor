#include "MySystem.h"
#include "ui_MySystem.h"
#include<QDebug>


MySystem::MySystem(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MySystem),
    timecount(59),
    iVideoFormat(0)
{
    ui->setupUi(this);

   QDesktopWidget *desktop=QApplication::desktop();
   //QRect clientRect = desktop->availableGeometry();
   //获取设备可用区宽高
   //int width=clientRect.width();
   //int height=clientRect.height();
   int width=(desktop)->width();
   int height=(desktop)->height();


   resize(width,height);

    setWindowFlags(Qt::FramelessWindowHint |Qt::Window);//无边框
    showFullScreen();
   //setWindowFlags(Qt::FramelessWindowHint);//无边框
    //setGeometry(WINDOW_OFFSETX,WINDOW_OFFSETY,SYSTEM_WIN_WIDTH,SYSTEM_WIN_HEIGHT);


    if(desktop->width()>=1024)
    {
       iVideoFormat = VIDEO_1024_768;
    }
    else
    {
        iVideoFormat = VIDEO_800_600;
    }
    //QFont font;
    //font.setPointSize(FONT_SIZE);//字体大小
    //setFont(font);//其他控件一样
    QStringList qss;
    qss.append(QString("%1}").arg(QString::fromLocal8Bit(" font-family:'微软雅黑';")));
    setStyleSheet(qss.join(""));

    ResizeControl(width,height);
    SetControlStyleSheet();

    //setAttribute(Qt::WA_TranslucentBackground);//背景透明
    timer = new QTimer(this);
    timer->stop();
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(ProcessTimerOut()));
    timer->start();

   //qApp->setAttribute(Qt::WA_AcceptTouchEvents);

    UserMonitorWin.setAttribute(Qt::WA_AcceptTouchEvents);
    LoginSystemWin.setAttribute(Qt::WA_AcceptTouchEvents);

    connect(&UserMonitorWin,SIGNAL(ChangetoLoginSignal()),this,SLOT(toLoginWinSlot()));
    connect(&LoginSystemWin,SIGNAL(ChangToUserWin(int)),this,SLOT(toUserWinSlot(int)));
}

MySystem::~MySystem()
{
    delete ui;
}

void MySystem::SetControlStyleSheet()
{
    if(iVideoFormat == VIDEO_1024_768)
    {
        QPixmap pixmapWin = QPixmap(":/imag/image/Load_nologo.png").scaled(this->size());
        QPalette paletteWin(this->palette());
        paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
        this->setPalette(paletteWin);
        return;
    }
    else
    {
        QPixmap pixmapWin = QPixmap(":/imag/image_800/Load_nologo.png").scaled(this->size());
        QPalette paletteWin(this->palette());
        paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
        this->setPalette(paletteWin);
        return;

    }

}

void MySystem::ResizeControl(int width,int height)
{
   if(iVideoFormat == VIDEO_1024_768)
   {
       int CancelButtonX = (width - CANCEL_BUTTON_WIDTH)/2+2;
       int CancelButtonY =height-120-CANCEL_BUTTON_HEIGHT;
       ui->pushButtonCancel->move(CancelButtonX,CancelButtonY);
       ui->pushButtonCancel->resize(CANCEL_BUTTON_WIDTH,CANCEL_BUTTON_HEIGHT);
       ui->pushButtonCancel->setStyleSheet("QPushButton{border-image: url(:/imag/image/button.png);"
                                           "color:white"
                                           "}");

       int TiemLabelX =( width- INIT_TIME_LABEL_WIDTH)/2+7;
       int TimeLabelY =(height-INIT_TIME_LABEL_HEIGHT)/2 +25;
       ui->labelTime->move(TiemLabelX,TimeLabelY);
       ui->labelTime->resize(INIT_TIME_LABEL_WIDTH,INIT_TIME_LABEL_HEIGHT);
       ui->labelTime->setStyleSheet("color:white;"
                                "background-color:rgb(5,23,89);");
       QString strTime =QString::number(timecount)+"s";
       ui->labelTime->setText(strTime);


   }
   else
   {
       int CancelButtonX = (width - CANCEL_BUTTON_WIDTH_600P)/2+3;
       int CancelButtonY =height-94-CANCEL_BUTTON_HEIGHT_600P;
       ui->pushButtonCancel->move(CancelButtonX,CancelButtonY);
       ui->pushButtonCancel->resize(CANCEL_BUTTON_WIDTH_600P,CANCEL_BUTTON_HEIGHT_600P);
       ui->pushButtonCancel->setStyleSheet("QPushButton{border-image: url(:/imag/image_800/button.png);"
                                           "color:white"
                                           "}");

       int TiemLabelX =( width- INIT_TIME_LABEL_WIDTH_600P)/2+7;
       int TimeLabelY =(height-INIT_TIME_LABEL_HEIGHT_600P)/2 +20;
       ui->labelTime->move(TiemLabelX,TimeLabelY);
       ui->labelTime->resize(INIT_TIME_LABEL_WIDTH_600P,INIT_TIME_LABEL_HEIGHT_600P);
       ui->labelTime->setStyleSheet("color:white;"
                                "background-color:rgb(5,23,89);");
       QString strTime =QString::number(timecount)+"s";
       ui->labelTime->setText(strTime);


   }
}


void MySystem::toUserWinSlot(int iLoginType)
{
    LoginSystemWin.hide();
    UserMonitorWin.setLoginType(iLoginType);
    UserMonitorWin.showFullScreen();
}

void MySystem::toLoginWinSlot()
{
    UserMonitorWin.hide();
    LoginSystemWin.showFullScreen();
}

void MySystem::ProcessTimerOut()
{
    timecount -=1;
    if(timecount == 0)
    {
        on_pushButtonCancel_clicked();
    }
    else
    {
        QString strTime =QString::number(timecount)+"s";
        ui->labelTime->setText(strTime);
    }
}

void MySystem::on_pushButtonCancel_clicked()
{
    timer->stop();
    this->hide();
    LoginSystemWin.showFullScreen();
}



void MySystem::getConfigureInfo(QString Dir)
{

}


