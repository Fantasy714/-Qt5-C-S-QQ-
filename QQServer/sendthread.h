#ifndef SENDTHREAD_H
#define SENDTHREAD_H

#include <QObject>
#include <QRunnable>
#include <QTcpSocket>

class SendThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit SendThread(QTcpSocket * sock,QByteArray jsondata,QString fileName,QString msgtype = "",QObject *parent = nullptr);
    void run() override;
    void SendReply(QByteArray jsondata,QString fileNames1); //发送回应给客户端

signals:

private:
    QTcpSocket * m_tcp; //tcp套接字
    QByteArray m_jsonData; //需发送的Json数据
    QString m_MsgType; //信息类型（辅助确认发送信息的具体类型）
    QString m_fileName; //需发送的文件名
};

#endif // SENDTHREAD_H
