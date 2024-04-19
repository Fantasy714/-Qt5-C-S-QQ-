#ifndef QQSQLDATA_H
#define QQSQLDATA_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

class Qqsqldata : public QObject
{
    Q_OBJECT
public:
    explicit Qqsqldata(QObject *parent = nullptr);
    bool connectToSql(); //连接数据库
    QString Addaccount(int account, QString pwd); //注册新用户
    bool Deleteaccount(int account); //注销账户
    QString FindPwd(int account); //找回密码
    int LoginVerification(int acc, QString pwd); //登录
    QStringList UserMessages(int acc); //返回用户资料
    bool ChangeOnlineSta(int acc,QString sta); //更改在线状态
    bool CreateFriends(int acc); //注册成功时创建该用户的好友列表
    bool AddFriend(int acc1,int acc2); //添加好友
    QVector<int> ReturnFris(int acc); //返回该用户的所有好友
    QString OnLineSta(int acc); //返回该用户的在线状态
signals:

private:
    QSqlDatabase db;
    QSqlQuery result;
};

#endif // QQSQLDATA_H
