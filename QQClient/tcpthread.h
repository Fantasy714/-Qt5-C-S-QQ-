#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include "global.h"
#include <QBuffer>

class TcpThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpThread(QObject *parent = nullptr);
    void connectToServer(); //连接服务器
    void ConnectSuccess(); //连接服务器成功
    void ReadMsgFromServer(); //接收服务器传来的数据
    void ParseMsg(QByteArray); //解析传回的数据
    void DisconnectFromServer(); //服务器断开连接
    void MsgToJson(InforType type,int acc = -1,int targetacc = -1,QString Msg = "",QString MsgType = ""); //将数据转化为Json数据
    void SendToServer(QByteArray jsondata, QString fileName,InforType type,int fileNums); //发送数据给服务器
    void AutoConnect(); //自动重连
public slots:
    void StartConnect(); //开始连接,连接失败则自动重连
    void GetClose(); //接收关闭信号
    void recvAccMsg(QString type,int acc,QString pwd); //获取注册界面返回的信息
    void LoginToServer(bool isfirst,int acc,QString pwd,bool isChecked); //登录
    void sendSearchFriMsgToSer(int acc); //向服务器发送查找好友信息
    void sendFriAddMsgToSer(int myacc, int targetacc,QString type,QString yanzheng = ""); //发送好友申请信息给服务器
    void ChangeOnlineSta(int acc, QString onl); //改变在线状态
    void sendSmsToFri(int acc,int targetAcc,QString MsgType,QString Msg); //向好友发送信息
    void AskForUserData(QString isMe, int acc); //请求用户个人资料
    void ChangingUserDatas(int,QString); //更改用户资料
    void ChangingHS(int acc,QString fileN); //修改头像
signals:
    void sendResultToAccMsg(QString type,QString pwd,QString result); //发送结果回注册界面
    void sendResultToLogin(QString result); //发送结果回登录界面
    void sendResultToMainInterFace(int type,int acc,QString nickname,QString signature,QString result,QString uData,QString Msg = "",QString MsgType = ""); //发送结果回登录界面
    void isConnectingWithServer(bool onl); //连接成功时发出true,服务器断开连接发送false
protected:
    void CutPhoto(int acc,QString path);
    void SendFile(QString fileName,quint32 type); //发送文件
    void SendJson(QByteArray jsonData); //发送Json数据
    void WriteToFile(QString fileName);
private:
    QTcpSocket * m_tcp = nullptr; //tcp套接字
    unsigned short port = 9000; //端口号
    QString address = "127.0.0.1"; //ip地址
    int m_account; //账号
    QString m_pwd; //密码
    bool isConnecting = false; //是否已连接服务器
    QTimer * m_timer = nullptr; //定时器
    bool isFirstLogin; //是否是第一次登录
    QDir m_dir; //操作文件夹
    QFile m_file; //操作文件
    bool isRemember; //是否记住密码

    /* 发送 */
    int m_Recvaccount; //接收方账号

    /* 接收 */
    QByteArray m_byteArray; //保存数据
    quint16 m_type; //包头类型
    quint32 m_totalBytes; //数据包实际大小
    quint32 m_recvBytes; //已接收数据的大小

    QBuffer m_buffer; //数据缓存
    quint32 m_infotype; //具体数据类型
    QString m_fileName; //文件名称
    quint32 m_fileSize; //文件大小
    int m_TargetAcc; //文件发送目标账号

    int m_recvFileSize; //已接收的文件数据大小

    /* 操作用户数据文件夹 */
    const QString m_path = QCoreApplication::applicationDirPath() + "/userdata";
    const QString m_alluserspath = QCoreApplication::applicationDirPath() + "/userdata/allusers";
};

#endif // TCPTHREAD_H
