#include "maininterface.h"
#include "ui_maininterface.h"
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QAction>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMessageBox>
#include <QMetaMethod>
#include <QTimer>
#include <QScrollBar>

#define Margin 6

MainInterface::MainInterface(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainInterface)
{
    ui->setupUi(this);

    //创建用户数据目录
    Global::CreateWorkPath();

    //设置无边框窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen | Qt::WindowStaysOnTopHint);

    //设置背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    //初始化全局路径，全屏闭合回路
    m_pixmap = QPixmap(this->width(),this->height());
    m_globalPath.lineTo(m_pixmap.width(), 0);
    m_globalPath.lineTo(m_pixmap.width(), m_pixmap.height());
    m_globalPath.lineTo(0, m_pixmap.height());
    m_globalPath.lineTo(0, 0);

    //设置阴影边框
    QGraphicsDropShadowEffect * shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setOffset(0,0);
    shadow->setColor(Qt::black);
    shadow->setBlurRadius(10);
    ui->frame->setGraphicsEffect(shadow);

    //设置鼠标追踪
    setMouseTracking(true);

    //初始化tabwidget宽度
    TabWidgetWidth = ui->tabWidget->width();

    //添加搜索图标
    QAction * searchAc = new QAction(ui->SearchEdit);
    searchAc->setIcon(QIcon(":/lib/search.png"));
    ui->SearchEdit->addAction(searchAc,QLineEdit::LeadingPosition);

    //安装事件过滤器，处理绘画事件
    ui->frame->installEventFilter(this);
    update();

    //初始化登录界面
    m_log = new Login(this);
    connect(m_log,&Login::ToAccount,this,&MainInterface::ShowAccount);
    m_log->show();

    thread = new QThread;
    //创建网络连接对象
    m_mytcp = new TcpThread;

    connect(m_log,&Login::LoginClose,m_mytcp,&TcpThread::GetClose);
    connect(m_log,&Login::LoginToServer,m_mytcp,&TcpThread::LoginToServer);
    connect(m_mytcp,&TcpThread::sendResultToLogin,m_log,&Login::GetResultFromSer);
    connect(this,&MainInterface::SendMsgToServer,m_mytcp,&TcpThread::MsgToJson);
    connect(this,&MainInterface::MainInterfaceClose,m_mytcp,&TcpThread::GetClose);
    connect(m_mytcp,&TcpThread::isConnectingWithServer,this,&MainInterface::Reconnection);
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

    connect(ui->AddFriends,&QToolButton::clicked,this,&MainInterface::ShowFindFri);
    connect(ui->AddFriends_2,&QToolButton::clicked,this,&MainInterface::ShowFindFri);

    ui->treeWidget->verticalScrollBar()->setStyleSheet(Global::scrollbarStyle);
}

MainInterface::~MainInterface()
{
    //如果好友列表有改动则更新好友列表本地文件
    if(FriIsChanged)
    {
        //获取好友信息
        QJsonArray jsonArr;
        for(auto groupN : m_groupNames)
        {
            QJsonObject obj;
            obj.insert("name",groupN);
            QJsonArray arr;
            QList<int> FriAccs = m_friends.values(groupN);
            for(auto fri : FriAccs)
            {
                QString fridata = QString::number(fri) + "@@" + m_frinicknames[fri] + "@@" + m_frisignatures[fri];
                arr.append(fridata);
            }
            obj.insert("friends",arr);
            jsonArr.append(obj);
        }
        QJsonDocument doc(jsonArr);

        //存入本地用户文件中
        QFile file(Global::UserPath() + "/friends.json");
        file.open(QFile::WriteOnly | QFile::Truncate);
        file.write(doc.toJson());
        file.close();
    }
    thread->quit();
    thread->wait();
    thread->deleteLater();
    m_mytcp->deleteLater();
    m_log->deleteLater();
    m_editData->deleteLater();
    for(auto chat : m_chatWindows)
    {
        chat->deleteLater();
    }
    for(auto pdata : m_PersonData)
    {
        pdata->deleteLater();
    }
    for(auto af : m_addfri)
    {
        af->deleteLater();
    }
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

void MainInterface::GetResultFromSer(int type,int targetAcc,QString uData,QString Msg,QString MsgType)
{
    switch(type)
    {
    case LoginAcc:
    {
        //标记为已登录
        isLogined = true;
        m_loc = Center;

        //设置用户头像
        ui->HeadShotBtn->setToolTip("设置个人资料");
        QPixmap pix = Global::CreateHeadShot(Global::UserHeadShot());
        ui->HeadShotBtn->setIcon(QIcon(pix));
        ui->HeadShotBtn->setIconSize(QSize(ui->HeadShotBtn->width() - 5,ui->HeadShotBtn->height() - 5));
        //设置用户昵称签名
        ui->NickNameLab->setText(Global::UserNickName());
        ui->SignatureLine->setText(Global::UserSignature());

        //获取好友列表信息
        GetFriendsData();
        //初始化好友列表
        InitTreeWidget();
        //初始化好友列表右键菜单
        InitFriRitBtnMenu();

        //显示好友列表
        ui->tabWidget->setCurrentIndex(2);

        //退出登录界面
        m_log->closeSystemIcon();
        m_log->hide();

        //显示主界面
        m_sysIcon->setToolTip("QQ:" + Global::UserNickName() + "(" + QString::number(Global::UserAccount()) + ")");
        m_sysIcon->show();
        this->show();
        break;
    }
    case SearchFri:
    {
        if(MsgType == "查找失败")
        {
            emit SendReplyToFindFri(false);
            return;
        }
        else
        {
            AddFriend * af = new AddFriend(true,m_groupNames,uData.split("@@"));
            connect(af,&AddFriend::CloseAddFriend,this,&MainInterface::AddFriendClosed);
            m_addfri.append(af);
            af->show();
        }
        break;
    }
    case AddFri:
    {
        if(MsgType == "该好友已下线")
        {
            m_waitFriReply.remove(targetAcc);
            QMessageBox::information(this,"提示",QString("添加/删除好友%1失败,该用户已下线").arg(targetAcc));
        }
        else
        {
            if(MsgType == "发送好友申请")
            {
                qDebug() << "收到好友申请";
                AddFriend * af = new AddFriend(false,m_groupNames,uData.split("@@"),Msg);
                connect(af,&AddFriend::CloseAddFriend,this,&MainInterface::AddFriendClosed);
                m_addfri.append(af);
                af->show();
            }
            else if(MsgType == "成功删除好友")
            {
                qDebug() << "删除好友" << targetAcc << "中...";
                QString gpN = "";
                for(auto g : m_groupNames)
                {
                    qDebug() << g;
                    QList<int> fris = m_friends.values(g);
                    for(auto f : fris)
                    {
                        if(f == targetAcc)
                        {
                            qDebug() << "找到该好友，该好友在" << g << "分组";
                            gpN = g;
                            break;
                        }
                    }
                    //若已找到则退出循环
                    if(gpN != "")
                    {
                        break;
                    }
                }
                //删除该好友
                m_friends.remove(gpN,targetAcc);
                m_frinicknames.remove(targetAcc);
                m_frisignatures.remove(targetAcc);

                //已更改好友列表
                FriIsChanged = true;

                //更新好友列表
                UpdateTreeWidget();
            }
        }
        break;
    }
    case SendMsg:
    {
        if(MsgType == "发送图片")
        {
            ChatWindow * ctw = showFriChatWindow(targetAcc);
            ctw->FriendSendMsg(false,itsPicture,Msg);
            return;
        }
        else if(MsgType == "添加好友成功")
        {
            //好友列表已更改
            FriIsChanged = true;
            QString gn = m_waitFriReply[targetAcc];
            m_friends.insert(gn,targetAcc);
            m_frinicknames.insert(targetAcc,uData.split("@@").at(Dnickname));
            m_frisignatures.insert(targetAcc,uData.split("@@").at(Dsignature));
            UpdateTreeWidget();
        }

        //若未创建聊天信息框则创建
        ChatWindow* ctw = showFriChatWindow(targetAcc);
        ctw->FriendSendMsg(false,itsMsg,Msg);
        break;
    }
    case AskForData:
    {
        if(MsgType == "请求自己的")
        {
            QStringList Datas = uData.split("@@");
            PersonalData * pd = new PersonalData(true,Global::UserAccount(),Datas);
            connect(pd,&PersonalData::ClosePerData,this,&MainInterface::ClosePerData);
            connect(pd,&PersonalData::ChangingData,this,&MainInterface::EditPersonalData);
            connect(pd,&PersonalData::ChangingHeadShot,this,&MainInterface::ChangingHeadShot);
            m_PersonData.insert(Global::UserAccount(),pd);
            pd->show();
        }
        else
        {
            QString fnkn = uData.split("@@").at(Dnickname);
            QString fsig = uData.split("@@").at(Dsignature);

            qDebug() << Msg << fnkn << fsig;

            //显示用户资料
            QStringList Datas = uData.split("@@");
            PersonalData * pd = new PersonalData(false,targetAcc,Datas);
            connect(pd,&PersonalData::ClosePerData,this,&MainInterface::ClosePerData);
            m_PersonData.insert(targetAcc,pd);
            pd->show();

            //有任何更改则更新好友列表
            if(Msg != "不更新头像" || m_frinicknames[targetAcc] != fnkn || m_frisignatures[targetAcc] != fsig)
            {
                //更新好友列表
                FriIsChanged = true;
                if(m_frinicknames[targetAcc] != fnkn)
                {
                    m_frinicknames[targetAcc] = fnkn;
                }
                if(m_frisignatures[targetAcc] != fsig)
                {
                    m_frisignatures[targetAcc] = fsig;
                }

                UpdateTreeWidget();
            }
        }
        break;
    }
    case SendFileToFri:
    {
        if(MsgType == "发送文件")
        {
            ChatWindow * ctw = showFriChatWindow(targetAcc);
            ctw->SendOrRecvFile(false,Msg);
            return;
        }
        break;
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
        isPressed = true;
        //获取鼠标点下位置相对于窗口左上角的偏移量
        m_point = e->globalPos() - this->frameGeometry().topLeft();
    }
}

void MainInterface::mouseMoveEvent(QMouseEvent *e)
{
    //鼠标未按下则设置鼠标状态
    if(!isPressed)
    {
        ChangeCurSor(e->pos());
    }
    else
    {
        QPoint glbPos = e->globalPos();
        //若在未位于边框上则移动窗口
        if(e->buttons() & Qt::LeftButton && m_loc == Center)
        {
            move(glbPos - m_point);
            return;
        }

        //获取当前窗口左上角和右下角
        QPoint topLeft = this->frameGeometry().topLeft();
        QPoint BottomRight = this->frameGeometry().bottomRight();

        QRect cRect(topLeft,BottomRight);

        switch(m_loc)
        {
        case Top:
            //如果拖动窗口时底部y坐标减去当前鼠标y坐标已经小于窗口最小高度则不移动，继续移动不会更改窗口大小会推动窗口向下移动，下同
            if(BottomRight.y() - glbPos.y() > this->minimumHeight())
            {
                cRect.setY(glbPos.y());
            }
            break;
        case Bottom:
            cRect.setHeight(glbPos.y() - topLeft.y());
            break;
        case Left:
            if(BottomRight.x() - glbPos.x() < this->maximumWidth() && BottomRight.x() - glbPos.x() > this->minimumWidth())
            {
                cRect.setX(glbPos.x());
            }
            break;
        case Right:
            cRect.setWidth(glbPos.x() - topLeft.x());
            break;
        case Top_Left:
            if(BottomRight.y() - glbPos.y() > this->minimumHeight())
            {
                cRect.setY(glbPos.y());
            }
            if(BottomRight.x() - glbPos.x() < this->maximumWidth() && BottomRight.x() - glbPos.x() > this->minimumWidth())
            {
                cRect.setX(glbPos.x());
            }
            break;
        case Top_Right:
            if(BottomRight.y() - glbPos.y() > this->minimumHeight())
            {
                cRect.setY(glbPos.y());
            }
            cRect.setWidth(glbPos.x() - topLeft.x());
            break;
        case Bottom_Left:
            if(BottomRight.x() - glbPos.x() < this->maximumWidth() && BottomRight.x() - glbPos.x() > this->minimumWidth())
            {
                cRect.setX(glbPos.x());
            }
            cRect.setHeight(glbPos.y() - topLeft.y());
            break;
        case Bottom_Right:
            cRect.setHeight(glbPos.y() - topLeft.y());
            cRect.setWidth(glbPos.x() - topLeft.x());
            break;
        default:
            break;
        }
        this->setGeometry(cRect);
    }

    //若拖动后tabWidget宽度更改则同步更改tab的宽度
    if(ui->tabWidget->width() != TabWidgetWidth)
    {
        ui->tabWidget->setStyleSheet(QString("QTabBar::tab{font:20px;margin-left:35px;width:%1px;margin-right:35px;height:30px;border-style:solid;border-top-width:0px;border-left-width:0px;"
                                             "border-right-width:0px;border-bottom-width:1px;border-color:rgb(200,200,200);background-color:white;}QTabBar::tab:selected"
                                             "{border-bottom-width:3px;border-color:rgb(40,192,253);}QWidget{background-color:white;}").arg(ui->tabWidget->width() / 2 - 70));
        TabWidgetWidth = ui->tabWidget->width();
    }
}

void MainInterface::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    //释放时将bool值恢复false,鼠标恢复默认状态
    setCursor(QCursor(Qt::ArrowCursor));
    m_loc = Center;
    isPressed = false;
}

void MainInterface::ShowFindFri()
{
    m_FindFri->show();
}

void MainInterface::ChangeCurSor(const QPoint &p)
{
    //获取当前鼠标在窗口中的位置
    int x = p.x();
    int y = p.y();

    //获取当前位置与窗口最右侧及最下方的距离
    int fromRight = this->frameGeometry().width() - x;
    int fromBottom = this->frameGeometry().height() - y;

    //若当前位置x,y坐标都小于Margin距离则在左上角
    if(x < Margin && y < Margin)
    {
        m_loc = Top_Left;
        setCursor(QCursor(Qt::SizeFDiagCursor));
    }
    //在上边
    else if(x > Margin && fromRight > Margin && y < Margin)
    {
        m_loc = Top;
        setCursor(QCursor(Qt::SizeVerCursor));
    }
    //右上角
    else if(fromRight < Margin && y < Margin)
    {
        m_loc = Top_Right;
        setCursor(QCursor(Qt::SizeBDiagCursor));
    }
    //右边
    else if(fromRight < Margin && y > Margin && fromBottom > Margin)
    {
        m_loc = Right;
        setCursor(QCursor(Qt::SizeHorCursor));
    }
    //右下角
    else if(fromRight < Margin && fromBottom < Margin)
    {
        m_loc = Bottom_Right;
        setCursor(QCursor(Qt::SizeFDiagCursor));
    }
    //下边
    else if(fromBottom < Margin && x > Margin && fromRight > Margin)
    {
        m_loc = Bottom;
        setCursor(QCursor(Qt::SizeVerCursor));
    }
    //左下角
    else if(fromBottom < Margin && x < Margin)
    {
        m_loc = Bottom_Left;
        setCursor(QCursor(Qt::SizeBDiagCursor));
    }
    //左边
    else if(x < Margin && y > Margin && fromBottom > Margin)
    {
        m_loc = Left;
        setCursor(QCursor(Qt::SizeHorCursor));
    }
    //否则位于中心界面
    else
    {
        m_loc = Center;
        setCursor(QCursor(Qt::ArrowCursor));
    }
}

bool MainInterface::eventFilter(QObject *w, QEvent *e)
{
    if((w == ui->frame) && (e->type() == QEvent::Paint))
    {
        initShadow();
        return true;
    }

    return QWidget::eventFilter(w,e);
}

void MainInterface::GetFriendsData()
{
    //读取用户本地存放好友的json文件
    QFile file(Global::UserPath() + "/friends.json");
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
    //不获得焦点
    ui->treeWidget->setFocusPolicy(Qt::NoFocus);

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
        item->setSizeHint(0,QSize(ui->treeWidget->width(),36));

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

        item->setData(0,Qt::UserRole,acc);

        QPixmap pix = Global::CreateHeadShot(Global::AppAllUserPath() + "/" + QString::number(acc) + ".jpg");
        item->setIcon(0,QIcon(pix));

        item->setText(0,QString("%1\n%2").arg(m_frinicknames[acc]).arg(m_frisignatures[acc]));
        //设置提示
        item->setToolTip(0,QString("%1(%2)").arg(m_frinicknames[acc]).arg(acc));
        //设置好友行行高
        item->setSizeHint(0,QSize(0,60));
        return item;
    }
}

void MainInterface::InitFriRitBtnMenu()
{
    //初始化好友菜单
    m_Chat = new QAction(QIcon(":/lib/Chat.png"),"发送即时信息",this);
    connect(m_Chat,&QAction::triggered,this,[=](){
        if(m_friItem)
        {
            int acc = m_friItem->data(0,Qt::UserRole).toInt();
            showFriChatWindow(acc);
        }
    });

    m_FriData = new QAction("查看资料",this);
    connect(m_FriData,&QAction::triggered,[=](){
       int acc = m_friItem->data(0,Qt::UserRole).toInt();
       emit SendMsgToServer(AskForData,acc,-1,"","请求好友的");
    });

    m_moveFri = new QMenu(this);
    m_moveFri->setTitle("移动联系人至");
    //初始化移动好友菜单
    for(auto gN : m_groupNames)
    {
        QAction * act = new QAction(gN,this);
        connect(act,&QAction::triggered,this,&MainInterface::MoveFriend);
        m_movegroups.insert(gN,act);
        m_moveFri->addAction(act);
    }

    m_delete = new QAction(QIcon(":/lib/delete.png"),"删除好友",this);
    connect(m_delete,&QAction::triggered,this,&MainInterface::DelFri);

    m_frimenu = new QMenu(this);
    m_frimenu->addAction(m_Chat);
    m_frimenu->addAction(m_FriData);
    m_frimenu->addMenu(m_moveFri);
    m_frimenu->addAction(m_delete);

    //初始化分组菜单
    m_addgrp = new QAction("添加分组",this);
    connect(m_addgrp,&QAction::triggered,this,&MainInterface::AddGroup);

    m_renamegrp = new QAction("重命名",this);
    connect(m_renamegrp,&QAction::triggered,this,&MainInterface::ReNameGroup);

    m_removegrp = new QAction(QIcon(":/lib/delete.png"),"删除分组",this);
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
    if(isFri || Global::UserAccount() == searchAcc)
    {
        qDebug() << searchAcc;
        emit SendReplyToFindFri(true);
        return;
    }
    emit SendMsgToServer(SearchFri,-1,searchAcc);
}

void MainInterface::AddFriendClosed(QString type,int targetacc,QString GpNa,QString yanzheng)
{
    if(type == "完成")
    {
        //将该好友账号和分组信息记录下来
        m_waitFriReply.insert(targetacc,GpNa);
        emit SendMsgToServer(AddFri,Global::UserAccount(),targetacc,yanzheng,"发送好友申请");
    }
    else if(type == "同意")
    {
        //将该好友账号和分组信息记录下来
        m_waitFriReply.insert(targetacc,GpNa);
        emit SendMsgToServer(AddFri,Global::UserAccount(),targetacc,"","同意好友申请");
    }
    qDebug() << "退出添加好友: " << targetacc << " 界面" ;
    //获得该好友界面指针并退出该界面
    AddFriend * af = (AddFriend*)sender();
    m_addfri.removeOne(af);
    af->close();
    af->deleteLater();
}

void MainInterface::Reconnection(bool onl)
{
    if(onl == false)
    {
        qDebug() << "与服务器断开连接";
    }
    else
    {
        qDebug() << "恢复连接";
        //如果已登录到主界面则重新上线
        if(isLogined)
        {
            emit SendMsgToServer(ChangeOnlSta,Global::UserAccount(),-1,"在线");
        }
    }
}

void MainInterface::SendMsgToFri(int targetAcc,MsgType type, QString msg)
{
    switch(type)
    {
    case itsMsg:
    {
        emit SendMsgToServer(SendMsg,Global::UserAccount(),targetAcc,msg,"普通信息");
        break;
    }
    case itsPicture:
    {
        emit SendMsgToServer(SendMsg,Global::UserAccount(),targetAcc,msg,"发送图片");
        break;
    }
    case itsFile:
    {
        emit SendMsgToServer(SendFileToFri,Global::UserAccount(),targetAcc,msg,"发送文件");
        break;
    }
    case RecvFile:
    {
        emit SendMsgToServer(SendFileToFri,Global::UserAccount(),targetAcc,msg,"接收文件");
        break;
    }
    default:
        break;
    }
}

void MainInterface::DelFri()
{
    if(m_friItem)
    {
        //获取要删除的好友账号
        int targetacc = m_friItem->data(0,Qt::UserRole).toInt();
        emit SendMsgToServer(AddFri,Global::UserAccount(),targetacc,"","删除好友");
    }
}

ChatWindow* MainInterface::showFriChatWindow(int acc)
{
    ChatWindow * ctw = m_chatWindows.value(acc);
    if(ctw == nullptr)
    {
        ctw = new ChatWindow(acc,m_frinicknames[acc]);
        connect(ctw,&ChatWindow::SendMsgToFri,this,&MainInterface::SendMsgToFri);
        connect(m_mytcp,&TcpThread::SendProgressInfo,ctw,&ChatWindow::GetProgressInfo);
        m_chatWindows.insert(acc,ctw);
    }
    ctw->activateWindow();
    ctw->showNormal();
    return ctw;
}

void MainInterface::ClosePerData(int acc)
{
    PersonalData * pd = (PersonalData*)sender();
    m_PersonData.remove(acc);
    pd->close();
    pd->deleteLater();
}

void MainInterface::EditPersonalData(QStringList uD)
{
    if(m_editData == nullptr)
    {
        m_editData = new ChangeData(uD);
        connect(m_editData,&ChangeData::CloseEdit,this,&MainInterface::CloseEdit);
        connect(m_editData,&ChangeData::ChangeUserDatas,this,&MainInterface::ChangeUserDatas);
    }
    m_editData->show();
}

void MainInterface::CloseEdit()
{
    m_editData->close();
    delete m_editData;
    m_editData = nullptr;
}

void MainInterface::ChangeUserDatas(QString datas)
{
    //先更新主界面
    QString nkN = datas.split("@@").at(Dnickname);
    QString sig = datas.split("@@").at(Dsignature);

    if(nkN != Global::UserNickName() && sig != Global::UserSignature())
    {
        Global::InitUserNameAndSig(nkN,sig);
        ui->NickNameLab->setText(nkN);
        ui->SignatureLine->setText(sig);
        ChangingLoginFile(nkN,"");
        emit SendMsgToServer(UserChangeData,Global::UserAccount(),-1,datas,"");
        return;
    }
    if(nkN != Global::UserNickName())
    {
        Global::InitUserNameAndSig(nkN,Global::UserSignature());
        ui->NickNameLab->setText(nkN);
        ChangingLoginFile(nkN,"");
    }
    if(sig != Global::UserSignature())
    {
        Global::InitUserNameAndSig(Global::UserNickName(),sig);
        ui->SignatureLine->setText(sig);
    }

    emit SendMsgToServer(UserChangeData,Global::UserAccount(),-1,datas,"");
}

void MainInterface::ChangingLoginFile(QString NkN, QString pwd)
{
    Q_UNUSED(pwd);
    //更改昵称
    if(NkN != "")
    {
        QFile file(Global::UserPath() + "/login.txt");
        file.open(QFile::ReadOnly);
        QByteArray loginD = file.readAll();
        file.close();

        QString data(loginD);
        QStringList datas = data.split("@@");
        //删除原来的昵称
        datas.removeAt(0);
        //清空data中内容
        data.clear();
        //填入新昵称
        data.append(NkN);
        //加入其他信息
        for(auto d : datas)
        {
            data.append("@@");
            data.append(d);
        }
        //更新文件
        file.open(QFile::WriteOnly | QFile::Truncate);
        file.write(data.toUtf8());
        file.close();
    }
    else
    {

    }
}

void MainInterface::ChangingHeadShot()
{
    emit SendMsgToServer(UpdateHeadShot,Global::UserAccount());
    //更新主界面头像
    QTimer::singleShot(1000,[=](){
        QPixmap pix = Global::CreateHeadShot(Global::UserHeadShot());
        ui->HeadShotBtn->setIcon(QIcon(pix));
    });
}

void MainInterface::initShadow()
{
    QPainter painter(ui->frame);
    painter.fillRect(ui->frame->rect().adjusted(-10,-10,10,10),QColor(220,220,220));
}

void MainInterface::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(QColor(0,0,0,1)); //窗口全透明无法接收鼠标移动事件
    painter.drawPath(m_globalPath); //绘制全局路径
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
        int acc = item->data(0,Qt::UserRole).toInt();
        showFriChatWindow(acc);
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
            /*
            if(m_chaggrpitem != nullptr)
            {
                m_chaggrpitem->setFlags(Qt::ItemIsEnabled);
            }
            */

            qDebug() << "233";
            m_chaggrpitem = sitem;
            m_chagName = sitem->text(0);

            m_grpmenu->exec(QCursor::pos());
        }
        //右键点击好友
        else
        {
            m_friItem = sitem;
            m_frimenu->exec(QCursor::pos(0));
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
            //好友列表已更改
            FriIsChanged = true;
            //取出该分组下好友后删除该分组
            m_groupNames.removeOne(m_chagName);
            QList<int> moveList = m_friends.values(m_chagName);
            m_friends.remove(m_chagName);

            //好友移动分组菜单中同步删除
            QAction * act = m_movegroups.value(m_chagName);
            m_movegroups.remove(m_chagName);
            delete act;

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
            disconnect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainInterface::ItemNameChanged);
        }
        else
        {
            //未重复则将该分组名添加进分组名容器中
            m_groupNames.append(gn);
            //好友列表已更改
            FriIsChanged = true;
            disconnect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainInterface::ItemNameChanged);
            item->setFlags(Qt::ItemIsEnabled);

            //同步添加好友移动分组菜单
            QAction * act = new QAction(gn,this);
            connect(act,&QAction::triggered,this,&MainInterface::MoveFriend);
            m_movegroups.insert(gn,act);
            m_moveFri->addAction(act);
        }
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
                //好友列表已更改
                FriIsChanged = true;

                //同步修改好友移动分组菜单
                QAction * act = m_movegroups.value(m_chagName);
                act->setText(gn);
                m_movegroups.remove(m_chagName);
                m_movegroups.insert(gn,act);

                qDebug() << "已完成重命名";
            }
        }
        disconnect(ui->treeWidget,&QTreeWidget::itemChanged,this,&MainInterface::ItemNameChanged);
        item->setFlags(Qt::ItemIsEnabled);
    }
}

void MainInterface::on_HeadShotBtn_clicked()
{
    qDebug() << "打开自己的个人资料";
    emit SendMsgToServer(AskForData,Global::UserAccount(),-1,"","请求自己的");
}

void MainInterface::MoveFriend()
{
    if(m_friItem)
    {
        QAction * act = (QAction*)sender();

        //查找该好友的分组名
        int acc = m_friItem->data(0,Qt::UserRole).toInt();
        QString gpN = "";
        for(auto g : m_groupNames)
        {
            qDebug() << g;
            QList<int> fris = m_friends.values(g);
            for(auto f : fris)
            {
                if(f == acc)
                {
                    qDebug() << "找到该好友，该好友在" << g << "分组";
                    gpN = g;
                    break;
                }
            }
            //若已找到则退出循环
            if(gpN != "")
            {
                break;
            }
        }

        //若选择的为当前分组则不变
        if(act->text() == gpN)
        {
            QMessageBox::information(this,"提示","已在该分组下,请选择其他分组");
            return;
        }

        //移动到所选分组
        m_friends.remove(gpN,acc);
        m_friends.insert(act->text(),acc);
        FriIsChanged = true;
        UpdateTreeWidget();
        qDebug() << "已改变分组，原分组" << gpN << "现分组: " << act->text();
    }
}
