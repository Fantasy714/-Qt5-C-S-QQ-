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
#include <QMessageBox>
#include <QMetaMethod>

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

    connect(m_FindFri,&FindFriends::SearchingAcc,this,&MainInterface::SearchingAcc);
    connect(this,&MainInterface::SendReplyToFindFri,m_FindFri,&FindFriends::GetReply);
    connect(this,&MainInterface::sendSearchFriMsgToSer,m_mytcp,&TcpThread::sendSearchFriMsgToSer);

    connect(ui->AddFriends,&QToolButton::clicked,this,&MainInterface::ShowFindFri);
    connect(ui->AddFriends_2,&QToolButton::clicked,this,&MainInterface::ShowFindFri);
}

MainInterface::~MainInterface()
{
    thread->quit();
    thread->wait();
    thread->deleteLater();
    for(auto af : m_addfri)
    {
        delete af;
    }
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

void MainInterface::GetResultFromSer(QString type,int acc,QString nickname,QString signature,QString result,QString uData)
{
    if(type == "登录")
    {
        //获取用户信息
        m_account = acc;
        m_headshot = m_path + "/" + QString::number(m_account) + "/" + QString::number(m_account) + ".jpg";
        m_nickname = nickname;
        m_signature = signature;

        //设置登录用户文件夹位置
        m_userpath = m_path + "/" + QString::number(m_account);

        //设置用户头像
        ui->HeadShotBtn->setToolTip("设置个人资料");
        QPixmap pix = CreatePixmap(m_headshot);
        ui->HeadShotBtn->setIcon(QIcon(pix));
        ui->HeadShotBtn->setIconSize(QSize(ui->HeadShotBtn->width(),ui->HeadShotBtn->height()));
        //设置用户昵称签名
        ui->NickNameLab->setText(m_nickname);
        ui->SignatureLine->setText(m_signature);

        //初始化好友列表右键菜单
        InitFriRitBtnMenu();
        //获取好友列表信息
        GetFriendsData();
        //初始化好友列表
        InitTreeWidget();

        //退出登录界面
        m_log->closeSystemIcon();
        m_log->hide();

        //显示主界面
        m_sysIcon->setToolTip("QQ:" + m_nickname + "(" + QString::number(m_account) + ")");
        m_sysIcon->show();
        this->show();
    }
    else if(type == "查找好友")
    {
        if(result == "查找失败")
        {
            emit SendReplyToFindFri(false);
            return;
        }
        else
        {
            QStringList friData = uData.split("@@");
            AddFriend * af = new AddFriend(true,m_groupNames,friData);
            connect(af,&AddFriend::CloseAddFriend,this,&MainInterface::AddFriendClosed);
            af->show();
        }
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
    QFile file(m_userpath + "/friends.json");
    file.open(QFile::ReadOnly);
    QByteArray filedata = file.readAll();
    file.close();

    //将文件中数据读取到存放好友数据的容器中
    QJsonArray jsonarr = QJsonDocument::fromJson(filedata).array();
    for(auto group : jsonarr)
    {
        QString fenzuming = group.toObject().value("name").toString();
        qDebug() << "==========分组: " << fenzuming;
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
            qDebug() << "账号:" << friacc << "昵称:" << frinkname << "个性签名:" << frisig;
        }
    }
}

void MainInterface::InitTreeWidget()
{
    //隐藏列头
    ui->treeWidget->setHeaderHidden(true);
    //设置总列数
    ui->treeWidget->setColumnCount(1);
    //设置头像大小
    ui->treeWidget->setIconSize(QSize(50,50));
    //设置分组item和好友item对齐
    ui->treeWidget->setIndentation(0);
    //右键菜单
    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->treeWidget,&QTreeWidget::itemClicked,this,&MainInterface::onTreeWidgetClicked);
    connect(ui->treeWidget,&QTreeWidget::itemDoubleClicked,this,&MainInterface::onTreeWidgetDoubleClicked);
    connect(ui->treeWidget,&QTreeWidget::itemExpanded,this,&MainInterface::onItemExpended);
    connect(ui->treeWidget,&QTreeWidget::itemCollapsed,this,&MainInterface::onItemCollapsed);
    connect(ui->treeWidget,&QTreeWidget::customContextMenuRequested,this,&MainInterface::FriRightBtnMenu);

    UpdateTreeWidget();
}

void MainInterface::UpdateTreeWidget()
{
    ui->treeWidget->clear();

    for(auto gname : m_groupNames)
    {
        //添加分组
        QTreeWidgetItem * hitem = CreateTreeWidgetItem(gname);
        ui->treeWidget->addTopLevelItem(hitem);

        //添加好友
        QList<int> flist = m_friends.values(gname);
        for(auto f : flist)
        {
            QTreeWidgetItem * fitem = CreateTreeWidgetItem("",f);
            hitem->addChild(fitem);
        }
    }
}

QTreeWidgetItem *MainInterface::CreateTreeWidgetItem(QString fenzuming, int acc)
{
    //如果acc为默认值则传入的是分组名
    if(acc == -1)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(engroup);

        QPixmap pix(":/lib/FLeftArrow.png");
        pix = pix.scaled(12,12);
        item->setIcon(0,QIcon(pix));

        item->setText(0,fenzuming);
        //设置分组行不能选择
        item->setFlags(Qt::ItemIsEnabled);
        return item;
    }
    else
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(enfriend);

        QPixmap pix = CreatePixmap(m_alluserspath + "/" + QString::number(acc) + ".jpg");
        item->setIcon(0,QIcon(pix));

        item->setText(0,QString("%1\n%2").arg(m_frinicknames[acc]).arg(m_frisignatures[acc]));
        //设置好友行行高
        item->setSizeHint(0,QSize(0,60));
        return item;
    }
}

void MainInterface::InitFriRitBtnMenu()
{
    /* 好友列表右键菜单 */
    /*
        QMenu * m_frimenu; //好友右键菜单
        QAction * m_Chat; //与好友聊天
        QAction * m_delete; //删除好友
    */

    m_addgrp = new QAction("添加分组");
    connect(m_addgrp,&QAction::triggered,this,&MainInterface::AddGroup);

    m_renamegrp = new QAction("重命名");
    connect(m_renamegrp,&QAction::triggered,this,&MainInterface::ReNameGroup);

    m_removegrp = new QAction(QIcon(":/lib/delete.png"),"删除分组");
    connect(m_removegrp,&QAction::triggered,this,&MainInterface::RemoveGroup);

    m_grpmenu = new QMenu(this);
    m_grpmenu->addAction(m_addgrp);
    m_grpmenu->addAction(m_renamegrp);
    m_grpmenu->addAction(m_removegrp);
}

void MainInterface::SearchingAcc(QString acc)
{
    int searchAcc = acc.toInt();
    //若查找结果为空则不存在该好友，否则存在
    bool isFri = m_frinicknames.value(searchAcc) == "" ? false : true;
    //若已存在该好友或查找账号为自己则直接返回结果
    if(isFri || m_account == searchAcc)
    {
        qDebug() << searchAcc;
        emit SendReplyToFindFri(true);
        return;
    }
    emit sendSearchFriMsgToSer(searchAcc);
}

void MainInterface::AddFriendClosed(QString type,int acc,QString GpNa)
{
    if(type == "完成")
    {

    }
    //获得该好友界面指针并删除该指针
    AddFriend * af = (AddFriend*)sender();
    m_addfri.removeOne(af);
    delete af;
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

void MainInterface::onTreeWidgetClicked(QTreeWidgetItem *item)
{
    //如果单击项为分组项，则展开或收起
    if(item->type() == engroup)
    {
        if(item->isExpanded())
        {
            ui->treeWidget->collapseItem(item);
        }
        else
        {
            ui->treeWidget->expandItem(item);
        }
    }
}

void MainInterface::onTreeWidgetDoubleClicked(QTreeWidgetItem *item)
{
    if(item->type() == enfriend)
    {
        qDebug() << "准备打开聊天界面";
    }
}

void MainInterface::onItemExpended(QTreeWidgetItem *item)
{
    QPixmap pix(":/lib/FDownArrow.png");
    pix = pix.scaled(12,12);
    item->setIcon(0,QIcon(pix));
}

void MainInterface::onItemCollapsed(QTreeWidgetItem *item)
{
    QPixmap pix(":/lib/FLeftArrow.png");
    pix = pix.scaled(12,12);
    item->setIcon(0,QIcon(pix));
}

void MainInterface::FriRightBtnMenu(const QPoint &pos)
{
    QTreeWidgetItem * sitem = nullptr;
    sitem = ui->treeWidget->itemAt(pos);
    if(sitem)
    {
        //右键点击分组
        if(sitem->type() == engroup)
        {
            //若为默认分组则不可改名和删除
            if(sitem->text(0) == "我的好友")
            {
                m_renamegrp->setEnabled(false);
                m_removegrp->setEnabled(false);
            }
            else if(!m_renamegrp->isEnabled())
            {
                m_renamegrp->setEnabled(true);
                m_removegrp->setEnabled(true);
            }

            //将上一个右键点击的分组设置为不可编辑
            /*
             * 用户若在重命名分组未更改时直接右键点击其他分组名那么上一个
             * 分组便依然为可编辑状态，需在此将上一个分组设置回不可编辑
             */
            if(m_chaggrpitem != nullptr)
            {
                m_chaggrpitem->setFlags(Qt::ItemIsEnabled);
            }

            qDebug() << "233";
            m_chaggrpitem = sitem;
            m_chagName = sitem->text(0);

            m_grpmenu->exec(QCursor::pos());
        }
        //右键点击好友
        else
        {

        }
    }
}

void MainInterface::AddGroup()
{
    //是新增分组
    isaddgrp = true;

    QTreeWidgetItem * item = new QTreeWidgetItem(engroup);

    QPixmap pix(":/lib/FLeftArrow.png");
    pix = pix.scaled(12,12);
    item->setIcon(0,QIcon(pix));

    //设置分组行可编辑
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    ui->treeWidget->addTopLevelItem(item);

    //编辑该新建分组
    ui->treeWidget->editItem(item);
    connect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainInterface::ItemNameChanged);
}

void MainInterface::ReNameGroup()
{
    //将该项设置为可编辑
    m_chaggrpitem->setFlags(m_chaggrpitem->flags() | Qt::ItemIsEditable);
    //qDebug() << m_chaggrpitem->flags();

    //编辑该分组
    ui->treeWidget->editItem(m_chaggrpitem,0);
    //不是添加分组
    isaddgrp = false;
    connect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainInterface::ItemNameChanged);
}

void MainInterface::RemoveGroup()
{
    switch(QMessageBox::warning(this,"警告","选定的分组将被删除,组内联系人将回移至系统默认分组‘我的好友’,你确认要删除该分组吗?",QMessageBox::Ok | QMessageBox::No))
    {
    case QMessageBox::Ok:
        {
            //取出该分组下好友后删除该分组
            m_groupNames.removeOne(m_chagName);
            QList<int> moveList = m_friends.values(m_chagName);
            m_friends.remove(m_chagName);
            //该分组下无好友则直接删除并将更改分组item置空
            if(moveList.size() <= 0)
            {
                delete m_chaggrpitem;
                m_chaggrpitem = nullptr;
            }
            //有好友需将好友迁移到默认分组后更新好友列表
            else
            {
                for(auto fri : moveList)
                {
                    m_friends.insert("我的好友",fri);
                }
                delete m_chaggrpitem;
                m_chaggrpitem = nullptr;
                UpdateTreeWidget();
            }
            break;
        }
    case QMessageBox::No:
        break;
    default:
        break;
    }
}

void MainInterface::ItemNameChanged(QTreeWidgetItem *item)
{
    //qDebug() << "更改...";
    //获取当前输入的分组名
    QString gn = item->text(0);
    //添加分组
    if(isaddgrp)
    {
        //查找是否已经存在该分组名
        int index = m_groupNames.indexOf(gn);
        if(index != -1 || gn == "")
        {
            QMessageBox::warning(this,"警告","分组名重复或为空，请重新创建!");
            delete item;
        }
        else
        {
            //未重复则将该分组名添加进分组名容器中
            m_groupNames.append(gn);
            //好友列表已更改
            FriIsChanged = true;
            item->setFlags(Qt::ItemIsEnabled);
        }
        disconnect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainInterface::ItemNameChanged);
    }
    //重命名
    else
    {
        /*
         * 用户若在重命名分组未更改时直接左键点击其他分组名那么上一个
         * 分组便依然为可编辑状态，需在此将上一个分组设置回不可编辑
        */
        if(item != m_chaggrpitem)
        {
            qDebug() << "用户取消更改后点击其他item";
            disconnect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainInterface::ItemNameChanged);
            m_chaggrpitem->setFlags(Qt::ItemIsEnabled);
            return;
        }
        /*
         * 用户若在重命名分组未更改时直接左键或右键点击当前分组名则判断该if语句，
         * 分组名未改变则直接执行下面的解除信号槽绑定和恢复当前分组不可编辑
         */
        //如果名称未改变则不更改
        if(m_chagName != gn)
        {
            //查找是否已经存在该分组名
            int index = m_groupNames.indexOf(gn);
            if(index != -1)
            {
                QMessageBox::warning(this,"警告","分组名重复，请重新更改!");
                item->setText(0,m_chagName);
            }
            else
            {
                //获取原分组位置和原分组下好友，之后删除原分组，再将原分组名下好友迁移至新分组名内
                int oldindex = m_groupNames.indexOf(m_chagName);
                m_groupNames.removeOne(m_chagName);
                QList<int> flist = m_friends.values(m_chagName);
                m_friends.remove(m_chagName);

                m_groupNames.insert(oldindex,gn);
                for(auto f : flist)
                {
                    m_friends.insert(gn,f);
                }

                qDebug() << "已完成重命名";
            }
        }
        disconnect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainInterface::ItemNameChanged);
        item->setFlags(Qt::ItemIsEnabled);
    }
}
