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

    //若无本地数据文件夹则创建
    Global::CreateWorkPath();

    if(!m_sqldata.connectToSql())
    {
        qDebug() << "数据库连接失败";
        return;
    }

    //创建tcp线程和任务类并连接信号槽后开始监听
    m_TcpTd = new QThread;
    m_TcpTask = new TcpThread;
    connect(this,&MainWindow::StartListen,m_TcpTask,&TcpThread::StartListen);
    connect(m_TcpTask,&TcpThread::NewCltConnected,this,&MainWindow::ServerhasNewConnection);
    connect(m_TcpTask,&TcpThread::CltDisConnected,this,&MainWindow::CltDisConnected);
    m_TcpTask->moveToThread(m_TcpTd);
    m_TcpTd->start();
    emit StartListen();
    ui->PlainTextEdit->appendPlainText(QString("服务器开始监听..."));

    //创建工作线程及任务类
    m_WorkTd = new QThread;
    m_WorkTask = new WorkThread;
    connect(m_TcpTask,&TcpThread::RecvFinished,m_WorkTask,&WorkThread::SplitDataPackAge);
    connect(m_WorkTask,&WorkThread::ThreadbackMsg,this,&MainWindow::getThreadMsg);
    connect(m_WorkTask,&WorkThread::UserOnLine,m_TcpTask,&TcpThread::UserOnLine);
    connect(m_WorkTask,&WorkThread::SendMsgToClt,m_TcpTask,&TcpThread::SendReply);
    m_WorkTask->moveToThread(m_WorkTd);
    m_WorkTd->start();
}

MainWindow::~MainWindow()
{
    m_TcpTd->quit();
    m_TcpTd->wait();
    m_TcpTd->deleteLater();
    m_TcpTask->~TcpThread();
    m_TcpTask->deleteLater();

    m_WorkTd->quit();
    m_WorkTd->wait();
    m_WorkTd->deleteLater();
    m_WorkTask->~WorkThread();
    m_WorkTask->deleteLater();

    delete ui;
}

void MainWindow::ServerhasNewConnection(quint16 port)
{
    ui->PlainTextEdit->appendPlainText(QString("有新客户端连接到服务器,服务器分配的端口号为: " + QString::number(port)));
    //已连接客户端数加一
    ui->Connect->setText(QString::number(ui->Connect->text().toInt() + 1));
}

void MainWindow::CltDisConnected(quint16 port, int acc)
{
    //若已登陆则将在线状态恢复为离线
    QString str = "客户端断开连接，断开连接的客户端端口号为: " + QString::number(port);
    if(acc != 0)
    {
        m_sqldata.ChangeOnlineSta(acc,"离线");
        str += ", 该客户端账号为: " + QString::number(acc);
        ui->Online->setText(QString::number(ui->Online->text().toInt() - 1));
    }
    ui->PlainTextEdit->appendPlainText(str);
    ui->Connect->setText(QString::number(ui->Connect->text().toInt() - 1));
}

void MainWindow::getThreadMsg(int type,int account,QString msg)
{
    //将客户端注册登录相关操作信息显示到服务端界面上
    switch(type)
    {
    case Registration:
    {
        QString res = "用户注册 >>> 账号: " + QString::number(account) + "," + msg;
        ui->PlainTextEdit->appendPlainText(res);
        break;
    }
    case FindPwd:
    {
        if(msg == "")
        {
            QString res = "用户找回密码 >>> 账号: " + QString::number(account) + ",找回密码失败";
            ui->PlainTextEdit->appendPlainText(res);
        }
        else
        {
            QString res = "用户找回密码 >>> 账号: " + QString::number(account) + ",找回密码成功"; //+",密码为:" + msg;
            ui->PlainTextEdit->appendPlainText(res);
        }
        break;
    }
    case LoginAcc:
    {
        QString res = "用户登录 >>> 账号: " + QString::number(account) + "," + msg;
        ui->PlainTextEdit->appendPlainText(res);
        if(msg == "登录成功")
        {
            ui->Online->setText(QString::number(ui->Online->text().toInt() + 1));
        }
        break;
    }
    case ChangeOnlSta:
    {
        QString res = "用户更新在线状态 >>> 账号:" + QString::number(account);
        ui->PlainTextEdit->appendPlainText(res);
        break;
    }
    }
}
