#pragma once
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>

class QFile;

class CAEyeHttpDownload : public QObject
{
    Q_OBJECT
public:
    CAEyeHttpDownload();
    ~CAEyeHttpDownload();

public:
    void DownloadFile(QString strUrl,QString strSavePath);
    void StartRequest(QUrl url);

private slots:
    void httpFinished();
    void httpReadyRead();
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);

private:
    QUrl url;
    QNetworkAccessManager qnam;
    QNetworkReply *reply;
    QFile *file;
    int httpGetId;
    bool httpRequestAborted;
    QString m_strfileName;

signals: 
    void sendProcessData(int,int);
    void sendFinishData(int,QString);
};