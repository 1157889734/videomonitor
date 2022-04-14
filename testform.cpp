#include "testform.h"
#include "ui_testform.h"

#include <QDebug>
#include <QDesktopWidget>
#include "define.h"

#include "CMPlayerInterface.h"
//#include "me/glyuvwidget.h"

enum  VIDEO_FORMAT
{
    VIDEO_1024_768,
    VIDEO_800_600,

};

TestForm::TestForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestForm)
{
    ui->setupUi(this);

    int iRet = -1;

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
    //setWindowFlags(Qt::FramelessWindowHint |Qt::Window);//无边框
   // showFullScreen();
   //  this->setCursor(Qt::BlankCursor);

    //setGeometry(WINDOW_OFFSETX,WINDOW_OFFSETY,SYSTEM_WIN_WIDTH,SYSTEM_WIN_HEIGHT);

    //QFont font;
    //font.setPointSize(MONITOR_FONT_SIZE);//字体大小
    //setFont(font);//其他控件一样
    QStringList qss;
    qss.append(QString("%1}").arg(QString::fromLocal8Bit(" font-family:'微软雅黑';")));
    setStyleSheet(qss.join(""));


    InitControl();
    SetControlStyleSheet();
    ResizeControl(width,height);

    /*
    T_WND_INFO tWndInfo;
    tWndInfo.nX = 0;
    tWndInfo.nY = 0;
    tWndInfo.nWidth = 1920;
    tWndInfo.nHeight = 1080;
    tWndInfo.pPlayFunc = NULL;
    tWndInfo.pPlayParam = NULL;
    CMPHandle ptCmpPlayers[4] = {NULL, NULL, NULL, NULL};
    QWidget *pWs[4] = {ui->widget_MonitorWin1,ui->widget_MonitorWin2, ui->widget_MonitorWin3, ui->widget_MonitorWin4};
    QWidget*  pWidget = NULL;
    for(int i = 0; i < 4; i++)
    {
        pWidget = pWs[i];
        tWndInfo.hWnd = (HWND)pWidget->winId();
        ptCmpPlayers[i] = CMP_Init(&tWndInfo,CMP_VDEC_NORMAL);
        iRet = CMP_OpenMediaPreview(ptCmpPlayers[i], "rtsp://admin:cx123456@192.168.1.64:554", 0);
        if(iRet  == -1)
        {
            qDebug()<< i << " open  error";
            //return;
        }
        break;
    }
*/
}

TestForm::~TestForm()
{
    delete ui;
}
/*********************************样式参数设置************************/
//设置控件样式表
void TestForm::SetControlStyleSheet()
{

    if(iVideoFormat == VIDEO_1024_768)
    {
        QPixmap pixmapWin = QPixmap(":/imag/image/Monitor_new.png").scaled(this->size());
        QPalette paletteWin(this->palette());
        paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
        this->setPalette(paletteWin);

        ui->widget_CarChose->setStyleSheet("border:2px solid rgb(11,78,193);");


        //monitor window
        ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中

        //label
        ui->labelCarChose->setStyleSheet("border:0px solid rgb(5,23,89);");

        ui->pushButton_Car1->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/1_on.png)}");
        ui->pushButton_Car2->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/2.png)}");
        ui->pushButton_Car3->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/3.png)}");
        ui->pushButton_Car4->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/4.png)}");
        ui->pushButton_Car5->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/5.png)}");
        ui->pushButton_Car6->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/6.png)}");
        ui->pushButton_Car7->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/7.png)}");
        ui->pushButton_Car8->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage/8.png)}");

    }
    else
    {
        QPixmap pixmapWin = QPixmap(":/imag/image_800/Monitor_new.png").scaled(this->size());
        QPalette paletteWin(this->palette());
        paletteWin.setBrush(QPalette::Background, QBrush(pixmapWin));
        this->setPalette(paletteWin);



        ui->widget_CarChose->setStyleSheet("border:2px solid rgb(11,78,193);");


        //monitor window
        ui->widget_MonitorWin1->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin2->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin3->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中
        ui->widget_MonitorWin4->setStyleSheet("QLabel{border-image: url(:/imag/image_800/video.png);border:3px solid rgb(11,78,193);}");//图片在资源文件中

        //label
        ui->labelCarChose->setStyleSheet("border:0px solid rgb(5,23,89);");

        ui->pushButton_Car1->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/1_on.png)}");
        ui->pushButton_Car2->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/2.png)}");
        ui->pushButton_Car3->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/3.png)}");
        ui->pushButton_Car4->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/4.png)}");
        ui->pushButton_Car5->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/5.png)}");
        ui->pushButton_Car6->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/6.png)}");
        ui->pushButton_Car7->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/7.png)}");
        ui->pushButton_Car8->setStyleSheet("QPushButton{border-image: url(:/carImag/carImage_800/8.png)}");


    }

}


void TestForm::ResizeControl(int Width,int Height)
{
    if(iVideoFormat == VIDEO_1024_768)
    {
        //statckwidget
        int LengthToTopMargin = Height *42/768 + MAIN_FUNCTION_BUTTON_HEIGHT+MAIN_FUNCTION_BUTTON_GRAPS*2;

        int stackWidth =Width -STATCKWIDGET_LEFT_MARGIN *2;
        int stackHeight =Height - LengthToTopMargin - STATCKWIDGET_BOTTOM_MARGIN;

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



    }
    else
    {
        //statckwidget
        int LengthToTopMargin = Height *39/600 + MAIN_FUNCTION_BUTTON_HEIGHT_600P+MAIN_FUNCTION_BUTTON_GRAPS_600P*2;

        int stackWidth =Width -STATCKWIDGET_LEFT_MARGIN_600P*2;
        int stackHeight =Height - LengthToTopMargin - STATCKWIDGET_BOTTOM_MARGIN_600P;

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

    }

}

 void TestForm::InitControl()
 {
     ui->labelCarChose->setText("<font color=white>车厢选择</b></font>");
 }
