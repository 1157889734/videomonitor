#include "AEyeHttpDownload.h"
#include <QtGui>
#include <QtNetwork>
#include <QMessageBox>


CAEyeHttpDownload::CAEyeHttpDownload()
{
    httpRequestAborted = false;
}

CAEyeHttpDownload::~CAEyeHttpDownload()
{

}

void CAEyeHttpDownload::DownloadFile(QString strUrl,QString strSavePath)
{
    url = strUrl;

    QFileInfo fileInfo(url.path());
    m_strfileName = strSavePath;
    //m_strfileName.append("\\");
    m_strfileName.append(fileInfo.fileName());
    if (m_strfileName.isEmpty())
        m_strfileName = "index.html";

    if (QFile::exists(m_strfileName)) {
     /*   if (QMessageBox::question(this, tr("HTTP"),
            tr("There already exists a file called %1 in "
            "the current directory. Overwrite?").arg(fileName),
            QMessageBox::Yes|QMessageBox::No)
            == QMessageBox::No)
            return;*/
        QFile::remove(m_strfileName);
    }

    file = new QFile(m_strfileName);
    if (!file->open(QIODevice::WriteOnly)) {
      /*  QMessageBox::information(this, tr("HTTP"),
            tr("Unable to save the file %1: %2.")
            .arg(fileName).arg(file->errorString()));*/
        delete file;
        file = 0;
        return;
    }
//
//#ifndef Q_WS_MAEMO_5
//    progressDialog->setWindowTitle(tr("HTTP"));
//    progressDialog->setLabelText(tr("Downloading %1.").arg(fileName));
//#endif
//    downloadButton->setEnabled(false);
//
//    // schedule the request
//    httpRequestAborted = false;
    StartRequest(url);

}

void CAEyeHttpDownload::StartRequest(QUrl url)
{
    reply = qnam.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()),this, SLOT(httpFinished()));
    connect(reply, SIGNAL(readyRead()),this, SLOT(httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),this, SLOT(updateDataReadProgress(qint64,qint64)));
}

void CAEyeHttpDownload::httpFinished()
{
    // 0 ³É¹¦£» 1 Ê§°Ü
    int iFlag = 0;
    if (httpRequestAborted) {
        if (file) {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }
        reply->deleteLater();
        return;
    }

    file->flush();
    file->close();

    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
        iFlag = 1;
        file->remove();
       /* QMessageBox::information(this, tr("HTTP"),
            tr("Download failed: %1.")
            .arg(reply->errorString()));*/
    } else if (!redirectionTarget.isNull()) {        
        QUrl newUrl = url.resolved(redirectionTarget.toUrl());
        if (1/*QMessageBox::question(this, tr("HTTP"),tr("Redirect to %1 ?").arg(newUrl.toString()),QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes*/) {
                url = newUrl;
                reply->deleteLater();
                file->open(QIODevice::WriteOnly);
                file->resize(0);
                StartRequest(url);
                return;
        }
    } else {
        //QString fileName = QFileInfo(QUrl(urlLineEdit->text()).path()).fileName();
        //statusLabel->setText(tr("Downloaded %1 to %2.").arg(fileName).arg(QDir::currentPath()));
        //downloadButton->setEnabled(true);
    }

    reply->deleteLater();
    reply = 0;
    delete file;
    file = 0;

    emit sendFinishData(iFlag,m_strfileName);
}

void CAEyeHttpDownload::httpReadyRead()
{
    if (file)
        file->write(reply->readAll());
}

void CAEyeHttpDownload::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (httpRequestAborted)
        return;
    
    emit sendProcessData(bytesRead,totalBytes);
}