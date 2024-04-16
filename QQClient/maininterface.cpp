#include "maininterface.h"
#include "ui_maininterface.h"
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QAction>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

MainInterface::MainInterface(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainInterface)
{
    ui->setupUi(this);

    //创建用户本地数据文件夹
    if(!m_dir.exists(m_path))
    {
        qDebug() << "用户数据文件夹未创建";
        if(!m_dir.mkdir(m_path))
        {
            qDebug() << "用户数据文件夹创建失败!";
            return;
        }
    }

    QString allusers = m_path + "/allusers";
    if(!m_dir.exists(allusers))
    {
        qDebug() << "allusers数据文件夹未创建";
        if(!m_dir.mkdir(allusers))
        {
            qDebug() << "allusers数据文件夹创建失败!";
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
    m_accClass = Account::GetAccount(isfind);
    connect(m_accClass,&Account::BacktoLogin,m_log,&Login::ShowWindow);
    connect(m_accClass,&Account::AccountReq,m_mytcp,&TcpThread::recvAccMsg);
    connect(m_mytcp,&TcpThread::isConnectingWithServer,m_accClass,&Account::DisConnectedFromSer);
    connect(m_mytcp,&TcpThread::sendResultToAccMsg,m_accClass,&Account::recvResultFromTcp);
    m_log->hide();
    m_accClass->show();
}

void MainInterface::GetResultFromSer(QString type,int acc,QString nickname,QString signature)
{
    if(type == "登录")
    {
        //设置登录用户文件夹位置
        m_userpath = m_path + "/" + QString::number(m_account);
        //获取用户信息
        m_account = acc;
        m_headshot = m_path + "/" + QString::number(m_account) + "/" + QString::number(m_account) + ".jpg";
        m_nickname = nickname;
        m_signature = signature;

        //设置用户头像
        ui->HeadShotBtn->setToolTip("设置个人资料");
        QPixmap pix = CreatePixmap(m_headshot);
        ui->HeadShotBtn->setIcon(QIcon(pix));
        ui->HeadShotBtn->setIconSize(QSize(ui->HeadShotBtn->width(),ui->HeadShotBtn->height()));
        //设置用户昵称签名
        ui->NickNameLab->setText(m_nickname);
        ui->SignatureLine->setText(m_signature);

        //退出登录界面
        m_log->closeSystemIcon();
        m_log->hide();

        //显示主界面
        m_sysIcon->setToolTip("QQ:" + m_nickname + "(" + QString::number(m_account) + ")");
        m_sysIcon->show();
        this->show();
    }
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

QPixmap MainInterface::CreatePixmap(QString picPath)
{
    QPixmap src(picPath);
    QPixmap pix(src.width(),src.height());

    //设置图片透明
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    //设置图片边缘抗锯齿，指示引擎应使用平滑像素图变换算法绘制图片
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    //设置圆形半径，取图片较小边长作为裁切半径
    int radius = src.width() > src.height() ? src.height()/2 : src.width()/2;
    //绘制裁切区域的大小
    path.addEllipse(src.rect().center(),radius,radius);
    //设置裁切区域
    painter.setClipPath(path);
    //把源图片的内容绘制到创建的pixmap上，非裁切区域内容不显示
    painter.drawPixmap(pix.rect(),src);

    return pix;
}

void MainInterface::GetFriendsData()
{
    //读取用户本地存放好友的json文件
    QFile file(m_path + "/friends.json");
    file.open(QFile::ReadOnly);
    QByteArray filedata = file.readAll();
    file.close();

    //将文件中数据读取到存放好友数据的容器中
    QJsonArray jsonarr = QJsonDocument::fromJson(filedata).array();
    for(auto group : jsonarr)
    {
        QString fenzuming = group.toObject().value("name").toString();
        m_groupNames.append(fenzuming);
        QJsonArray arr = group.toObject().value("friends").toArray();
        for(auto fri : arr)
        {
            //将好友数据拆分开
            QString frimsg = fri.toString();
            int friacc = frimsg.split("@@").at(0).toInt();
            QString frinkname = frimsg.split("@@").at(1);
            QString frisig = frimsg.split("@@").at(2);

            //将数据放入各容器中
            m_friends.insert(fenzuming,friacc);
            m_frinicknames.insert(friacc,frinkname);
            m_frisignatures.insert(friacc,frisig);
        }
    }
}

void MainInterface::UpdateTreeWidget()
{

}

QTreeWidgetItem *MainInterface::CreateTreeWidgetItem(QString fenzuming, int acc)
{
    //如果acc为默认值则传入的是分组名
    if(acc == -1)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(engroup);
        item->setText(0,fenzuming);
        return item;
    }
    else
    {

    }
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
