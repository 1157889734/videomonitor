/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef FTPWINDOW_H
#define FTPWINDOW_H

#include <QObject>
#include <QFile>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>

#include "qftp.h"

class FtpWindow : public QObject
{
    Q_OBJECT

public:
    FtpWindow(QObject *parent = 0);
    ~FtpWindow();

    int connectToFtp(const QString &host,const QString &path, const QString &user,const QString &password,int iNvrNo);

    void disConnectFtp();

    void setLocalPath(const QString &path);

    void downLoadPath(const QString &path);

    int downloadFile(const QString &sSrvFile, const QString &sLocalFile);
	int AddloadFileList(const QString &sSrvFile, const QString &sLocalFile);
	QString getFtpInfoInFTPWindow();
	int getNvrNo(){return m_iNVRNo;}
	void cancelDownloadAbourtNvrDisconnect();
signals:
    void signalDownLoadStep(int nStep);
	 void signalDownLoadError(int nStep);
	  void ShowWarnSignalsSDK( QString title,  QString content);
//![0]
private slots:
    void connectOrDisconnect();
    void cancelDownload();

    void ftpCommandFinished(int commandId, bool error);
    void addToList(const QUrlInfo &urlInfo);
    void updateDataTransferProgress(qint64 readBytes,qint64 totalBytes);
	void ftpPutStateChanged(int iState);
	void ftpDone(bool bDone);
private:

//![1]
    QHash<QString, bool> m_isDirectory;
    QStringList          m_aFiles;
    QFtp                *m_ftp;
    QFile                m_file;
    int                  m_iState;
    quint64              m_ulFileSize;
    quint64              m_ulFileTotalSize;
    QString              m_sLocalPath;
    QString              m_sSrvPath;
    QString              m_sSrvUrl;
    bool                 m_bAutoDown;
//![1]
	int                  m_iNVRNo;
};

#endif
