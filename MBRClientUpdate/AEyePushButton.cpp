#include "stdafx.h"
#include "AEyePushButton.h"
#include <QPainter>
#include <QMouseEvent>

AEyePushButton::AEyePushButton(QWidget* parent/* = 0*/)
    : QPushButton(parent)
    , m_bLocked(false)
    , m_bEnabled(true)
    , m_ButtonStatus(BTN_STATUS_NORMAL)
    , m_bMousePress(false) {

}

AEyePushButton::~AEyePushButton() {

}

void AEyePushButton::setPicName(QString pic_name) {
    this->m_strPictureName = pic_name;
    setFixedSize(QPixmap(pic_name).size());
}

void AEyePushButton::enterEvent(QEvent *) {
    setCursor(Qt::PointingHandCursor);
    m_ButtonStatus = BTN_STATUS_ENTER;
    update();
}

void AEyePushButton::mousePressEvent(QMouseEvent *event) {
    if(Qt::LeftButton == event->button()) {   
        m_bMousePress = true;
        m_ButtonStatus = BTN_STATUS_PRESS;
        update();
    }
}

void AEyePushButton::mouseReleaseEvent(QMouseEvent *event) {
    if(m_bMousePress  && this->rect().contains(event->pos())) {    
        m_bMousePress = false;
        m_ButtonStatus = BTN_STATUS_NORMAL;
        update();
        emit clicked();
    }
}

void AEyePushButton::leaveEvent(QEvent *) {
    m_ButtonStatus = BTN_STATUS_NORMAL;
    update();
}

void AEyePushButton::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    QPixmap pixmap;
    if(m_bLocked) {
        pixmap.load(m_strPictureName + QString("_disabled"));
    } else if (!m_bEnabled) {
        pixmap.load(m_strPictureName + QString("_disabled"));
    } else {
        switch(m_ButtonStatus) {
        case BTN_STATUS_NORMAL:
            {
                pixmap.load(m_strPictureName);
                break; 
            }   

        case BTN_STATUS_ENTER:
            {
                pixmap.load(m_strPictureName + QString("_hover"));
                break;
            } 

        case BTN_STATUS_PRESS:
            {
                //pixmap.load(m_strPictureName + QString("_pressed"));	
                pixmap.load(m_strPictureName + QString("_hover"));	
                break;
            }    
        }
    }

    painter.drawPixmap(rect(), pixmap);
}

void AEyePushButton::setEnabled(bool bEnable) {
    if(m_bLocked) {
        return;
    }

    m_bEnabled = bEnable;
    QPushButton::setEnabled(bEnable);
}

void AEyePushButton::setLocked(bool bLocked) {
    m_bLocked = bLocked;
    QPushButton::setEnabled(!bLocked);
}
