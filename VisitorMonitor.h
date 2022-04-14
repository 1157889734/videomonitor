#ifndef VISITORMONITOR_H
#define VISITORMONITOR_H

#include <QWidget>
#include <define.h>
#include<QLabel>
#include<QDesktopWidget>
#include<CarriageDevice.h>
#include<QTimer>
#include<QDateTime>
#include<QPushButton>
#include<QDate>
#include<QMouseEvent>




namespace Ui {
class VisitorMonitor;
}


class VisitorMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit VisitorMonitor(QWidget *parent = nullptr);
    ~VisitorMonitor();
     void CreatDevice();
     void ResizeControl(int Width,int Height);
     void ResizeMainFunctionButton(int Width,int Height);
     void ResizeLabel(int width,int Height);
     void ResizeButton(int width,int Height);

     //设置样式表
     void SetControlStyleSheet();

     //停止视频播放
     void StopPlayVideo();

     // 切换视频窗口样式表
     void SWitchVideoWinStyleSheet();

     void InitControl();

     void SetPantoStyleSheet(int iCarriageIndex);

private slots:
        void on_pushButton_Login_clicked();

        void on_pushButton_VideoMonitor_clicked();

        void on_pushButton_VideoPlayback_clicked();

        void ProcessTimerOut();

        void ProcessPollTimerOut();

        void on_pushButton_prevCar_clicked();

        void on_pushButton_nextCar_clicked();

        void on_pushButton_pollstart_clicked();

        void on_pushButton_pollsuspend_clicked();

        void on_pushButton_Car1_clicked();

        void on_pushButton_Car2_clicked();

        void on_pushButton_Car3_clicked();

        void on_pushButton_Car4_clicked();

        void on_pushButton_Car5_clicked();

        void on_pushButton_Car6_clicked();

        void on_pushButton_Car7_clicked();

        void on_pushButton_Car8_clicked();

        void on_pushButtonPlayBackStart_clicked();

        void on_pushButtonPlayBackSuspend_clicked();

        void on_pushButtonPlayBackBack_clicked();

        void on_pushButtonPlayBackForward_clicked();

        void InitDeviceSlots();

        void toVistorWinSlot();



        void on_pushButtonPlayBackNextPage_clicked();

        void on_pushButton_nextPage_clicked();
        void StopPlayBackSlots(int,int);
        void StopRealMonitorSlots(int,int);

        void SingleVideoWinToVisorWinSlots();

protected:
    void CalVideoArea(int width,int height);
    void mousePressEvent(QMouseEvent *event);

signals :
    void    ChangetoLoginWinSignal();
    void    SucceedInitDeviceSignals(CCarriageDevice **);
    void    ChangetoSingleVideoWinSignals(QVariant);
    void    SingleVideoWinPollSignals(int);
    void    SingleVideoPlaySignals();


private:
    Ui::VisitorMonitor *ui;

    int  MainState;
    int  SubState;
    QTimer *timer;
    int LastSelectCarriageIdx;
    int CurSelectCarriageIdx;
    QTimer  *pollTimer;

    int  m_iCurrentPage;
    int  iVideoFormat;
    bool bSingleVideoWin;

public:
    //所有设备
    CCarriageDevice*    			m_Devices[MMS_DEVICE_TYPE_COUNT];
    HWND                           m_Videos[VIDEO_WINDOWS_COUNT];
    QPushButton*                   m_CarButton[MMS_DEVICE_TYPE_COUNT];
    VideoArea                      m_VideoArea[VIDEO_WINDOWS_COUNT];
    int                            iSingleCameraIdx;
};

#endif // VISITORMONITOR_H
