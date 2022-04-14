#include "stdafx.h"
#include "AEyeSplash.h"
#include <QSplashScreen>
#include <QDateTime>
#include <QTimer>

AEyeSplash* AEyeSplash::getInstance() {
	static AEyeSplash instance;
	return &instance;
}

AEyeSplash::AEyeSplash() {
	m_pQSplashScreen = new(std::nothrow) QSplashScreen;
}

AEyeSplash::~AEyeSplash() {
	//delete m_pQSplashScreen;
	//m_pQSplashScreen = NULL;
}

void AEyeSplash::setLoadingSurf(int iType) {
	switch(iType) { 
    case AEyeSplash_Starting:
        m_strPixmap = "./Resources/img/loading/L1";
        break;

	case AEyeSplash_MBRLoding:
		m_strPixmap = "./Resources/img/loading/L2";
		break;
		
	case AEyeSplash_ReadCard:
		m_strPixmap = "AEyeSplash_ReadCard.gif";
		break;

	case AEyeSplash_Update:
		m_strPixmap = "./Resources/img/loading/loadding_update.gif";
		break;
		
	default:
		m_strPixmap = "AEyeSplash_default.gif";
		break;
	}

    m_pQSplashScreen->setPixmap(QPixmap::fromImage(QImage(m_strPixmap)));
    m_pQSplashScreen->show();
}

void AEyeSplash::showLoadingMessage(QString strMessage, QColor color/*  = Qt::white */, Qt::Alignment alignment/*  = Qt::AlignRight | Qt::AlignTop */) {
	m_pQSplashScreen->showMessage(strMessage, alignment, color); 
}

void AEyeSplash::endLoading(QWidget* pQWidget, int iSecond/* = 0*/) {
	QDateTime nTimeBegin = QDateTime::currentDateTime();
	QDateTime nTimeNow;
	do {
		nTimeNow = QDateTime::currentDateTime();
		m_pQSplashScreen->repaint();
	} while(nTimeBegin.secsTo(nTimeNow) <= iSecond);
	
	m_pQSplashScreen->finish(pQWidget);
}
