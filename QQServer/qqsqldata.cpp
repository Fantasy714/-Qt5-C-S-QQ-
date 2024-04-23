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
    bool success = result.exec(QString("insert into qqaccount(account,pwd) value(%1,'%2');").arg(account).arg(pwd));
    if(success)
    {
        //成功则创建数据库好友列表
        CreateFriends(account);
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
    if(isonl != "离线")
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

QStringList Qqsqldata::UserMessages(int acc)
{
    QStringList userDatas;
    result.exec(QString("select nickname,signature,sex,age,birthday,location,blood_type,work,sch_comp from qqaccount where account = %1;").arg(acc));
    if(!result.next())
    {
        qDebug() << "查找资料失败";
        return userDatas;
    }
    QString userD;
    qDebug() << result.size();
    for(int i = 0; i < 9; i++)
    {
        userD = result.value(i).toString();
        userDatas.append(userD);
        //qDebug() << userD;
    }
    return userDatas;
}

bool Qqsqldata::ChangeUserMessages(int acc, QStringList uD)
{
    for(auto d : uD)
    {
        qDebug() << d;
    }

    bool suc = result.exec(QString("update qqaccount set nickname = '%1',signature = '%2',sex = '%3',age = %4,birthday = '%5',location = '%6',blood_type = '%7',work = '%8',sch_comp = '%9' "
                                   "where account = %10;").arg(uD.at(ennickname)).arg(uD.at(ensignature)).arg(uD.at(ensex)).arg(uD.at(enage).toInt()).arg(uD.at(enbirthday)).arg(uD.at(enlocation)).arg(uD.at(enblood_type)).arg(uD.at(enwork)).arg(uD.at(ensch_comp)).arg(acc));
    return suc;
}

bool Qqsqldata::ChangeOnlineSta(int acc,QString sta)
{
    bool suc = result.exec(QString("update qqaccount set onlinestatus = '%1' where account = %2;").arg(sta).arg(acc));
    if(!suc)
    {
        qDebug() << "在线状态更新失败";
    }
    return suc;
}

bool Qqsqldata::CreateFriends(int acc)
{
    bool res = result.exec(QString("insert into qqfriends(account,friends)value(%1,'')").arg(acc));
    if(!res)
    {
        qDebug() << "创建数据库好友列表失败";
    }
    return res;
}

bool Qqsqldata::AddFriend(int acc1, int acc2)
{
    //更新第一个用户的好友列表
    result.exec(QString("select friends from qqfriends where account = %1;").arg(acc1));
    if(!result.next())
    {
        qDebug() << "acc1查找好友失败!";
        return false;
    }
    QString fris1 = result.value("friends").toString();
    fris1 += QString::number(acc2) + ",";
    bool res = result.exec(QString("update qqfriends set friends = '%1' where account = %2;").arg(fris1).arg(acc1));
    if(res == false)
    {
        qDebug() << "更改好友列表失败";
        return false;
    }

    //更新第二个用户的好友列表
    result.exec(QString("select friends from qqfriends where account = %1;").arg(acc2));
    if(!result.next())
    {
        qDebug() << "acc1查找好友失败!";
        return false;
    }
    QString fris2 = result.value("friends").toString();
    fris2 += QString::number(acc1) + ",";
    res = result.exec(QString("update qqfriends set friends = '%1' where account = %2;").arg(fris2).arg(acc2));
    if(res == false)
    {
        qDebug() << "更改好友列表失败";
        return false;
    }
}

bool Qqsqldata::DelFriend(int acc1, int acc2)
{
    //更新第一个用户的好友列表
    result.exec(QString("select friends from qqfriends where account = %1;").arg(acc1));
    if(!result.next())
    {
        qDebug() << "acc1查找好友失败!";
        return false;
    }

    //获取好友列表字符串
    QString fris1 = result.value("friends").toString();
    //获取每个好友的账号
    QStringList friends1 = fris1.split(",");
    //删除好友的账号
    friends1.removeOne(QString::number(acc2));

    //清空并重新将其他好友加入字符串
    fris1.clear();
    for(auto f : friends1)
    {
        fris1.append(f);
        fris1.append(",");
    }

    //更新
    bool res = result.exec(QString("update qqfriends set friends = '%1' where account = %2;").arg(fris1).arg(acc1));
    if(res == false)
    {
        qDebug() << "更改好友列表失败";
        return false;
    }



    //更新第二个用户的好友列表
    result.exec(QString("select friends from qqfriends where account = %1;").arg(acc2));
    if(!result.next())
    {
        qDebug() << "acc2查找好友失败!";
        return false;
    }

    //获取好友列表字符串
    QString fris2 = result.value("friends").toString();
    //获取每个好友的账号
    QStringList friends2 = fris2.split(",");
    //删除好友的账号
    friends2.removeOne(QString::number(acc1));

    //清空并重新将其他好友加入字符串
    fris2.clear();
    for(auto f : friends2)
    {
        fris2.append(f);
        fris2.append(",");
    }

    //更新
    res = result.exec(QString("update qqfriends set friends = '%1' where account = %2;").arg(fris2).arg(acc2));
    if(res == false)
    {
        qDebug() << "更改好友列表失败";
        return false;
    }
}

QVector<int> Qqsqldata::ReturnFris(int acc)
{
    result.exec(QString("select friends from qqfriends where account = %1;").arg(acc));
    if(!result.next())
    {
        qDebug() << "查找好友失败";
    }
    QStringList frislist = result.value("friends").toString().split(",");
    QVector<int> v;
    for(auto fri : frislist)
    {
        v.push_back(fri.toInt());
        qDebug() << fri.toInt();
    }
    return v;
}

QString Qqsqldata::OnLineSta(int acc)
{
    result.exec(QString("select onlinestatus from qqaccount where account = %1;").arg(acc));
    if(!result.next())
    {
        qDebug() << "无该账号";
        return "";
    }
    QString sta = result.value("onlinestatus").toString();
    return sta;
}

