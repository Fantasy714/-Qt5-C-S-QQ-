#include "sqldata.h"
#include <QtDebug>

SqlData::SqlData(QObject * parent)
{

}

void SqlData::ConnectSql()
{
    db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("qqdata");
    db.setUserName("root");
    db.setPassword("123456");
    bool ok = db.open();
    if(!ok){
        qDebug() << "数据库启动失败";
    }
}
