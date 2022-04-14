#include "stdafx.h"
#include "MBRClientUpdate.h"
#include "AEyeHttpDownload.h"
#include "AEyePushButton.h"
#include <QUrl>
#include <QTimer>
#include <QLabel>
#include <QLayout>
#include <QProgressBar>
#include <AEyeTaskExtract.h>
#include <QApplication>
#define CFG_FILE		 "config.ini"
#define CFG_APPNAME  "ClientUpdate"
#define CFG_NEWFILE	 "file"

MBRClientUpdate::MBRClientUpdate(const QString& strCMD, QWidget *parent/* = 0*/)
    : AEyeBaseWidget(parent)
    , m_strURL(strCMD) {
		m_pAEyeHttpDownload = NULL;
		QString strDir = qApp->applicationDirPath();
		m_pQPixmap = new QPixmap("./Resources/update/bg");
		
		setFixedSize(m_pQPixmap->size());

        if(m_strURL.isEmpty()) {
            //m_strURL = // 获取更新路径
        }
        QVBoxLayout* pQVBoxLayoutMain = new QVBoxLayout;

		pQVBoxLayoutMain->addLayout(_h0());
        pQVBoxLayoutMain->addSpacing(55);
        pQVBoxLayoutMain->addLayout(_h1());
		pQVBoxLayoutMain->addStretch();
        pQVBoxLayoutMain->addLayout(_h2());
		pQVBoxLayoutMain->addSpacing(30);

        pQVBoxLayoutMain->setSpacing(0);
        pQVBoxLayoutMain->setContentsMargins(0, 0, 0, 0);
        setLayout(pQVBoxLayoutMain);

		_setText("update...");
        m_pQProgressBar->setRange(0, 100);
        m_pQProgressBar->setValue(0);
        
		/*解压*/
		QFile _file;
		QString strFile = "VideoMonitorPack.zip";		
		
		QDir dir;
		if(!dir.exists(strDir)) {
			dir.mkpath(strDir);
		}
		QString strFilePath = QString("%1/%2").arg(strDir).arg(strFile);
// 		_file.setFileName(strFilePath);
// 		if (!_file.open(QIODevice::WriteOnly)) {
// 			_file.close();
// 			return ;
// 		}
// 		_file.close();
// 		
		AEyeTaskExtract* pAEyeTaskExtract = new AEyeTaskExtract(strFilePath, strDir);
		connect(pAEyeTaskExtract, SIGNAL(downloaded(QString, QString)), this, SIGNAL(downloaded(QString, QString)), Qt::DirectConnection);
		connect(pAEyeTaskExtract, SIGNAL(finished()), pAEyeTaskExtract, SLOT(deleteLater()));
		connect(pAEyeTaskExtract,SIGNAL(sigUnzipping(int ,int)),this,SLOT(_updateProgress(int,int)));
		connect(pAEyeTaskExtract,SIGNAL(sigUnziped()),this,SLOT(_updatUnziped()));
		pAEyeTaskExtract->start();
}

MBRClientUpdate::~MBRClientUpdate() {
    //_stop();
    if(NULL != m_pAEyeHttpDownload)
    {
        delete m_pAEyeHttpDownload;
        m_pAEyeHttpDownload = NULL;
    }
}

QHBoxLayout* MBRClientUpdate::_h0() {
    AEyePushButton* pAEyePushButtonClose = new AEyePushButton;
    pAEyePushButtonClose->setPicName("./Resources/update/close");
    connect(pAEyePushButtonClose, SIGNAL(clicked()), this, SLOT(_close()));

    QHBoxLayout* pQHBoxLayout = new QHBoxLayout;
	pQHBoxLayout->addStretch();
    pQHBoxLayout->addWidget(pAEyePushButtonClose);

    pQHBoxLayout->setSpacing(0);
    pQHBoxLayout->setContentsMargins(0, 0, 0, 0);
    return pQHBoxLayout;
}

QHBoxLayout* MBRClientUpdate::_h1() {
    m_pQLabelText = new QLabel;
    QFont aQFont;
    aQFont.setPixelSize(24);
    m_pQLabelText->setFont(aQFont);
	QPalette nQPalette;
	nQPalette.setColor(QPalette::WindowText, Qt::white);
	m_pQLabelText->setPalette(nQPalette);

    QHBoxLayout* pQHBoxLayout = new QHBoxLayout;
	pQHBoxLayout->addStretch();
    pQHBoxLayout->addWidget(m_pQLabelText);
    pQHBoxLayout->addStretch();

    pQHBoxLayout->setSpacing(0);
    pQHBoxLayout->setContentsMargins(0, 0, 0, 0);
    return pQHBoxLayout;
}

void MBRClientUpdate::_setText(const QString& strText) {
	m_pQLabelText->setText(strText);
}

void MBRClientUpdate::_test() {
	int iValue = m_pQProgressBar->value();
	if(iValue == m_pQProgressBar->maximum()) {
		_setText("update...");
		return;
	}

	m_pQProgressBar->setValue(iValue + 1);
}

QHBoxLayout* MBRClientUpdate::_h2() {
    m_pQProgressBar = new QProgressBar;
	QString strStyle = "QProgressBar {border-radius: -1px; background-image:url(./Resources/update/chunkbg); text-align: center; }" 
		"QProgressBar::chunk {border-radius:-1px;border:1px solid black;background-image:url(./Resources/update/chunk); }";
	m_pQProgressBar->setStyleSheet(strStyle);
	//m_pQProgressBar->setTextVisible(false);

    QHBoxLayout* pQHBoxLayout = new QHBoxLayout;
    pQHBoxLayout->addWidget(m_pQProgressBar);

    pQHBoxLayout->setContentsMargins(31, 0, 31, 0);
    return pQHBoxLayout;
}

void MBRClientUpdate::_close() {
	//_stop();
    close();
}

void MBRClientUpdate::paintEvent(QPaintEvent* pEvent) {
    AEyeBaseWidget::paintEvent(pEvent);
    QPainter painter(this);
    painter.drawPixmap(rect(), *m_pQPixmap);
}

void MBRClientUpdate::keyPressEvent(QKeyEvent* event) {
	switch(event->key()) {
	case Qt::Key_Escape:
		return;
		break;

	default:
		AEyeBaseWidget::keyPressEvent(event);
		break;
	}
}

//void MBRClientUpdate::_stop() {
//    m_pAEyeHttpDownload->abort();
//}

//void MBRClientUpdate::_download() {
//    m_pQTimerGetFileSize->stop();
//    bool ok;
//    QString strUrl = m_strURL;
//    //totalSize = ui.lineEdit_2->text().toLongLong(&ok, 10);
//    m_pQUrl = new QUrl(strUrl);
//    m_pAEyeHttpDownload->getFileSize(*m_pQUrl);
//}

void MBRClientUpdate::_done() {
    QString str = "download complete";
    //QMessageBox::warning(this, "Message", str);
}

void MBRClientUpdate::_updateProgress(int bytesRead, int totalbytes) {
	m_pQProgressBar->setMaximum(totalbytes);
	m_pQProgressBar->setValue(bytesRead);
}
void MBRClientUpdate::_updatUnziped()
{
	startClient();
	QString strDir = qApp->applicationDirPath();
	QString strFile = "UpgradeCfg.ini";		
	QString strFilePath = QString("%1/%2").arg(strDir).arg(strFile);
	QFile fileTempCfg(strFilePath);
	fileTempCfg.remove();
	strFile = "updateflag.ini";		
	strFilePath = QString("%1/%2").arg(strDir).arg(strFile);
	QFile fileTempFlag(strFilePath);
	fileTempFlag.remove();
	strFilePath = QString("%1/%2").arg(strDir).arg(strFile);
	strFile = "VideoMonitorPack.zip";
	strFilePath = QString("%1/%2").arg(strDir).arg(strFile);
	QFile fileTempZip(strFilePath);
	fileTempZip.remove();
	exit(0);
	
}
void MBRClientUpdate::_updateFinished(int iFlag,QString strNewFile)
{
    if(0 == iFlag)
    {
        WriteConfig(CFG_NEWFILE,strNewFile.toStdString());
    }
    else
    {
    }

    close();
	startClient();
}

bool MBRClientUpdate::startClient() {
	char szPath[MAX_PATH] = {0};
	::GetModuleFileNameA(NULL, szPath, MAX_PATH);

	std::string strFull = szPath;
	std::string strPath = strFull.substr(0, strFull.rfind("\\") + 1);
	std::string strUpdateFile = strPath + "VideoMonitor.exe";
	if(ShellExecuteA(NULL, "open", strUpdateFile.c_str(), NULL, NULL, SW_SHOW) >= (HANDLE) 32) {
		return true;
	}

	return false;
}
std::string MBRClientUpdate::GetAppPath()
{   
    std::string strFilePath;
    char cFilePath[MAX_PATH] = {0};
    GetModuleFileNameA(NULL,cFilePath,MAX_PATH);
    strFilePath = cFilePath;

    int iPosIndex;
    iPosIndex = strFilePath.rfind('\\');
    strFilePath = strFilePath.substr(0,iPosIndex+1);
    return strFilePath;
}

std::string MBRClientUpdate::GetUpdatePath()
{   
    std::string UpdatePath;
    UpdatePath = GetAppPath()+"Update\\";
    return UpdatePath;
}

std::string MBRClientUpdate::GetCfgFile()
{
    std::string strCfgFile = GetUpdatePath()+ CFG_FILE;
    return strCfgFile;
}

void MBRClientUpdate::WriteConfig(std::string strKeyName, std::string strValue)
{
    std::string strCfgFile = GetCfgFile();
    WritePrivateProfileStringA(CFG_APPNAME,strKeyName.c_str(),strValue.c_str(),strCfgFile.c_str());
}