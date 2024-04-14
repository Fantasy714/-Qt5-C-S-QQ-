#include "maininterface.h"
#include "ui_maininterface.h"
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QAction>

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
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen | Qt::WindowStaysOnTopHint);

    //添加搜索图标
    QAction * searchAc = new QAction(ui->SearchEdit);
    searchAc->setIcon(QIcon(":/lib/search.png"));
    ui->SearchEdit->addAction(searchAc,QLineEdit::LeadingPosition);

    //初始化登录界面
    m_log = new Login(this);
    connect(m_log,&Login::ToAccount,this,&MainInterface::ShowAccount);
    m_log->show();

    thread = new QThread(this);
    //创建网络连接对象
    m_mytcp = new TcpThread;
    connect(this,&MainInterface::MainInterfaceClose,m_mytcp,&TcpThread::GetClose);
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

    //初始化系统托盘
    initSystemIcon();

    //初始化查找好友窗口
    m_FindFri = FindFriends::createFindFriends();

    connect(ui->AddFriends,&QToolButton::clicked,this,&MainInterface::ShowFindFri);
    connect(ui->AddFriends_2,&QToolButton::clicked,this,&MainInterface::ShowFindFri);
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

void MainInterface::GetResultFromSer(int acc,QString nickname,QString signature)
{
    //获取用户信息
    m_account = acc;
    m_headshot = m_path + "/" + QString::number(m_account) + "/" + QString::number(m_account) + ".jpg";
    m_nickname = nickname;
    m_signature = signature;

    //初始化用户头像昵称签名
    ui->HeadShotBtn->setIcon(QIcon(m_headshot));
    ui->HeadShotBtn->setIconSize(QSize(ui->HeadShotBtn->width(),ui->HeadShotBtn->height()));
    ui->HeadShotBtn->setMask(QRegion(ui->HeadShotBtn->rect(),QRegion::RegionType::Ellipse));
    ui->NickNameLab->setText(m_nickname);
    ui->SignatureLine->setText(m_signature);
    m_log->closeSystemIcon();
    m_log->hide();
    m_sysIcon->setToolTip("QQ:" + m_nickname + "(" + QString::number(m_account) + ")");
    m_sysIcon->show();
    this->show();
}

void MainInterface::initSystemIcon()
{
    //创建托盘菜单
    m_show = new QAction("打开主面板",this);
    connect(m_show,&QAction::triggered,this,&MainInterface::showNormal);

    m_quit = new QAction("退出",this);
    connect(m_quit,&QAction::triggered,this,&QApplication::quit);
    m_menu = new QMenu(this);
    m_menu->addAction(m_show);

    m_menu->addSeparator();
    m_menu->addAction(m_quit);

    //初始化系统托盘
    QIcon icon(":/lib/QQ.png");
    m_sysIcon = new QSystemTrayIcon(this);
    m_sysIcon->setIcon(icon);
    m_sysIcon->setContextMenu(m_menu);
    connect(m_sysIcon,&QSystemTrayIcon::activated,this,&MainInterface::on_activatedSysTrayIcon);
    m_sysIcon->show();
}

void MainInterface::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger: //单击恢复
        this->show();
        break;
    case QSystemTrayIcon::DoubleClick: //双击恢复
        this->show();
        break;
    default:
        break;
    }
}

void MainInterface::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        isMainWidget = true;
        m_point = e->globalPos() - pos();
    }
}

void MainInterface::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton && isMainWidget)
    {
        move(e->globalPos() - m_point);
    }
}

void MainInterface::mouseReleaseEvent(QMouseEvent *event)
{
    //释放时将bool值恢复false
    isMainWidget = false;
    event->accept();
}

void MainInterface::ShowFindFri()
{
    m_FindFri->show();
}

void MainInterface::on_CloseBtn_clicked()
{
    qDebug() << "正常退出";
    emit MainInterfaceClose(); //发送关闭信号
    QThread::msleep(25); //等待25毫秒等待线程定时器暂停
    QApplication::quit();
}

void MainInterface::on_MiniBtn_clicked()
{
    this->hide();
}
