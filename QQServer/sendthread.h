#ifndef SENDTHREAD_H
#define SENDTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include "global.h"
#include <QCoreApplication>

class SendThread : public QObject
{
    Q_OBJECT
public:
    explicit SendThread(QObject *parent = nullptr);
    void GetSendMsg(QTcpSocket * sock, InforType type,int account, QByteArray jsonData, QString fileName,int fileNum); //获取要发送的信息及套接字
    void SendReply(); //发送回应给客户端
    void SendFile(QString fileName); //发送文件
    void SendJson(); //发送Json数据

signals:

private:
    QTcpSocket * m_tcp; //tcp套接字
    quint32 m_Infotype; //信息类型,发送文件时需发送该类型
    QByteArray m_jsonData; //需发送的Json数据
    QString m_fileName; //需发送的文件名
    int m_fileNum; //发送文件的数量
    int m_account; //接收方账号

    const QString m_path = QCoreApplication::applicationDirPath() + "/usersdata"; //用户文件夹
};

#endif // SENDTHREAD_H
