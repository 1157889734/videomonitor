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

#include <QtNetwork>
#include <QDebug>

#include "ftpwindow.h"

FtpWindow::FtpWindow(QObject *parent)
    : QObject(parent), m_ftp(NULL), m_bAutoDown(false)
{
    m_iState = 0;
	m_iNVRNo   = 0;
    disConnectFtp();

    m_ftp = new QFtp(this);
    connect(m_ftp, SIGNAL(commandFinished(int,bool)),
            this, SLOT(ftpCommandFinished(int,bool)));
    connect(m_ftp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(addToList(QUrlInfo)));
    connect(m_ftp, SIGNAL(dataTransferProgress(qint64,qint64)),
            this, SLOT(updateDataTransferProgress(qint64,qint64)));
	connect(m_ftp, SIGNAL(stateChanged(int)), this, SLOT(ftpPutStateChanged(int)));

	connect(m_ftp,SIGNAL(done(bool)),this,SLOT(ftpDone(bool)));

}
void FtpWindow::ftpDone(bool bDone)
{
	if(bDone)
	{
		qDebug()<<"ftp done";
	}
}
void FtpWindow::ftpPutStateChanged(int iState)
{
	switch(iState){
	case QFtp::Unconnected:
		//emit signalDownLoadStep(0);
		//emit signalDownLoadError(m_iState);
		disConnectFtp();
		break;
	default:
		break;
	}
}
FtpWindow::~FtpWindow()
{
    disConnectFtp();
    if(m_ftp)
    {
        m_ftp->close();
        delete m_ftp;
    }
}


void FtpWindow::setLocalPath(const QString &path)
{
    m_sLocalPath = path;
    m_sLocalPath += "/";
}

//![0]
void FtpWindow::connectOrDisconnect()
{

}

int FtpWindow::connectToFtp(const QString &host,const QString &path, const QString &user,const QString &password,int iNvrNo)
{
	m_iNVRNo = iNvrNo;
    disConnectFtp();
    if(m_sSrvUrl == host)
    {
        return 0;
    }

//![2]
    QUrl url;
    url.setUserName(user);
    url.setPassword(password);
    url.setHost(host);
    url.setPath(path);

    m_sSrvUrl = host;
    m_sSrvPath = path;

    int nRet = 0;
    nRet = m_ftp->connectToHost(url.host(), url.port(21));
    if (!url.userName().isEmpty())
    {
        nRet = m_ftp->login(QUrl::fromPercentEncoding(url.userName().toLatin1()), url.password());
    }
    else
    {
        nRet = m_ftp->login();
    }
    if (!url.path().isEmpty())
    {
        nRet = m_ftp->cd(url.path());
    }
//![2]

    return nRet;
}

void FtpWindow::disConnectFtp()
{
    m_file.close();
    m_aFiles.clear();
    m_isDirectory.clear();
    m_ulFileSize = 0;
    m_ulFileTotalSize = 0;
}

void FtpWindow::downLoadPath(const QString &path)
{
    m_aFiles.clear();
    if(m_sSrvPath == path)
    {
        m_ftp->list();
        return;
    }
    m_sSrvPath = path;
    m_ftp->cd(path);
}

//![3]
int FtpWindow::downloadFile(const QString &sSrvFile, const QString &sLocalFile)
{
    QString fileName = sLocalFile;
    m_file.setFileName(fileName);
    if (!m_file.open(QIODevice::WriteOnly))
    {
        return -1;
    }
	m_ftp->setTransferMode(QFtp::Passive);
	return m_ftp->get(sSrvFile, &m_file);
}
//![4]
int FtpWindow::AddloadFileList(const QString &sSrvFile, const QString &sLocalFile)
{
	if (m_file.isOpen())
	{
		QUrlInfo allFileInfo;
		allFileInfo.setDir(false);
		allFileInfo.setName(sSrvFile);
		
		addToList(allFileInfo);

		return 1;
	}
	else
	{
		return downloadFile(sSrvFile,sLocalFile);
	}
	
}
//![5]
void FtpWindow::cancelDownload()
{
    m_ftp->abort();
    if (m_file.exists())
    {
        m_file.close();
        m_file.remove();
    }
}
void FtpWindow::cancelDownloadAbourtNvrDisconnect()
{
	m_ftp->abort();
	if (m_file.exists())
	{
		m_file.close();
		m_file.remove();
	}
}
//![5]

void FtpWindow::ftpCommandFinished(int, bool error)
{
	if (error)
	{
		emit signalDownLoadStep(0);
		emit signalDownLoadError(m_iState);
	}
    QFtp::Command cmd = m_ftp->currentCommand();

    //qDebug() << "cmd:" << cmd;
    if (cmd == QFtp::ConnectToHost)
    {
        if (error)
        {
            return;
        }
        return;
    }

    if(cmd == QFtp::Cd)
    {
        m_ftp->list();
    }

//![7]
    if (cmd == QFtp::Login)
    {

    }
//![7]

//![8]
    if (cmd == QFtp::Get)
    {
        if (error)
        {
            m_file.close();
            m_file.remove();
        }
        else
        {
            m_file.close();
            if(m_aFiles.size())
            {
                const QString &file = m_aFiles.at(0);
				int nPos = file.lastIndexOf("/");;
				int nPosEnd = file.length();
				QString strFileName =file.mid(nPos+1,file.length());
				m_ulFileSize = m_ulFileTotalSize = 0;
                downloadFile(file, m_sLocalPath+strFileName);
				
                m_aFiles.removeAt(0);
            }
            else
            {
               
                m_ulFileSize = m_ulFileTotalSize;
                if(m_ulFileTotalSize > 0)
                {
                    emit signalDownLoadStep((int)(100*m_ulFileSize/m_ulFileTotalSize));
                }
                else
                {
                   emit signalDownLoadStep(0);
                }

            }
        }
    }
//![8]
//![9]
    else if (cmd == QFtp::List)
    {
        if(m_aFiles.size())
        {
            const QString &file = m_aFiles.at(0);
            downloadFile(file, m_sLocalPath+file);
            m_aFiles.removeAt(0);
        }
    }
//![9]
}
//![10]
void FtpWindow::addToList(const QUrlInfo &urlInfo)
{
    QString sName = urlInfo.name();
    m_isDirectory[sName] = urlInfo.isDir();
    m_aFiles.append(sName);
    m_ulFileTotalSize += urlInfo.size();
}
//![10]
//![13]
void FtpWindow::updateDataTransferProgress(qint64 readBytes,
                                           qint64 totalBytes)
{
    if(m_ulFileTotalSize <= 0)
    {
        m_ulFileTotalSize = totalBytes;
    }
    if(m_ulFileTotalSize > 0)
    {
        emit signalDownLoadStep(int(100*(m_ulFileSize+readBytes)/m_ulFileTotalSize));
		
        if(readBytes >= totalBytes)
        {
            m_ulFileSize += totalBytes;
        }
    }
    else
    {
        emit signalDownLoadStep(0);
    }
}
//![13]
QString FtpWindow::getFtpInfoInFTPWindow()
{
	QString strInfo = "";
	QFtp::Error enumError = m_ftp->error();
	QFtp::State enumState =  m_ftp-> state();

	if (QFtp::Unconnected==enumState||QFtp::Closing==enumState)
	{
		strInfo= m_ftp->errorString();
	}
	if (QFtp::ConnectionRefused==enumError||QFtp::NotConnected==enumError||QFtp::HostNotFound==enumError)
	{
		strInfo= m_ftp->errorString();
	}
	return strInfo;
}

