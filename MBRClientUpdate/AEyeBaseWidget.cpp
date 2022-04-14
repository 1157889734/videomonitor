#include "stdafx.h"
#include "AEyeBaseWidget.h"
#include "AEyeSplash.h"
#include <QDesktopWidget>
#include <QApplication>

AEyeBaseWidget::AEyeBaseWidget(QWidget* parent/* = 0*/)
    : QDialog(parent) {
        setWindowFlags(/*Qt::WindowStaysOnTopHint |*/ Qt::FramelessWindowHint |  Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::Dialog);
        mouse_press = false;
}

AEyeBaseWidget::~AEyeBaseWidget() {

}

void AEyeBaseWidget::mousePressEvent(QMouseEvent* pQMouseEvent) {
    switch(pQMouseEvent->button()) {
    case Qt::LeftButton:
        mouse_press = true;
        break;

    default:
        break;
    }

    move_point = pQMouseEvent->globalPos() - pos();
}

void AEyeBaseWidget::mouseReleaseEvent(QMouseEvent* pQMouseEvent) {
    mouse_press = false;
}

void AEyeBaseWidget::mouseMoveEvent(QMouseEvent* pQMouseEvent) {
    if(mouse_press) {
        QPoint move_pos = pQMouseEvent->globalPos();
        move(move_pos - move_point);
    }
}

void AEyeBaseWidget::showWidget() {
    AEYESPLASH->endLoading(this);
    setWindowTitle("update exe");

    showNormal();

    raise();
    activateWindow();
}

void AEyeBaseWidget::showExec() {
    AEYESPLASH->endLoading(this);
    setWindowTitle("update exe");
    setModal(true);

    raise();
    activateWindow(); 

    exec();
}
