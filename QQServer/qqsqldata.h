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
signals:

private:
    QSqlDatabase db;
    QSqlQuery result;
};

#endif // QQSQLDATA_H
