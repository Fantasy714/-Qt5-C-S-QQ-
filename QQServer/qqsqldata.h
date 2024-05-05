#ifndef QQSQLDATA_H
#define QQSQLDATA_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "global.h"

class Qqsqldata : public QObject
{
    Q_OBJECT
public:
    explicit Qqsqldata(QObject* parent = nullptr);
    bool connectToSql(); //连接数据库

    /* 用户未登录时操作 */
    QString Addaccount(int account, QString pwd); //注册新用户
    bool CreateFriends(int acc); //注册成功时在数据库创建该用户的好友列表
    QString FindPwd(int account); //找回密码
    int LoginVerification(int acc, QString pwd); //登录

    /* 用户登录后操作 */
    QVector<int> ReturnFris(int acc); //返回该用户的所有好友
    QStringList UserMessages(int acc); //返回用户资料
    bool ChangeUserMessages(int acc, QStringList uD); //更改用户资料
    bool AddFriend(int acc1, int acc2); //添加好友
    bool DelFriend(int acc1, int acc2); //删除好友

    /* 其他 */
    bool ChangeOnlineSta(int acc, QString sta); //更改在线状态
    QString OnLineSta(int acc); //返回该用户的在线状态
    bool Deleteaccount(int account); //注销账户
signals:

private:
    QSqlDatabase db; //连接数据库
    QSqlQuery result; //操作数据库
};

#endif // QQSQLDATA_H
