#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QCoreApplication>

//保存信息类型
enum InforType { Registration = 1125, FindPwd, LoginAcc, SearchFri, AddFri, ChangeOnlSta, SendMsg };

class TcpThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpThread(QObject *parent = nullptr);
    void connectToServer(); //连接服务器
    void ConnectSuccess(); //连接服务器成功
    void ReadMsgFromServer(); //接收服务器传来的数据
    void ParseMsg(QByteArray,QByteArray); //解析传回的数据
    void DisconnectFromServer(); //服务器断开连接
    void MsgToJson(InforType type,int acc = -1,int targetacc = -1,QString Msg = "",QString yanzheng = ""); //将数据转化为Json数据
    void SendToServer(QByteArray jsondata, QString fileName); //发送数据给服务器
    void AutoConnect(); //自动重连
public slots:
    void StartConnect(); //开始连接,连接失败则自动重连
    void GetClose(); //接收关闭信号
    void recvAccMsg(QString type,int acc,QString pwd); //获取注册界面返回的信息
    void LoginToServer(bool isfirst,int acc,QString pwd,bool isChecked); //登录
    void sendSearchFriMsgToSer(int acc); //向服务器发送查找好友信息
    void sendFriAddMsgToSer(int myacc, int targetacc,QString type,QString yanzheng = ""); //发送好友申请信息给服务器
    void ChangeOnlineSta(int acc, QString onl); //改变在线状态
signals:
    void sendResultToAccMsg(QString type,QString pwd,QString result); //发送结果回注册界面
    void sendResultToLogin(QString result); //发送结果回登录界面
    void sendResultToMainInterFace(int type,int acc,QString nickname,QString signature,QString result,QString uData,QString Msg = "",QString MsgType = ""); //发送结果回登录界面
    void isConnectingWithServer(bool onl); //连接成功时发出true,服务器断开连接发送false
private:
    QTcpSocket * m_tcp = nullptr; //tcp套接字
    unsigned short port = 9000; //端口号
    QString address = "127.0.0.1"; //ip地址
    int m_account; //账号
    QString m_pwd; //密码
    bool isConnecting = false; //是否已连接服务器
    QTimer * m_timer = nullptr; //定时器
    bool isFirstLogin; //是否是第一次登录
    QString m_path = QCoreApplication::applicationDirPath() + "/userdata"; //用户数据文件夹位置
    QString m_alluserspath = m_path + "/allusers"; //存放好友头像文件夹位置
    QDir m_dir; //操作文件夹
    QFile m_file; //操作文件
    bool isRemember; //是否记住密码
};

#endif // TCPTHREAD_H
