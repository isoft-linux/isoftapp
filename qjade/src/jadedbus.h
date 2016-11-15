/*
 * Copyright (C) 2014 - 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *               2016 fujiang <fujiang.zhu@i-soft.com.cn>
 */

#ifndef JADEDBUS_H
#define JADEDBUS_H

#include <QObject>

//#include "jaded_interface.h"
#include "isoftapp-generated.h"

/* qjade 运行期间存在的队列，所有的安装/删除/升级等操作，数据直接来自此队列;
   没有操作时，此队列一般为空;
*/
class TaskQueue
{
public:
    TaskQueue(QString name,QString action,QString status="" ) //,QString icon,QString desc, )
    {
        this->name = name;
        //this->icon = icon;
        //this->desc = desc;
        this->status = status;
        this->action = action;
    }
public:
    QString name; // 不允许重复
    QString icon;
    QString desc;
    QString status; // “i”-初始;
    QString action;
};

class JadedPackageObject;

class JadedBus : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> updates READ updates NOTIFY updateChanged)
    Q_PROPERTY(QList<QObject*> installed READ installed NOTIFY installedChanged)
    Q_PROPERTY(QList<QObject*> searchs READ searchs NOTIFY searchChanged)
    Q_PROPERTY(QList<QObject*> taskQueue READ taskQueue NOTIFY taskQueueChanged)

public:
    JadedBus(QObject *parent = 0);
    ~JadedBus();

    Q_INVOKABLE QString getInfo(QString name);
    Q_INVOKABLE void getUpdate();
    Q_INVOKABLE void getInstalled();
    Q_INVOKABLE void search(QString name);
    Q_INVOKABLE void install(QString name);
    Q_INVOKABLE void installFile(QString filePath);
    Q_INVOKABLE void uninstall(QString id);
    Q_INVOKABLE void update(QString name);
    Q_INVOKABLE void cancel(QString id);
    Q_INVOKABLE void getTaskQueue();
    Q_INVOKABLE void runCmd(QString cmd);
    Q_INVOKABLE void setPathMode(QString path,QString mode);
    Q_INVOKABLE void getPathMode();
    Q_INVOKABLE void getMyPkgNumber();
    Q_INVOKABLE QString getBackend(QString name);

    QList<QObject*> updates() const { return m_updateList; }
    QList<QObject*> installed() const { return m_installedList; }
    QList<QObject*> searchs() const { return m_searchList; }
    QList<QObject*> taskQueue() const { return m_oldQueue; }

Q_SIGNALS:
    void errored(const QString &name,const QString &detail);
    void updateChanged(int count);
    void installedChanged();
    void searchChanged(int count);
    void taskStarted(const QString &name);
    void taskChanged(const QString &name);
    void taskQueueChanged(int count);
    void getUpdateError();
    void getInstalledError();
    void searchError();
    void perChanged(const QString &name,int perCent);
    void settingChanged(const QString &path,int mode); // format [path|||mode]
    void myPkgNumChanged(int count);

private Q_SLOTS:
    void m_errored(const QString &name,const QString &detail);
    void getUpdateFinished(const QString &update);
    void getInstalledFinished(const QStringList &installed);
    void searchFinished(const QStringList &search);
    void m_taskStarted(const QString &name);
    void taskFinished(const QString &name);
    void m_taskQueueChanged(const QStringList &task);
    void getUpdateTimeout();
    void getInstalledTimeout();
    void searchTimeout();
    void percentChged(qlonglong status, const QString &file, double percent);
    void errorChged(qlonglong error, const QString &details, qlonglong errcode);
    void searchChged(const QString &pkgName,qlonglong status);
    void listChged(const QString &pkgName);
    void getAllPkgList(const QString &pkgName,qlonglong status);
    void getFinished(const QString &pkgName,qlonglong status);
    void runTaskTimeOut();
    void getPkgListTimeOut();
    void getSettingChanged(const QString &pathMode);
    void getMyPkgNumTimeout();
    void getIconTimeOut();

private:
    //cn::com::isoft::JadedInterface* m_jaded;
    //org::isoftlinux::Isoftapp *m_isoftapp = Q_NULLPTR;
    QList<QObject*> m_updateList;
    QList<QObject*> m_installedList;
    QList<QObject*> m_searchList;
    QList<QObject*> m_oldQueue; // 通过 taskQueue（）被qml使用

    bool m_isTaskExist(QString name);
    void m_runTask();

    QTimer *runTaskTimer; // runTaskTimeOut()
    QTimer *getPkgListTimer;
    QTimer *getIconTimer;
};

class JadedPackageObject : public QObject 
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString version READ version WRITE setVersion 
               NOTIFY versionChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription 
               NOTIFY descriptionChanged)

    Q_PROPERTY(QString category READ category WRITE setCategory
               NOTIFY categoryChanged)
    Q_PROPERTY(QString size READ size WRITE setSize
               NOTIFY sizeChanged)
    Q_PROPERTY(QString datetime READ datetime WRITE setDatetime
               NOTIFY datetimeChanged)

public:
    JadedPackageObject(const QString &id, 
                        const QString &name, 
                        const QString &icon, 
                        const QString &version, 
                        const QString &description, 
                        const QString &category,
                       const QString &size,
                       const QString &datetime,
                        QObject *parent = 0) 
      : QObject(parent) 
    {
        m_id = id; m_name = name; m_icon = icon; m_version = version; 
        m_description = description;
        m_category = category;
        m_size = size;
        m_datetime = datetime;
    }

    QString id() const { return m_id; }
    void setId(const QString &id) 
    {
        if (id != m_id) m_id = id; emit idChanged();
    }

    QString name() const { return m_name; }
    void setName(const QString &name) 
    { 
        if (name != m_name) m_name = name; emit nameChanged(); 
    }

    QString icon() const { return m_icon; }
    void setIcon(const QString &icon) 
    {
        if (icon != m_icon) m_icon = icon; emit iconChanged();
    }

    QString version() const { return m_version; }
    void setVersion(const QString &version) 
    {
        if (version != m_version) m_version = version; emit versionChanged();
    }

    QString description() const { return m_description; }
    void setDescription(const QString &description) 
    {
        if (description != m_description) 
            m_description = description; emit descriptionChanged();
    }

    QString category() const { return m_category; }
    void setCategory(const QString &category)
    {
        if (category != m_category)
            m_category = category; emit categoryChanged();
    }

    QString size() const { return m_size; }
    void setSize(const QString &size)
    {
        if (size != m_size)
            m_size = size; emit sizeChanged();
    }
    QString datetime() const { return m_datetime; }
    void setDatetime(const QString &datetime)
    {
        if (datetime != m_datetime)
            m_datetime = datetime; emit datetimeChanged();
    }


Q_SIGNALS:
    void idChanged();
    void nameChanged();
    void iconChanged();
    void versionChanged();
    void descriptionChanged();
    void categoryChanged();
    void sizeChanged();
    void datetimeChanged();

private:
    QString m_id;
    QString m_name;
    QString m_icon;
    QString m_version;
    QString m_description;
    QString m_category;
    QString m_size;
    QString m_datetime;
};

#endif  // JADEDBUS_H
