#ifndef SINGLEVIDEO_H
#define SINGLEVIDEO_H

#include <QWidget>
#include "GlobleDefine.h"
#include<QDesktopWidget>
#include<define.h>
#include<QMouseEvent>

namespace Ui {
class CSingleVideo;
}


class CSingleVideo : public QWidget
{
    Q_OBJECT

public:
    explicit CSingleVideo(QWidget *parent = nullptr);
    ~CSingleVideo();
    void SetControlStyleSheet();
    void ResizeControl(int Width,int Height);

protected:
        void mousePressEvent(QMouseEvent *event);

private slots:
  void    ChangetoSingleVideoWinSlots(QVariant);


  void SingleVideoPlaySlots();

signals:
  void SingleVideoWintoVistorWinSignals();
  void SingleVideoWintoUserWinSignals();

private:
    Ui::CSingleVideo *ui;
    int  iVideoFormat;
    SingleVideoParams VideoParmas;
    VideoArea                      m_VideoArea;

public:
    HWND                           m_Videos;
};

#endif // SINGLEVIDEO_H
