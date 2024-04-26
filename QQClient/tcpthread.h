#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDir>
#include <QFile>
#include "global.h"
#include <QBuffer>

class TcpThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpThread(QObject *parent = nullptr);

    //传送接收信息
    void SendToServer(QByteArray jsondata, QString fileName,InforType type,int fileNums); //发送数据给服务器
    void ReadMsgFromServer(); //接收服务器传来的数据

    //解析和封装信息
    void MsgToJson(int type,int acc = -1,int targetacc = -1,QString Msg = "",QString MsgType = ""); //将数据转化为Json数据
    void ParseMsg(QByteArray); //解析传回的数据
public slots:
    //连接服务器端
    void StartConnect(); //开始连接,连接失败则自动重连
    void GetClose(); //接收关闭信号

    //收到注册界面信息
    void recvAccMsg(int type,int acc,QString pwd); //获取注册界面返回的信息

    //收到登录信号
    void LoginToServer();
signals:
     //发送结果回注册界面和登录界面
    void sendResultToAccMsg(int type,QString pwd,QString result);
    void sendResultToLogin(QString result);
    //连接成功时发出true,服务器断开连接发送false
    void isConnectingWithServer(bool onl);
    //发送结果回主界面
    void sendResultToMainInterFace(int type,int targetAcc,QString uData = "",QString Msg = "",QString MsgType = "");
protected:
    //连接服务器及自动重连操作
    void connectToServer(); //连接服务器
    void ConnectSuccess(); //连接服务器成功
    void DisconnectFromServer(); //服务器断开连接
    void AutoConnect(); //自动重连

    //网络传输相关
    void SendFile(QString fileName,quint32 type); //发送文件
    void SendJson(QByteArray jsonData); //发送Json数据
    void WriteToFile(QString fileName); //写入文件
private:
    QTcpSocket * m_tcp = nullptr; //tcp套接字
    quint32 port = 9000; //端口号
    QString address = "127.0.0.1"; //ip地址

    /* 自动重连 */
    QTimer * m_timer = nullptr; //定时器

    /* 发送 */
    int m_Recvaccount; //接收方账号

    /* 接收 */
    QByteArray m_byteArray; //保存数据
    quint16 m_type; //包头类型
    quint32 m_totalBytes; //数据包实际大小
    quint32 m_recvBytes; //已接收数据的大小

    QBuffer m_buffer; //数据缓存
    quint32 m_infotype; //具体数据类型
    int m_TargetAcc; //文件发送目标账号
    QString m_fileName; //文件名称
    quint32 m_fileSize; //文件大小
    int m_recvFileSize; //已接收的文件数据大小
};

#endif // TCPTHREAD_H
