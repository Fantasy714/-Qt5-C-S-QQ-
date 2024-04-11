#include "qqsqldata.h"
#include <QtDebug>
#include <QTime>
#include <QtGlobal>

Qqsqldata::Qqsqldata(QObject *parent) : QObject(parent)
{

}

bool Qqsqldata::connectToSql()
{
    db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("qqdata");
    db.setUserName("root");
    db.setPassword("123456");
    bool ok = db.open();
    if(!ok){
        return false;
    } else {
        result = (QSqlQuery)db; //进行绑定 此后可以使用query对象对数据库进行操作
        return true;
    }
}

QString Qqsqldata::Addaccount(int account, QString pwd)
{
    QString picname = QString::number(account) + ".jpg";
    bool success = result.exec(QString("insert into qqaccount(account,pwd,headshot) value(%1,'%2','%3');").arg(account).arg(pwd).arg(picname));
    if(success)
    {
        //设置随机数种子
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        int photoId = rand() % 7 + 1; //随机分配一个默认头像编号
        QString headshot = ":/lib/DefaultHead/touxiang" + QString::number(photoId) + ".jpg";
        return headshot;
    }
    else
    {
        return "";
    }
}

bool Qqsqldata::Deleteaccount(int account)
{
    bool success = result.exec(QString("delete from qqaccount where account = %1;").arg(account));
    return success;
}

QString Qqsqldata::FindPwd(int account)
{
    QString pwd = "";
    result.exec(QString("select pwd from qqaccount where account = %1;").arg(account));
    if(!result.next()){
        qDebug() << "找回密码失败";
    }
    pwd = result.value("pwd").toString();
    return pwd;
}

//验证登录账户密码是否错误，返回1为登录成功，返回0为重复登录，返回-1为账号密码错误
int Qqsqldata::LoginVerification(int acc, QString pwd)
{
    result.exec(QString("select pwd,onlinestatus from qqaccount where account = %1;").arg(acc));
    if(!result.next())
    {
        qDebug() << "账户错误";
        return -1;
    }
    QString spwd = result.value("pwd").toString();
    if(spwd != pwd)
    {
        qDebug() << "密码错误";
        return -1;
    }
    QString isonl = result.value("onlinestatus").toString();
    if(isonl == "离线")
    {
        qDebug() << "重复登录";
        return 0;
    }
    bool suc = result.exec(QString("update qqaccount set onlinestatus = '在线' where account = %1;").arg(acc));
    if(!suc)
    {
        qDebug() << "在线状态更新失败";
    }
    return 1;
}

