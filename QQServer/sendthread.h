#ifndef SENDTHREAD_H
#define SENDTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include "global.h"

class SendThread : public QObject
{
    Q_OBJECT
public:
    explicit SendThread(QObject *parent = nullptr);
    void GetSendMsg(int type,int account, QString fileName); //获取要发送的信息及套接字
    void SendReply(QTcpSocket*,QByteArray,int); //发送回应给客户端
    void SendFile(QTcpSocket*,QString fileName); //发送文件
    void SendJson(QTcpSocket*,QByteArray); //发送Json数据

signals:

private:
    quint32 m_Infotype; //信息类型,发送文件时需发送该类型
    QString m_fileName; //需发送的文件名
    int m_account; //接收方账号
};

#endif // SENDTHREAD_H
