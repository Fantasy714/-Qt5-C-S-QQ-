#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QObject>
#include <QRunnable>
#include <QReadWriteLock>
#include <QTcpSocket>
#include "qqsqldata.h"
#include <QJsonDocument>
#include <QCoreApplication>
#include <QDir>

class WorkThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit WorkThread(QTcpSocket * tcp,QObject *parent = nullptr);
    void run() override;
    void recvData(QByteArray,QByteArray); //解析收到的数据
    void ReplyToJson(QString type,QString pwd,QString result = "", QString fileName = "", int acc = 0); //将信息转换为json
    void SendReply(QByteArray jsondata,QString fileNames1);
    void recvRegistered(int,QString); //查找注册结果
    void recvFind(int); //查找找回密码结果
    void CltLogin(int,QString); //客户端登录
signals:
    void ThreadbackMsg(QString type,int account,QString msg,int target = 0); //从线程传消息回服务器
    void UserOnLine(int acc,quint16 sockport); //用户上线则发送该tcp套接字加入服务器在线用户哈希表中
private:
    Qqsqldata sql; //连接数据库
    static QReadWriteLock mutex; //读写锁
    QTcpSocket * m_tcp; //tcp套接字
    QString m_path = QCoreApplication::applicationDirPath() + "/usersdata"; //用户数据文件夹位置
    QDir m_dir; //操作文件目录
    bool isFirstLogin = false; //是否是第一次登录
};

#endif // WORKTHREAD_H
