#ifndef __AEYEBASEWIDGET_H__
#define __AEYEBASEWIDGET_H__

#include <QDialog>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>

class  AEyeBaseWidget : public QDialog {
    Q_OBJECT

public:
    explicit AEyeBaseWidget(QWidget* parent = 0);
    ~AEyeBaseWidget();
    virtual void showWidget();
    virtual void showExec();

protected:
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

private:
    QPoint move_point;

public:
    bool mouse_press;
};

#endif // !__AEYEBASEWIDGET_H__
