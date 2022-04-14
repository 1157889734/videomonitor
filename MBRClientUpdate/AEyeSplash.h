#ifndef __AEYESPLASH_H__
#define __AEYESPLASH_H__

#include <QObject>
#include <QColor>

#define AEYESPLASH AEyeSplash::getInstance()

enum {
    AEyeSplash_Starting = 0,
	AEyeSplash_MBRLoding,
	AEyeSplash_ReadCard,
	AEyeSplash_Update,
};

class QTimer;
class QSplashScreen;
class  AEyeSplash : public QObject {
	Q_OBJECT
	
signals:
	
private slots:
	
public:
	static AEyeSplash* getInstance();
	~AEyeSplash();
	
	void setLoadingSurf(int iType);
	void showLoadingMessage(QString strMessage, QColor color = Qt::white, Qt::Alignment alignment = Qt::AlignRight | Qt::AlignTop);
	void endLoading(QWidget*, int iSecond = 0);
	
private:
	AEyeSplash();
	AEyeSplash(const AEyeSplash&);
	AEyeSplash& operator= (const AEyeSplash&);
	
private:
	QSplashScreen* m_pQSplashScreen;
	QString m_strPixmap;
};

#endif // !__AEYESPLASH_H__
