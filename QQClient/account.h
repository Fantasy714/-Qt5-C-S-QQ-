#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QWidget>
#include <QMovie>
#include <QMutex>
#include <QCloseEvent>
#include "global.h"

namespace Ui {
class Account;
}

class Account : public QWidget
{
    Q_OBJECT

public:
    static Account* GetAccount(bool find); //单例模式
    ~Account();
    Account& operator=(const Account&) = delete; //单例模式(删除拷贝构造和赋值构造)
    Account(Account &) = delete;
    void closeEvent(QCloseEvent *event) override; //重写关闭事件，退出则返回登录界面

public slots:
    void recvResultFromTcp(int type,QString pwd,QString result); //接收tcp套接字传回的结果
    void DisConnectedFromSer(bool onl); //断开连接

protected:
    void isFind(bool find); //为找回密码需隐藏密码输入栏

signals:
    void BacktoLogin(bool fromAcc); //返回登录界面
    void AccountReq(int type,int acc,QString pwd = ""); //发送账号密码信息

private slots:
    void on_pushButton_2_clicked(); //返回

    void on_pushButton_clicked(); //注册/找回

private:
    explicit Account(bool find,QWidget *parent = nullptr); //单例模式（将构造函数私有）
    Ui::Account *ui;
    bool m_find = false; //是否为找回密码
    QMovie * m_movie; //动态图
    static Account * m_Acc; //单例模式
    static QMutex m_mutex; //锁
};

#endif // ACCOUNT_H
