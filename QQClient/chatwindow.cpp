#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QPainter>
#include <QLabel>

ChatWindow::ChatWindow(int acc,int targetAcc,QString nickN,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    //初始化好友信息
    m_account = acc;
    m_targetAcc = targetAcc;
    m_nickname = nickN;

    //初始化好友头像位置及自己的头像及用户文件夹位置
    m_Mypath = QCoreApplication::applicationDirPath() + "/userdata/" + QString::number(acc);
    m_MyHeadShot = QCoreApplication::applicationDirPath() + "/userdata/" + QString::number(acc) + "/" + QString::number(acc) + ".jpg";
    m_FriHeadShot = QCoreApplication::applicationDirPath() + "/userdata/allusers/" + QString::number(targetAcc) + ".jpg";

    //设置无边框窗口
    setWindowFlag(Qt::FramelessWindowHint);

    //设置好友的昵称
    ui->FriName->setText(m_nickname);

    ui->splitter->setStretchFactor(1,1);

    //设置间距
    ui->ChatList->setSpacing(8);
\
    //绑定最大小化及关闭按钮的信号槽
    connect(ui->MiniBtn,&QToolButton::clicked,this,&ChatWindow::showMinimized);
    connect(ui->BigBtn,&QToolButton::clicked,this,&ChatWindow::showMaximized);
    connect(ui->CloseBtn,&QToolButton::clicked,this,&ChatWindow::hide);
    connect(ui->CloseBtn_2,&QPushButton::clicked,this,&ChatWindow::hide);
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        isMainWidget = true;
        m_point = e->globalPos() - pos();
    }
}

void ChatWindow::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton && isMainWidget)
    {
        move(e->globalPos() - m_point);
    }
}

void ChatWindow::mouseReleaseEvent(QMouseEvent *event)
{
    //释放时将bool值恢复false
    isMainWidget = false;
    event->accept();
}

QPixmap ChatWindow::CreatePixmap(QString picPath)
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

QWidget* ChatWindow::CreateWidget(bool isMe,QString MsgType, QString Msg)
{
    QHBoxLayout * layout = new QHBoxLayout;
    QLabel * MsgLab = new QLabel;
    QWidget * widget = new QWidget(this);
    //设置时间控件
    if(MsgType == "时间")
    {
        MsgLab->setText(Msg);
        MsgLab->setStyleSheet("QLabel{color:rgb(127,127,127);font-size:11px;}");
        layout->setAlignment(MsgLab,Qt::AlignCenter);
        widget->setLayout(layout);
        return widget;
    }

    QPixmap pix;
    QLabel * Hs = new QLabel;
    //设置头像及布局方向
    if(isMe)
    {
        //若是自己的信息则从右往左排列
        layout->setDirection(QHBoxLayout::RightToLeft);
        pix = CreatePixmap(m_MyHeadShot);
    }
    else
    {
        //若是好友的信息则从左往右排列
        layout->setDirection(QHBoxLayout::LeftToRight);
        pix = CreatePixmap(m_FriHeadShot);
    }
    pix = pix.scaled(20,20);
    Hs->setPixmap(pix);

    //设置信息及气泡
    if(MsgType == "普通消息" || MsgType == "添加好友成功")
    {
        MsgLab->setText(Msg);
        if(isMe)
        {
            MsgLab->setStyleSheet("QLabel{color:white;font-size:13px;background-color:rgb(18,183,245);border-style:none;border-radius:5px;}");
        }
        else
        {
            MsgLab->setStyleSheet("QLabel{color:black;font-size:13px;background-color:rgb(229,229,229);border-style:none;border-radius:5px;}");
        }
    }

    layout->addWidget(Hs);
    layout->addWidget(MsgLab);

    widget->setLayout(layout);
    return widget;
}

void ChatWindow::FriendSendMsg(bool isMe,QString MsgType, QString Msg)
{
    //若为第一条信息则设置当前时间
    QString NowTime;
    if(LastMsgTime.secsTo(QDateTime::currentDateTime()) >= 300)
    {
        LastMsgTime = QDateTime::currentDateTime();
        NowTime = LastMsgTime.toString("yyyy-mm-dd hh:mm:ss");

        QWidget * widgetT = CreateWidget(false,"时间",NowTime);
        QListWidgetItem * itemT = new QListWidgetItem(ui->ChatList);
        ui->ChatList->setItemWidget(itemT,widgetT);
    }
    QWidget * widget = CreateWidget(isMe,MsgType,Msg);
    QListWidgetItem * item = new QListWidgetItem(ui->ChatList);
    ui->ChatList->setItemWidget(item,widget);
}

void ChatWindow::on_SendBtn_clicked()
{
    FriendSendMsg(true,"普通信息",ui->textEdit->toPlainText());
    ui->textEdit->clear();
}
