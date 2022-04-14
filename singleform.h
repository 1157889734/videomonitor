#ifndef SINGLEFORM_H
#define SINGLEFORM_H

#include <QWidget>
#include "mainsdk.h"

namespace Ui {
class SingleForm;
}

class SingleForm : public QWidget
{
    Q_OBJECT

public:
    explicit SingleForm(QWidget *parent = 0);
    ~SingleForm();

    int Start(const char *rtsp);
    int Stop();

signals:
    void signalCmd(int nCmd);
protected:

    void mousePressEvent(QMouseEvent *event);
private:
    Ui::SingleForm *ui;

    T_WND_INFO  m_tWndInfo;
    CMPHandle   m_pCMPHandle;
};

#endif // SINGLEFORM_H
