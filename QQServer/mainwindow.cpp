#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Global::CreateWorkPath();

    if(!m_sqldata.connectToSql())
    {
        qDebug() << "数据库连接失败";
        return;
    }

    //服务器开始监听
    m_serv = new QTcpServer(this);
    m_serv->listen(QHostAddress::Any,m_port);
    ui->PlainTextEdit->appendPlainText(QString("服务器开始监听..."));
    connect(m_serv,&QTcpServer::newConnection,this,&MainWindow::ServerhasNewConnection);

    m_ReadTd = new QThread;
    m_ReadTask = new ReadThread;
    connect(this,&MainWindow::StartRead,m_ReadTask,&ReadThread::ReadDataFromClt);
    m_ReadTask->moveToThread(m_ReadTd);
    m_ReadTd->start();

    m_WorkTd = new QThread;
    m_WorkTask = new WorkThread;
    connect(m_ReadTask,&ReadThread::RecvFinished,m_WorkTask,&WorkThread::ParseMsg);
    connect(m_WorkTask,&WorkThread::ThreadbackMsg,this,&MainWindow::getThreadMsg);
    connect(m_WorkTask,&WorkThread::UserOnLine,this,&MainWindow::UserOnLine);
    connect(m_WorkTask,&WorkThread::SendMsgToClt,this,&MainWindow::SendMsgToClt);
    m_WorkTask->moveToThread(m_WorkTd);
    m_WorkTd->start();

    m_SendTd = new QThread;
    m_SendTask = new SendThread;
    connect(this,&MainWindow::StartWrite,m_SendTask,&SendThread::SendReply);
    m_SendTask->moveToThread(m_WorkTd);
    m_SendTd->start();
}

MainWindow::~MainWindow()
{
    for(auto sock: m_sockets)
    {
        sock->close();
        sock->deleteLater();
    }

    m_ReadTd->quit();
    m_ReadTd->wait();
    m_ReadTd->deleteLater();
    m_ReadTask->deleteLater();

    m_WorkTd->quit();
    m_WorkTd->wait();
    m_WorkTd->deleteLater();
    m_WorkTask->deleteLater();

    m_SendTd->quit();
    m_SendTd->wait();
    m_SendTd->deleteLater();
    m_SendTask->deleteLater();

    m_serv->deleteLater();

    delete ui;
}

void MainWindow::ServerhasNewConnection()
{
    QTcpSocket * sock = m_serv->nextPendingConnection();
    quint16 port = sock->peerPort();
    qDebug() << "服务器分配的端口号为:" << port;
    ui->PlainTextEdit->appendPlainText(QString("有新客户端连接到服务器,服务器分配的端口号为: " + QString::number(port)));
    connect(sock,&QTcpSocket::readyRead,this,&MainWindow::ReadMsgFromClt);
    connect(sock,&QTcpSocket::disconnected,this,&MainWindow::CltDisconnected);
    m_sockets.insert(port,sock);
}

void MainWindow::ReadMsgFromClt()
{
    //sender返回指向发送信号的对象的指针
    QTcpSocket * sock = (QTcpSocket*)sender();
    emit StartRead(sock);
}

void MainWindow::CltDisconnected()
{
    QTcpSocket * sock = (QTcpSocket*)sender();
    QString str = "客户端断开连接，断开连接的客户端端口号为: " + QString::number(sock->peerPort());
    //若已登陆则将在线状态恢复为离线并从在线哈希表中删除
    int acc = m_onlines.key(sock);
    qDebug() << acc << "用户退出";
    if(acc != 0)
    {
        m_onlines.remove(acc);
        m_sqldata.ChangeOnlineSta(acc,"离线");
        str += ", 该客户端账号为: " + QString::number(acc);
    }
    ui->PlainTextEdit->appendPlainText(str);
    m_sockets.remove(sock->peerPort());
    sock->close();
    sock->deleteLater();
}

void MainWindow::getThreadMsg(InforType type,int account,QString msg,int target)
{
    //将客户端请求内容显示到服务端界面上
    switch(type)
    {
    case Registration:
    {
        QString res = "用户注册>>>账号: " + QString::number(account) + "," + msg;
        ui->PlainTextEdit->appendPlainText(res);
        break;
    }
    case FindPwd:
    {
        if(msg == "")
        {
            QString res = "用户找回密码>>>账号: " + QString::number(account) + ",找回密码失败";
            ui->PlainTextEdit->appendPlainText(res);
        }
        else
        {
            QString res = "用户找回密码>>>账号: " + QString::number(account) + ",找回密码成功,密码为:" + msg;
            ui->PlainTextEdit->appendPlainText(res);
        }
        break;
    }
    case LoginAcc:
    {
        QString res = "用户登录>>>账号: " + QString::number(account) + "," + msg;
        ui->PlainTextEdit->appendPlainText(res);
        if(msg == "登录成功")
        {
            //若有其他用户在线则查找是否有该用户好友并通知该用户的好友该用户上线
            if(m_onlines.size() != 0)
            {

            }
        }
        break;
    }
    case ChangeOnlSta:
    {
        QString res = "用户更新在线状态>>>账号:" + QString::number(account);
        ui->PlainTextEdit->appendPlainText(res);
        break;
    }
    }
}

void MainWindow::UserOnLine(int acc, quint16 sockport)
{
    QTcpSocket * thissock = m_sockets[sockport];
    m_onlines.insert(acc,thissock);
    qDebug() << "账号" << acc << "成功加入在线用户哈希表";
}

void MainWindow::SendMsgToClt(quint16 port, int type, int acc, int targetacc, QByteArray jsondata, QString msgtype, QString fileName)
{
    //发送文件数量
    int fileNum = 0;
    //发送的账号,接收端依据账号和信息类型判断文件存放的文件夹
    int sendAcc = -1;

    QTcpSocket * sock;

    //若需要转发则要获取目标客户端套接字
    if(msgtype == "发送好友申请" || msgtype == "成功删除好友" || type == SendMsg)
    {
        sock = m_onlines[targetacc];
    }
    else
    {
        sock = m_sockets[port];
        if(msgtype == "第一次登录") //第一次登录需发送头像和好友json文件
        {
            sendAcc = acc;
            fileNum = 2;
        }
    }

    if(msgtype == "发送好友申请" || type == SearchFri || msgtype == "需更新头像")
    {
        fileNum = 1;
        if(msgtype == "需更新头像")
        {
            sendAcc = acc;
        }
    }



    m_SendTask->GetSendMsg(type,sendAcc,fileName);
    emit StartWrite(sock,jsondata,fileNum);
}
