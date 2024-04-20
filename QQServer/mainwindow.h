#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qqsqldata.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QDir>
#include "workthread.h"
#include <QReadWriteLock>
#include "sendthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void ServerhasNewConnection(); //服务器有新连接
    void ReadMsgFromClt(); //有数据要读取
    void CltDisconnected(); //有客户端断开连接
public slots:
    void getThreadMsg(QString type,int account,QString msg,int target); //从工作线程中获取客户端的请求内容
    void UserOnLine(int acc,quint16 sockport); //用户上线则加入在线哈希表中
    void SendMsgToClt(quint16 port,int type,int acc,int targetacc,QByteArray jsondata,QString fileName,QString msgtype); //发送信息给客户端
private:
    Ui::MainWindow *ui;
    //保存信息类型
    enum InforType { Registration = 1125, FindPwd, LoginAcc, SearchFri, AddFri, ChangeOnlSta, SendMsg };
    Qqsqldata * m_sqldata; //数据库
    unsigned short m_port = 9000; //服务器监听端口号
    QTcpServer * m_serv; //Tcp服务器
    QHash<quint16,QTcpSocket*> m_sockets; //tcp通信套接字
    WorkThread * workT; //工作线程对象
    QHash<int,QTcpSocket*> m_onlines; //在线用户哈希表
    QString m_path = QCoreApplication::applicationDirPath() + "/usersdata"; //用户数据文件夹位置
    QDir m_dir; //操作文件目录
    static QReadWriteLock * mutex;
};
#endif // MAINWINDOW_H
