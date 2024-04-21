#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QLabel>
#include <QDebug>
#include <QMessageBox>

#define Margin 5

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

    //设置鼠标追踪
    this->setMouseTracking(true);

    //用好友的头像和昵称作窗口的图标和名称
    QPixmap pix;
    pix = CreatePixmap(m_FriHeadShot);
    //设置窗口图标
    setWindowIcon(QIcon(pix));
    setWindowTitle(m_nickname);

    //设置好友的昵称
    ui->FriName->setText(m_nickname);

    //禁止隐藏聊天框和输入框
    int index1 = ui->splitter->indexOf(ui->widget);
    int index2 = ui->splitter->indexOf(ui->widget_2);
    ui->splitter->setCollapsible(index1,false);
    ui->splitter->setCollapsible(index2,false);
\
    //绑定最大小化及关闭按钮的信号槽
    connect(ui->MiniBtn,&QToolButton::clicked,this,&ChatWindow::showMinimized);
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
        isPressed = true;
        //获取鼠标点下位置相对于窗口左上角的偏移量
        m_point = e->globalPos() - frameGeometry().topLeft();
    }
    e->accept();
}

void ChatWindow::mouseMoveEvent(QMouseEvent *e)
{
    //鼠标未按下则设置鼠标状态
    if(!isPressed)
    {
        qDebug() << e->pos();
        ChangeCurSor(e->pos());
    }
    else
    {

    }
    e->accept();
}

void ChatWindow::mouseReleaseEvent(QMouseEvent *event)
{
    //释放时将bool值恢复false
    isPressed = false;
    event->accept();
}

void ChatWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPoint NowPos = event->pos();

    //若双击位置位于标题栏内则则最大化或正常化
    if(NowPos.y() < 50)
    {
        on_BigBtn_clicked();
    }
    event->accept();
}

void ChatWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(this->rect().)
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

int ChatWindow::returnItemHeight(MsgType MsgType,int wLgh)
{
    //判断信息类型
    switch(MsgType)
    {
    case itsTime:
        return 20;
    case itsMsg:
    {
        //根据发送信息的字数判断item的高度
        if(wLgh < 19)
        {
            return 60;
        }
        else if(wLgh < 57)
        {
            return 100;
        }
        else if(wLgh < 133)
        {
            return 130;
        }
        else if(wLgh < 300)
        {
            return 150;
        }
        else if(wLgh < 380)
        {
            return 180;
        }
        else if(wLgh < 520)
        {
            return 230;
        }
        else
        {
            return -1;
        }
    }
    case itsPicture:
        break;
    case itsFile:
        break;
    default:
        break;
    }
}

void ChatWindow::ChangeCurSor(const QPoint &p)
{
    //获取当前鼠标在窗口中的位置
    int x = p.x();
    int y = p.y();

    //获取当前位置与窗口最右侧及最下方的距离
    int fromRight = frameGeometry().width() - x;
    int fromBottom = frameGeometry().height() - y;

    //若当前位置x,y坐标都小于Margin距离则在左上角
    if(x < Margin && y < Margin)
    {
        m_loc = Top_Left;
        setCursor(QCursor(Qt::SizeFDiagCursor));
    }
}

QWidget *ChatWindow::CreateWidget(bool isMe, MsgType MsgType, QString Msg)
{
    QWidget * widget = new QWidget(this);
    QHBoxLayout * layout = new QHBoxLayout;
    switch(MsgType)
    {
    case itsTime:
    {
        QLabel * timeLab = new QLabel;
        timeLab->setText(Msg);
        timeLab->setStyleSheet("QLabel{color:rgb(127,127,127);}");
        //设置高和宽尽量小
        timeLab->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

        //设置四周边距
        layout->setContentsMargins(0,5,0,0);

        //添加时间label
        layout->addStretch();
        layout->addWidget(timeLab);
        layout->addStretch();
        break;
    }
    case itsMsg:
    {
        //头像Label
        QLabel * HeadS = new QLabel;
        QPixmap pix;
        //判断是自己的头像还是对方的
        if(isMe)
        {
            pix = CreatePixmap(m_MyHeadShot).scaled(40,40);
        }
        else
        {
            pix = CreatePixmap(m_FriHeadShot).scaled(40,40);
        }
        HeadS->setPixmap(pix);

        //信息Label
        QLabel * MsgLab = new QLabel;
        MsgLab->setText(Msg);
        //设置高和宽尽量小
        MsgLab->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
        //自动换行
        MsgLab->setWordWrap(true);
        //判断是好友还是自己设置对应的label样式
        if(isMe)
        {
            MsgLab->setStyleSheet("QLabel{background-color:rgb(18,183,245);color:white;padding-top:10px;padding-bottom:10px;"
                                  "padding-left:8px;padding-right:8px;border-style:none;border-radius:8px;}");
            //好友信息在左边，自己在右边
            layout->setDirection(QHBoxLayout::RightToLeft);
        }
        else
        {
            MsgLab->setStyleSheet("QLabel{background-color:rgb(229,229,229);color:black;padding-top:10px;padding-bottom:10px;"
                                  "padding-left:8px;padding-right:8px;border-style:none;border-radius:8px;}");
            layout->setDirection(QHBoxLayout::LeftToRight);
        }

        //设置四周边距
        layout->setContentsMargins(10,5,10,5);

        layout->addWidget(HeadS);
        layout->addWidget(MsgLab);
        layout->addStretch();
    }
    }
    widget->setLayout(layout);
    return widget;
}


void ChatWindow::FriendSendMsg(bool isMe,MsgType MsgType, QString Msg)
{
    QDateTime curTime = QDateTime::currentDateTime();
    //消息间隔大于五分钟则再次显示时间
    if(LastMsgTime.secsTo(curTime) > 300)
    {
        //更新最后一次消息的时间
        LastMsgTime = curTime;

        //设置item高度
        QListWidgetItem * TimeItem = new QListWidgetItem(ui->ChatList);
        QSize TSize = TimeItem->sizeHint();
        TimeItem->setSizeHint(QSize(TSize.width(),returnItemHeight(itsTime)));

        //取出QString格式时间
        QString TimeStr = curTime.toString("yyyy/m/d h:mm:ss");
        QWidget * TimeWgt = CreateWidget(true,itsTime,TimeStr);

        //将widget添加入聊天框中
        ui->ChatList->setItemWidget(TimeItem,TimeWgt);
    }

    QListWidgetItem * item = new QListWidgetItem(ui->ChatList);
    QSize size = item->sizeHint();
    item->setSizeHint(QSize(size.width(),returnItemHeight(MsgType,Msg.length())));

    QWidget * widget = CreateWidget(isMe,MsgType,Msg);
    ui->ChatList->setItemWidget(item,widget);
}

void ChatWindow::on_SendBtn_clicked()
{
    QString SendMsg = ui->textEdit->toPlainText();
    if(SendMsg == "")
    {
        QMessageBox::information(this,"提示","请输入要发送的信息");
        return;
    }
    FriendSendMsg(true,itsMsg,SendMsg);
    ui->textEdit->clear();
}

void ChatWindow::on_BigBtn_clicked()
{
    //未最大化则最大化并更改图标
    if(!isMaxed)
    {
        isMaxed = true;
        ui->BigBtn->setIcon(QPixmap(":/lib/BigToNormal.png"));
        this->showMaximized();
    }
    else
    {
        isMaxed = false;
        ui->BigBtn->setIcon(QPixmap(":/lib/Big.png"));
        this->showNormal();
    }
}
