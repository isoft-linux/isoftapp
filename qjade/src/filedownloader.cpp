/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

#include <QStandardPaths>

#include "filedownloader.h"

FileDownloader::FileDownloader() 
    :m_Dir(QStandardPaths::writableLocation(QStandardPaths::TempLocation)) 
{}

FileDownloader::~FileDownloader() { m_cleanup(); }

void FileDownloader::m_cleanup() 
{
    disconnect(&m_NAM, SIGNAL(finished(QNetworkReply*)),                            
        this, SLOT(finished(QNetworkReply*)));                                     
                                                                                   
    if (m_File) m_File->close(); delete m_File; m_File = NULL;                     
                                                                                   
    if (m_NR) {                                                                    
        disconnect(m_NR, &QNetworkReply::downloadProgress,                          
            this, &FileDownloader::downloadProgress);                              
        m_NR->deleteLater();                                                       
        m_NR = NULL;                                                               
    }
}

QString FileDownloader::url() const { return m_Url; }
void FileDownloader::setUrl(QString &url) 
{
    if (url == "") return;
    m_Url = url;
    m_FileName = QUrl(m_Url).fileName();
    m_FilePath = m_Dir + "/" + m_FileName;
    m_File = new QFile(m_FilePath);
    if (m_File) m_File->open(QIODevice::WriteOnly);
    connect(&m_NAM, SIGNAL(finished(QNetworkReply*)),                              
        this, SLOT(finished(QNetworkReply*)));                                     
    printf("\n%s,%d,[%s]\n",__FUNCTION__,__LINE__,qPrintable(url));
    m_NR = m_NAM.get(QNetworkRequest(QUrl(m_Url)));                                        
    if (m_NR) {                                                                    
        connect(m_NR, &QNetworkReply::downloadProgress,                            
            this, &FileDownloader::downloadProgress);                              
    }
    emit urlChanged();
    emit fileNameChanged();
    emit filePathChanged();
}

QString FileDownloader::dir() const { return m_Dir; }
void FileDownloader::setDir(QString &dir) 
{
    if (dir != m_Dir) m_Dir = dir; emit dirChanged();
}

QString FileDownloader::fileName() const { return m_FileName; }

QString FileDownloader::filePath() const { return m_FilePath; }

void FileDownloader::cancel() { if (m_NR) m_NR->abort(); m_cleanup(); }

void FileDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) 
{
    //qDebug() << "DEBUG: " << bytesReceived << bytesTotal;
    emit progress(bytesReceived, bytesTotal);
}

void FileDownloader::finished(QNetworkReply *reply) 
{ 
    m_DownloadedData = reply->readAll();
    if (m_File) m_File->write(m_DownloadedData); m_cleanup();
    emit downloaded(); 
}
 
QByteArray FileDownloader::downloadedData() const { return m_DownloadedData; }
