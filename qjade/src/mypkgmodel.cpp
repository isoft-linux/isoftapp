/*                                                                                                       
 *               2016 fj <fujiang@i-soft.com.cn>
 */

#include <QNetworkRequest>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLocale>
#include <QTimer>

#include "mypkgmodel.h"
#include "globaldeclarations.h"
#include "util.h"
#include "globallist.h"
extern bool g_offline;

MypkgModel::MypkgModel(QObject *parent)
  : QObject(parent), 
    m_category("") 
{
}

MypkgModel::~MypkgModel()
{
    foreach (QObject *obj, m_dataList) {                                           
        if (obj) delete obj; obj = NULL;                                           
    }                                                                              
    m_dataList.clear();
}

void MypkgModel::m_get()
{
    QTimer::singleShot(3000, this, SLOT(getPackageFinished()));
}

//-----------------------------------------------------------------------------
// TODO: 让QML来set category值
//-----------------------------------------------------------------------------
void MypkgModel::setCategory(QString category)
{
    m_get();
    printf("\n%s,%d,cat[%s]\n",__FUNCTION__,__LINE__,qPrintable(category));
}

/*
* 点击左侧类别，在右侧显示软件包列表
*
*/
void MypkgModel::getPackageFinished()
{
    for (int i = 0; i < AllPkgList.size(); ++i) {
        if(AllPkgList.at(i).datetime == "0") {
            continue;
        }
        for(int j =0;j< g_qjadePkgList.size();j++) {
            if (AllPkgList.at(i).pkgName == g_qjadePkgList.at(j).name) {
                QString installed="";
                if (AllPkgList.at(i).status == 1) {
                    installed = "1";
                } else {
                    installed = "0";
                }
                m_dataList.append(new MyPackageObject(g_qjadePkgList.at(j).name,
                    g_qjadePkgList.at(j).title,
                    g_qjadePkgList.at(j).description,
                    g_qjadePkgList.at(j).icon,
                    g_qjadePkgList.at(j).url,
                    installed));
            }
        }
    }

    printf("trace:%s,%d,g_qjadePkgList[%d],AllPkgList[%d].\n",__FUNCTION__,__LINE__,g_qjadePkgList.size(),AllPkgList.size());
    if (m_dataList.size() > 0 && g_offline != true) {
        emit packageChanged();
    } else {
        emit error();
    }
}

QString MypkgModel::category() const { return m_category; }

QList<QObject*> MypkgModel::packages() const { return m_dataList; }

void MypkgModel::removeAt(int index) { m_dataList.removeAt(index); }
