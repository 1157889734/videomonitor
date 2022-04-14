#ifndef __MBRCLIENTUPDATE_H__
#define __MBRCLIENTUPDATE_H__

#include "AEyeBaseWidget.h"
#include <string>

class QUrl;
class QTimer;
class QProgressBar;
class QHBoxLayout;
class CAEyeHttpDownload;
class QLabel;
class MBRClientUpdate : public AEyeBaseWidget {
    Q_OBJECT

signals:

    private slots:
        void _close();
        void _done();
        void _updateProgress(int bytesRead, int totalbytes);
		void _updatUnziped();
        void _updateFinished(int iFlag,QString strNewFile);
		void _test();

public:
    MBRClientUpdate(const QString& strCMD, QWidget* parent = 0);
    ~MBRClientUpdate();

protected:
    virtual void paintEvent(QPaintEvent*);
	virtual void keyPressEvent(QKeyEvent* event);

protected:
    QHBoxLayout* _h0();
    QHBoxLayout* _h1();
    QHBoxLayout* _h2();

private:
	void _setText(const QString&);

private:
    qint64 m_iTotalSize;
    QProgressBar* m_pQProgressBar;
    CAEyeHttpDownload* m_pAEyeHttpDownload;
    QString m_strURL;
    QString m_strUsrDir;
    QUrl* m_pQUrl;
	QPixmap* m_pQPixmap;
	QLabel* m_pQLabelText;


private:
    std::string GetAppPath();
	bool startClient();
    std::string GetUpdatePath();
    std::string GetCfgFile();
    void WriteConfig(std::string strKeyName, std::string strValue);
};

#endif
