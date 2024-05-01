#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "qqsqldata.h"
#include "global.h"

class TcpThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpThread(QObject *parent = nullptr);
    ~TcpThread();
public slots:
    void StartListen(); //开始监听
    void UserOnLine(int acc,quint16 sockport); //用户上线则加入在线哈希表中
    void SendReply(quint16 port, int type, int acc, int targetacc, QByteArray jsondata, QString msgtype, QString fileName); //发送数据给客户端
protected:
    /* 客户端连接和断开连接 */
    void ServerhasNewConnection(); //有新的客户端连接
    void CltDisconnected(); //有客户端断开连接

    /* 读取和发送数据相关 */
    void ReadDataFromClt(); //读取数据
    void SendFile(QTcpSocket*,QString fileName,int infotype,int RecvAccount); //发送文件
    void SendJson(QTcpSocket*,QByteArray); //发送Json数据
signals:
    void NewCltConnected(quint16); //有客户端连接到服务器
    void CltDisConnected(quint16,int); //客户端断开连接
    void RecvFinished(QByteArray byteArray,quint16 port); //信息接收完成

private:
    /* 服务器 */
    quint16 m_port = 9000; //服务器监听端口号
    QTcpServer * m_serv;

    /* 在线用户套接字 */
    QHash<quint16,QTcpSocket*> m_sockets; //tcp通信套接字
    QHash<int,QTcpSocket*> m_onlines; //在线用户哈希表

    /* 数据操作 */
    QByteArray m_buffer; //保存数据

    /* 数据库操作 */
    Qqsqldata m_sql; //操作数据库
};

#endif // TCPTHREAD_H
