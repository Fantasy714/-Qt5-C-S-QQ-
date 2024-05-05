#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QFile>
#include <QTime>
#include "global.h"

class TcpThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpThread(QObject* parent = nullptr);

    //发送接收信息
    void SendToServer(QByteArray jsondata, QString fileName, InforType type, int fileNums, int RecvAccount); //发送数据给服务器
    void ReadMsgFromServer(); //接收服务器传来的数据

    //解析和转换信息
    void MsgToJson(int type, int acc = -1, int targetacc = -1, QString Msg = "", QString MsgType = ""); //将数据转换为Json数据
    void ParseMsg(QByteArray); //解析传回的数据

    bool isCloseing = false; //是否准备退出
public slots:
    //连接服务器端
    void StartConnect(); //开始连接,连接失败则自动重连
    void GetClose(); //接收关闭信号

    //收到注册界面信息
    void recvAccMsg(int type, int acc, QString pwd); //获取注册界面返回的信息
    //收到登录信号
    void LoginToServer();
signals:
    //发送结果回注册界面和登录界面
    void sendResultToAccMsg(int type, QString pwd, QString result);
    void sendResultToLogin(QString result);
    //连接成功时发出true,服务器断开连接发送false
    void isConnectingWithServer(bool onl);
    //发送结果回主界面
    void sendResultToMainInterFace(int type, int targetAcc, QString uData = "", QString Msg = "", QString MsgType = "");
    //发送文件收发进度信息
    void SendProgressInfo(int friAcc, QString fileName, qint64 speed, int value, QTime RestTime, bool isMe);
protected:
    //连接服务器及自动重连操作
    void connectToServer(); //连接服务器
    void ConnectSuccess(); //连接服务器成功
    void DisconnectFromServer(); //服务器断开连接
    void AutoConnect(); //自动重连

    //网络传输相关
    void SplitDataPackAge(QByteArray); //拆分数据包
    void SendFile(QString fileName, quint32 type, int RecvAccount); //发送文件
    void SendJson(QByteArray jsonData); //发送Json数据
private:
    QTcpSocket* m_tcp = nullptr; //tcp套接字
    quint32 port = 9000; //端口号
    QString address = "127.0.0.1"; //ip地址

    /* 自动重连 */
    QTimer* m_timer = nullptr; //定时器

    /* 接收 */
    QByteArray m_buffer; //保存数据

    /* 接收文件 */
    QFile m_file; //操作文件
    qint64 m_fileSize; //文件大小
    int FilePackAgeCount; //已接收文件数据包个数

    /* 接收好友文件 */
    int m_InfoType; //发送类型
    int m_SendAcc; //发送方账号
    double m_lastsize; //上次已接收大小
    double m_nowsize; //本次已接收大小
    QTime m_lastT; //上次接收时间
    QTime m_nowT; //本次接收时间
};

#endif // TCPTHREAD_H
