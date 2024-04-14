#include "maininterface.h"
#include "ui_maininterface.h"
#include <QThread>
#include <QFile>
#include <QFileInfo>

MainInterface::MainInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainInterface)
{
    ui->setupUi(this);

    //创建文件夹
    if(!m_dir.exists(m_path))
    {
        qDebug() << "用户数据文件夹未创建";
        if(!m_dir.mkdir(m_path))
        {
            qDebug() << "用户数据文件夹创建失败!";
            return;
        }
    }

    //设置无边框窗口
    setWindowFlags(Qt::FramelessWindowHint);
    //测试

    this->show();
    //测试

    /*
    //初始化登录界面
    m_log = new Login(this);
    connect(m_log,&Login::ToAccount,this,&MainInterface::ShowAccount);
    m_log->show();

    thread = new QThread(this);
    //创建网络连接对象
    m_mytcp = new TcpThread;
    connect(m_mytcp,&TcpThread::isConnectingWithServer,m_log,&Login::isConnectingWithServer);
    connect(m_log,&Login::LoginClose,m_mytcp,&TcpThread::GetClose);
    connect(m_log,&Login::LoginToServer,m_mytcp,&TcpThread::LoginToServer);
    connect(m_mytcp,&TcpThread::sendResultToLogin,m_log,&Login::GetResultFromSer);
    connect(m_mytcp,&TcpThread::sendResultToMainInterFace,this,&MainInterface::GetResultFromSer);
    m_mytcp->moveToThread(thread);

    //连接到服务器
    connect(this,&MainInterface::StartConnecting,m_mytcp,&TcpThread::StartConnect);
    thread->start();
    emit StartConnecting();
    //qDebug() << "主界面的线程ID为:" << QThread::currentThreadId();
    */
}

MainInterface::~MainInterface()
{
    thread->quit();
    thread->wait();
    thread->deleteLater();
    delete m_mytcp;
    delete m_log;
    delete ui;
}

void MainInterface::ShowAccount(bool isfind)
{
    m_acc = Account::GetAccount(isfind);
    connect(m_acc,&Account::BacktoLogin,m_log,&Login::ShowWindow);
    connect(m_acc,&Account::AccountReq,m_mytcp,&TcpThread::recvAccMsg);
    connect(m_mytcp,&TcpThread::isConnectingWithServer,m_acc,&Account::DisConnectedFromSer);
    connect(m_mytcp,&TcpThread::sendResultToAccMsg,m_acc,&Account::recvResultFromTcp);
    m_log->hide();
    m_acc->show();
}

void MainInterface::GetResultFromSer()
{
    m_log->hide();
    this->show();
}

