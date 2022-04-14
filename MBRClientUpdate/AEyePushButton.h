#ifndef __AEYEPUSHBUTTON_H__
#define __AEYEPUSHBUTTON_H__

#include <QPushButton>

class  AEyePushButton : public QPushButton {
	Q_OBJECT

public:
	explicit AEyePushButton(QWidget* parent = 0);
	~AEyePushButton();

	void setPicName(QString pic_name);
    void setLocked(bool bLocked);
    virtual void setEnabled(bool bEnable);

protected:
	virtual void enterEvent(QEvent *);
	virtual void leaveEvent(QEvent *);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void paintEvent(QPaintEvent *);

private:
    enum ButtonStatus {
        BTN_STATUS_NORMAL, 
        BTN_STATUS_ENTER,
        BTN_STATUS_PRESS, 
    };                      

    bool m_bMousePress;
    bool m_bEnabled;
    bool m_bLocked;
	ButtonStatus m_ButtonStatus;
	QString m_strPictureName;
    HCURSOR m_HCURSORPre;
};

#endif      // !__AEYEPUSHBUTTON_H__
