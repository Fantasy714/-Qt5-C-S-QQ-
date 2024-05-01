#include "login.h"
#include "ui_login.h"
#include <QtDebug>
#include <QMenu>
#include <QRegExpValidator>
#include <QGraphicsColorizeEffect>
#include <QThread>
#include <QListWidget>
#include <QMessageBox>
#include <QTimer>
#include <QPainter>
#include <QLabel>
#include <QHBoxLayout>
#include <QDir>

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
{
    ui->setupUi(this);
    //设置无边框窗口,任务栏不显示
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen | Qt::WindowStaysOnTopHint);

    //设置背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    //初始化主界面动态图
    m_movie = new QMovie(":/lib/main.gif");
    m_movie->setScaledSize(QSize(430,130));
    ui->MovieLabel->setMovie(m_movie);
    m_movie->start();

    //设置阴影边框
    QGraphicsDropShadowEffect * shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setOffset(0,0);
    shadow->setColor(Qt::black);
    shadow->setBlurRadius(10);
    ui->frame->setGraphicsEffect(shadow);
    //安装事件过滤器,处理frame的绘画事件
    ui->frame->installEventFilter(this);
    update();

    //初始化用户数据
    initUserData();

    //账户AccountLine只能输入数字
    ui->AccountLine->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    //密码PwdLineEdit不能输入#
    ui->PwdLine->setValidator(new QRegExpValidator(QRegExp("[^#]*")));
    //密码PwdLineEdit输入时显示密码文
    ui->PwdLine->setEchoMode(QLineEdit::Password);

    //设置账号密码栏通过鼠标单击和tab键获取焦点
    ui->AccountLine->setFocus();
    ui->AccountLine->setFocusPolicy(Qt::StrongFocus);
    ui->PwdLine->setFocusPolicy(Qt::StrongFocus);

    //添加文本框图标
    QAction * AcAction = new QAction(ui->AccountLine);
    AcAction->setIcon(QIcon(":/lib/Accont.png"));
    ui->AccountLine->addAction(AcAction,QLineEdit::LeadingPosition);

    QAction * PwdAction = new QAction(ui->PwdLine);
    PwdAction->setIcon(QIcon(":/lib/pwd.png"));
    ui->PwdLine->addAction(PwdAction,QLineEdit::LeadingPosition);
    //初始化系统托盘
    InitSysTrayicon();

    ui->TipsWidget->hide();
    ui->PwdErr->hide();
    ui->shuruacc->hide();
    ui->shurupwd->hide();
}

Login::~Login()
{
    delete m_movie;
    delete ui;
}

void Login::initShadow()
{
    QPainter painter(ui->frame);
    painter.fillRect(ui->frame->rect().adjusted(-10,-10,10,10),QColor(220,220,220));
}

bool Login::eventFilter(QObject *w, QEvent *e)
{
    if((w == ui->frame) && (e->type() == QEvent::Paint))
    {
        initShadow();
        return true;
    }

    return QWidget::eventFilter(w,e);
}

void Login::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        //点下时在控件上
        isMainWidget = true;
        m_point = e->globalPos() - pos();
    }
}

void Login::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton && isMainWidget)
    {
        move(e->globalPos() - m_point);
    }
}

void Login::mouseReleaseEvent(QMouseEvent *event)
{
    //释放时将bool值恢复false
    isMainWidget = false;
    event->accept();
}

//初始化系统托盘
void Login::InitSysTrayicon()
{
    //创建托盘菜单
    m_show = new QAction("打开主面板",this);
    connect(m_show,&QAction::triggered,this,&Login::ShowWindow);

    m_quit = new QAction("退出",this);
    connect(m_quit,&QAction::triggered,this,&Login::on_CloseToolBtn_clicked);
    m_menu = new QMenu(this);

    m_menu->addAction(m_show);
    m_menu->addSeparator();
    m_menu->addAction(m_quit);

    //初始化系统托盘
    QIcon icon(":/lib/System.png");
    m_sysIcon = new QSystemTrayIcon(this);
    m_sysIcon->setIcon(icon);
    m_sysIcon->setToolTip("QQ");
    m_sysIcon->setContextMenu(m_menu);
    connect(m_sysIcon,&QSystemTrayIcon::activated,this,&Login::on_activatedSysTrayIcon);
    m_sysIcon->show();
}

void Login::on_CloseToolBtn_clicked()
{
    //qDebug() << "正常退出";
    emit LoginClose(); //发送关闭信号
    QThread::msleep(25); //等待25毫秒等待线程定时器暂停
    QApplication::quit();
}

void Login::on_MiniToolBtn_clicked()
{
    this->hide();
}

void Login::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger: //单击恢复
        ShowWindow();
        break;
    case QSystemTrayIcon::DoubleClick: //双击恢复
        ShowWindow();
        break;
    default:
        break;
    }
}

void Login::on_FindBtn_clicked()
{
    if(!Global::isConnecting)
    {
        //如果当前未显示提示信息则显示提示五秒钟
        if(ui->TipsWidget->isHidden())
        {
            ui->tipsLabel->setText("登录超时，请检查网络或本机防火墙设置。");
            ui->TipsWidget->show();
            QTimer::singleShot(5000,ui->TipsWidget,SLOT(hide()));
        }
    }
    else
    {
        emit ToAccount(true);
        Registering = true;
    }
}

void Login::on_NewAcBtn_clicked()
{
    if(!Global::isConnecting)
    {
        if(ui->TipsWidget->isHidden())
        {
            ui->tipsLabel->setText("登录超时，请检查网络或本机防火墙设置。");
            ui->TipsWidget->show();
            QTimer::singleShot(5000,ui->TipsWidget,SLOT(hide()));
        }
    }
    else
    {
        emit ToAccount(false);
        Registering = true;
    }
}

void Login::ShowWindow(bool fromAcc)
{
    //判断是否是从注册界面发回的信号
    if(fromAcc == false)
    {
        //如果正在注册则不返回
        if(Registering == true)
        {
            qDebug() << "请从注册/找回窗口返回!";
            return;
        }
        else
        {
            this->show();
        }
    }
    else
    {
        Registering = false;
        this->show();
    }
}

void Login::initUserData()
{
    QDir dir;
    QFile file;
    //查找本地保存账号
    dir.setPath(Global::AppWorkPath());
    QStringList dirNames = dir.entryList(QDir::Dirs);
    dirNames.removeOne(".");
    dirNames.removeOne("..");
    dirNames.removeOne("allusers");
    //若无已保存账号则头像显示默认头像
    if(dirNames.isEmpty())
    {
        QPixmap pix = Global::CreateHeadShot(":/lib/default.jpg");
        ui->label->setPixmap(pix);
    }
    for(auto dirName : dirNames)
    {
        m_Accs.append(dirName);
        file.setFileName(Global::AppWorkPath() + "/" + dirName + "/login.txt");
        //qDebug() << file.fileName() << file.exists();
        if(!file.open(QFile::ReadOnly))
        {
            qDebug() << "打开用户login文件失败";
        }
        QByteArray data = file.readLine();
        file.close();
        QString userMsg = QString(data);
        //逗号拆分用户登录信息
        /*
         * 用户登录文件格式
         * 昵称     头像位置     记住/不记住     记住为密码，不记住则为空
         */
        QStringList Msgs = userMsg.split("##");
        m_Names.append(Msgs.at(0));
        m_Icons.append(Global::AppWorkPath() + "/" + dirName + "/" + Msgs.at(1));
        QString isRem = Msgs.at(2);
        if(isRem == "记住")
        {
            isRemember.append(true);
            m_Pwds.append(Msgs.at(3));
        }
        else
        {
            isRemember.append(false);
            m_Pwds.append("");
        }
    }
    initComboBox();
}

void Login::initComboBox()
{
    //加载自定义的下拉列表框
    QListWidget * lwidget = new QListWidget(this);
    //不获得焦点，删除虚线边框
    lwidget->setFocusPolicy(Qt::NoFocus);
    lwidget->setStyleSheet("QListWidget{outline:0px;}");
    ui->comboBox->setModel(lwidget->model());
    ui->comboBox->setView(lwidget);
    //设置comboBox最大项数
    ui->comboBox->setMaxVisibleItems(3);

    for(int i = 0; i < m_Accs.size(); i++)
    {
        //存放此用户数据的标号
        m_dataLoc.push_back(i);

        QHBoxLayout * layout = new QHBoxLayout;

        //设置头像
        QLabel * label1 = new QLabel;
        QPixmap pix = Global::CreateHeadShot(m_Icons.at(i));
        label1->setPixmap(pix);
        label1->setFixedSize(40,40);
        label1->setScaledContents(true);

        //设置用户名和账号
        QVBoxLayout * vlayout = new QVBoxLayout;
        QLabel * label2 = new QLabel;
        label2->setText(QString("%1").arg(m_Names.at(i)));
        label2->setStyleSheet("QLabel{color:black;font-size:13pt;}");
        QLabel * label3 = new QLabel;
        label3->setText(QString("%2").arg(m_Accs.at(i)));
        label3->setStyleSheet("QLabel{color:grey;font-size:10pt;}");
        vlayout->addWidget(label2);
        vlayout->addWidget(label3);

        //添加删除按钮
        QPushButton * btn = new QPushButton;
        btn->setFixedSize(32,32);
        btn->setStyleSheet("QPushButton{background-color:transparent;border-image:url(:/lib/guanbi.png);}"
                           "QPushButton:hover{background-color:transparent;border-image:url(:/lib/guanbi2.png);}");

        layout->addWidget(label1);
        layout->addLayout(vlayout);
        layout->addWidget(btn);
        QWidget * widget = new QWidget(this);
        widget->setLayout(layout);
        QListWidgetItem * lwitem = new QListWidgetItem(lwidget);
        //绑定删除按钮信号槽
        connect(btn,&QPushButton::clicked,this,[=](){
           switch(QMessageBox::warning(this,"警告","确认要删除该账号吗?",QMessageBox::Ok | QMessageBox::No))
           {
           case QMessageBox::Ok:
           {
              /*
               * 由于此处删除后容器会重新排序,因此如果直接使用标号删除只有第一次删除能够正常删除，
               * 之后标号则会大于容器的大小，不与容器内容相对应了，所以此处将标号放入QVector容器中
               * 查找QVector容器中该标号的位置，用这个位置来删除用户数据，并同步删除QVector中的标号,
               * 才能够与要删除的用户数据同步
               */
               deleteUserData(m_Accs.at(m_dataLoc.indexOf(i)));
               m_Accs.removeAt(m_dataLoc.indexOf(i));
               m_Pwds.removeAt(m_dataLoc.indexOf(i));
               m_Icons.removeAt(m_dataLoc.indexOf(i));
               isRemember.removeAt(m_dataLoc.indexOf(i));
               m_dataLoc.removeOne(i);
               lwidget->removeItemWidget(lwitem);
               delete lwitem;
               //如果将账号全部删除则设置为初始状态
               if(m_dataLoc.length() == 0)
               {
                   QPixmap pix = Global::CreateHeadShot(":/lib/default.jpg");
                   ui->label->setPixmap(pix);
                   ui->AccountLine->setText("");
                   ui->PwdLine->setText("");
                   ui->RememberCheck->setChecked(false);
               }
               break;
           }
           case QMessageBox::No:
               break;
           default:
               break;
           }
        });
        lwidget->setItemWidget(lwitem,widget);
    }
}

void Login::deleteUserData(QString acc)
{
    QString userPath = Global::AppWorkPath() + "/" + acc;
    QDir dir(userPath);
    if(!dir.removeRecursively())
    {
        qDebug() << "删除用户文件失败";
    }
    qDebug() << "若账户中已添加好友则删除本地账号后重新登录会导致好友丢失，"
                "在每次下线检查好友列表是否改动时若有改动则同步向服务器发送改动后的好友列表文件即可解决";
}

void Login::closeSystemIcon()
{
    m_sysIcon->hide();
}

void Login::GetResultFromSer(QString result)
{
    if(result == "账号密码错误")
    {
        qDebug() << "账号密码错误";
        ui->PwdErr->show();
    }
    else if(result == "重复登录")
    {
        qDebug() << "重复登录";
        if(ui->TipsWidget->isHidden())
        {
            qDebug() << "changing tips text";
            ui->tipsLabel->setText("你已在QQ登录了" + ui->AccountLine->text() + "，不能重复登录。");
            ui->TipsWidget->show();
            QTimer::singleShot(5000,ui->TipsWidget,SLOT(hide()));
        }
    }
}

void Login::on_pushButton_clicked()
{
    //获取用户输入账号密码
    QString acc = ui->AccountLine->text();
    QString pwd = ui->PwdLine->text();
    qDebug() << acc << pwd;
    //检查是否输入账号
    if(acc == "")
    {
        if(ui->shuruacc->isHidden())
        {
            ui->shuruacc->show();
            QTimer::singleShot(3000,ui->shuruacc,SLOT(hide()));
        }
        return;
    }
    //检查是否输入密码
    if(pwd == "")
    {
        if(ui->shurupwd->isHidden())
        {
            ui->shurupwd->show();
            QTimer::singleShot(3000,ui->shurupwd,SLOT(hide()));
        }
        return;
    }

    //检查账号密码合法性
    if(acc.length() < 4 || acc.length() > 9)
    {
        QMessageBox::information(this,"提示","账号为4至9位!");
        return;
    }
    if(pwd.length() < 6)
    {
        QMessageBox::information(this,"提示","密码至少6位");
        return;
    }

    //检查是否登录服务器
    if(!Global::isConnecting)
    {
        if(ui->TipsWidget->isHidden())
        {
            ui->tipsLabel->setText("登录超时，请检查网络或本机防火墙设置。");
            ui->TipsWidget->show();
            QTimer::singleShot(5000,ui->TipsWidget,SLOT(hide()));
        }
        return;
    }
    //如果用户数据中无该账号记录则为第一次登录
    if(m_Accs.indexOf(acc) == -1)
    {
        Global::isFirstLogin = true;
    }

    //设置账号密码和是否记住密码
    Global::isRemember = ui->RememberCheck->isChecked();
    Global::InitLoginUserInfo(acc.toInt(),pwd);

    emit LoginToServer();
}

//选择ComboBox时改变账号头像等信息
void Login::on_comboBox_currentIndexChanged(int index)
{
    if(index >= 0)
    {
        //点击已存储账号后设置为非第一次登录
        Global::isFirstLogin = false;

        //qDebug() << index;
        QPixmap pix = Global::CreateHeadShot(m_Icons.at(index));
        ui->label->setPixmap(pix);
        ui->AccountLine->setText(m_Accs.at(index));
        ui->PwdLine->setText(m_Pwds.at(index));
        ui->RememberCheck->setChecked(isRemember.at(index));
    }
}

//忘记密码提示框出现后点击确定返回
void Login::on_pushButton_2_clicked()
{
    ui->PwdErr->hide();
}

//用户改变账号文本栏中文本时，若本地用户数据无该账号记录则显示默认头像,有则显示对应头像
void Login::on_AccountLine_textChanged(const QString &arg1)
{
    //qDebug() << arg1;
    //若用户本地数据中有该账号信息则显示该账号头像，否则显示默认头像
    int loc = m_Accs.indexOf(arg1);
    if(loc == -1)
    {
        QPixmap pix = Global::CreateHeadShot(":/lib/default.jpg");
        ui->label->setPixmap(pix);
    }
    else
    {
        QPixmap pix = Global::CreateHeadShot(m_Icons.at(loc));
        ui->label->setPixmap(pix);
    }
}
