#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include "qqsqldata.h"
#include "workthread.h"
#include "sendthread.h"
#include "readthread.h"
#include "global.h"

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
    void getThreadMsg(InforType type,int account,QString msg,int target); //从工作线程中获取客户端的请求内容
    void UserOnLine(int acc,quint16 sockport); //用户上线则加入在线哈希表中
    void SendMsgToClt(quint16 port,int type,int acc,int targetacc,QByteArray jsondata,QString msgtype,QString fileName); //发送信息给客户端
signals:
    void StartRead(); //开始读取客户端信息
    void StartWrite(QTcpSocket*,QByteArray,int); //开始发送信息给客户端
private:
    Ui::MainWindow *ui;

    Qqsqldata m_sqldata; //数据库

    unsigned short m_port = 9000; //服务器监听端口号
    QTcpServer * m_serv; //Tcp服务器

    QThread * m_ReadTd; //读线程
    ReadThread * m_ReadTask; //读线程任务类

    QThread * m_WorkTd; //工作线程
    WorkThread * m_WorkTask; //工作线程任务类

    QThread * m_SendTd; //写线程
    SendThread * m_SendTask; //写线程任务类

    QHash<quint16,QTcpSocket*> m_sockets; //tcp通信套接字
    QHash<int,QTcpSocket*> m_onlines; //在线用户哈希表

    const QString m_path = QCoreApplication::applicationDirPath() + "/usersdata"; //用户数据文件夹
};
#endif // MAINWINDOW_H
