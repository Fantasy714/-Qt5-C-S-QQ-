#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <QThreadPool>
#include <QDir>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if(!m_dir.exists(m_path))
    {
        qDebug() << "未创建用户数据文件夹";
        if(!m_dir.mkdir(m_path))
        {
            qDebug() << "创建用户数据文件夹失败";
            return;
        }
    }

    //初始化数据库
    m_sqldata = new Qqsqldata(this);
    if(!m_sqldata->connectToSql()){
        qDebug() << "数据库连接失败!";
        return;
    }

    //服务器开始监听
    m_serv = new QTcpServer(this);
    m_serv->listen(QHostAddress::Any,m_port);
    ui->PlainTextEdit->appendPlainText(QString("服务器开始监听..."));
    connect(m_serv,&QTcpServer::newConnection,this,&MainWindow::ServerhasNewConnection);
}

MainWindow::~MainWindow()
{
    for(auto sock: m_sockets)
    {
        sock->close();
        sock->deleteLater();
    }
    delete ui;
}

void MainWindow::ServerhasNewConnection()
{
    QTcpSocket * sock = m_serv->nextPendingConnection();
    auto port = sock->peerPort();
    qDebug() << "服务器分配的端口号为:" << port;
    ui->PlainTextEdit->appendPlainText(QString("有新客户端连接到服务器,服务器分配的端口号为: " + QString::number(port)));
    connect(sock,&QTcpSocket::readyRead,this,&MainWindow::ReadMsgFromClt);
    connect(sock,&QTcpSocket::disconnected,this,&MainWindow::CltDisconnected);
    m_sockets.append(sock);
}

void MainWindow::ReadMsgFromClt()
{
    //sender返回指向发送信号的对象的指针
    QTcpSocket * sock = (QTcpSocket*)sender();
    WorkThread * WThread = new WorkThread(sock);
    connect(WThread,&WorkThread::ThreadbackMsg,this,&MainWindow::getThreadMsg);
    connect(WThread,&WorkThread::UserOnLine,this,&MainWindow::UserOnLine);
    QThreadPool::globalInstance()->start(WThread);
}

void MainWindow::CltDisconnected()
{
    QTcpSocket * sock = (QTcpSocket*)sender();
    QString str = "客户端断开连接，断开连接的客户端端口号为: " + QString::number(sock->peerPort());
    qDebug() << str;
    ui->PlainTextEdit->appendPlainText(str);
    m_sockets.removeOne(sock);
    sock->close();
    sock->deleteLater();
}

void MainWindow::getThreadMsg(QString type,int account,QString msg,int target)
{
    //将客户端请求内容显示到服务端界面上
    if(type == "注册")
    {
        QString res = type + ":账号" + QString::number(account) + "," + msg;
        ui->PlainTextEdit->appendPlainText(res);
    }
    else if(type == "找回密码")
    {
        if(msg == "")
        {
            QString res = type + ":账号" + QString::number(account) + ",找回密码失败";
            ui->PlainTextEdit->appendPlainText(res);
        }
        else
        {
            QString res = type + ":账号" + QString::number(account) + ",找回密码成功,密码为:" + msg;
            ui->PlainTextEdit->appendPlainText(res);
        }
    }
    else if(type == "登录")
    {
        QString res = type + ":账号" + QString::number(account) + "," + msg;
        ui->PlainTextEdit->appendPlainText(res);
        if(msg == "登录成功")
        {
            //若有其他用户在线则查找是否有该用户好友并通知该用户的好友该用户上线
            if(m_onlines.size() != 0)
            {

            }
        }
    }
}

void MainWindow::UserOnLine(int acc, QTcpSocket *sock)
{
    qDebug() << "账号： " << acc << "已加入在线哈希表";
    m_onlines.insert(acc,sock);
}
