/*                                                                              
 * Copyright (C) 2014 AnthonOS Open Source Community                               
 *               2014 Leslie Zhai <xiang.zhai@i-soft.com.cn>                       
 */

#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H
 
#include <QObject>
#include <QFile>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
 
class FileDownloader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString dir READ dir WRITE setDir NOTIFY dirChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(QString filePath READ filePath NOTIFY filePathChanged)

public:
    FileDownloader();
    ~FileDownloader();

    QString url() const;
    void setUrl(QString &url);
    QString dir() const;
    void setDir(QString &dir);
    QString fileName() const;
    QString filePath() const;
    QByteArray downloadedData() const;

    Q_INVOKABLE void cancel();

signals:
    void urlChanged();
    void dirChanged();
    void fileNameChanged();
    void filePathChanged();
    void downloaded();
    void progress(qint64 bytesReceived, qint64 bytesTotal);
 
private slots:
    void finished(QNetworkReply *reply);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
 
private:
    void m_cleanup();
    
    QString m_Url;
    QString m_Dir;
    QString m_FileName;
    QString m_FilePath;
    QNetworkAccessManager m_NAM;
    QNetworkReply *m_NR;
    QByteArray m_DownloadedData;
    QFile *m_File;
};
 
#endif // FILEDOWNLOADER_H
