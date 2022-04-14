#ifndef MYSYSTEM_H
#define MYSYSTEM_H

#include <QDialog>
#include "define.h"
#include<QPushButton>
#include<QColor>
#include<QTimer>
#include<QLabel>

#include<LoginSystem.h>
#include<UserMonitor.h>
#include <QDesktopWidget>
#include "GlobleDefine.h"
#include<QSettings>
#include<string.h>
#include<iostream>
#include<SingleVideo.h>

namespace Ui {
class MySystem;
}

using namespace  std;

class MySystem : public QDialog
{
    Q_OBJECT

public:
    explicit MySystem(QWidget *parent = nullptr);
    ~MySystem();

    void getConfigureInfo(QString Dir);
    void SetControlStyleSheet();
    void ResizeControl(int width,int height);



public slots:

private:
    Ui::MySystem *ui;

public :
    UserMonitor   UserMonitorWin;
    LoginSystem   LoginSystemWin;

    QTimer *timer;
    int timecount;

    //连接设备的线程
    HANDLE								m_hConnectingDeviceThread;
    int                                 iVideoFormat;

signals:
    void InitDeviceSignals();
    void toVistorWinSignals();


private slots:
    void ProcessTimerOut();
    void toUserWinSlot(int iLoginType);
    void toLoginWinSlot();

    void on_pushButtonCancel_clicked();
};

#endif // MYSYSTEM_H
