#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>

#define Margin 10

ChatWindow::ChatWindow(int targetAcc,QString nickN,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_TransparentForMouseEvents,false);

    //初始化好友信息
    m_targetAcc = targetAcc;
    m_nickname = nickN;

    //初始化好友头像位置及自己的头像及用户文件夹位置
    m_FriHeadShot = QCoreApplication::applicationDirPath() + "/userdata/allusers/" + QString::number(targetAcc) + ".jpg";

    //设置无边框窗口
    setWindowFlags(Qt::FramelessWindowHint);

    //设置阴影边框
    initShadow();

    //设置鼠标追踪
    this->setMouseTracking(true);

    //用好友的头像和昵称作窗口的图标和名称
    QPixmap pix;
    pix = Global::CreateHeadShot(m_FriHeadShot);
    //设置窗口图标
    setWindowIcon(QIcon(pix));
    setWindowTitle(m_nickname);

    //设置好友的昵称
    ui->FriName->setText(m_nickname);

    ui->ChatList->setSpacing(0);

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
        m_point = e->globalPos() - this->frameGeometry().topLeft();
    }
}

void ChatWindow::mouseMoveEvent(QMouseEvent *e)
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
            if(BottomRight.x() - glbPos.x() > this->minimumWidth())
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
            if(BottomRight.x() - glbPos.x() > this->minimumWidth())
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
            if(BottomRight.x() - glbPos.x() > this->minimumWidth())
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
}

void ChatWindow::mouseReleaseEvent(QMouseEvent *event)
{
    //释放时将bool值恢复false,鼠标恢复默认状态
    setCursor(QCursor(Qt::ArrowCursor));
    m_loc = Center;
    isPressed = false;
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
    painter.fillRect(this->rect().adjusted(15,15,-15,-15),QColor(220,220,220));
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
        /*
         * 后续应权衡SizePolicy和窗口容纳字数多少更改此处消息item高度
        */
        //根据发送信息的字数判断item的高度
        if(wLgh < 19)
        {
            return 50;
        }
        else if(wLgh < 57)
        {
            return 90;
        }
        else if(wLgh < 95)
        {
            return 106;
        }
        else if(wLgh < 133)
        {
            return 120;
        }
        else if(wLgh < 300)
        {
            return 140;
        }
        else if(wLgh < 380)
        {
            return 170;
        }
        else if(wLgh < 520)
        {
            return 220;
        }
        else
        {
            return -1;
        }
    }
    case itsPicture:
        return 250;
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
            pix = Global::CreateHeadShot(Global::UserHeadShot()).scaled(40,40);
        }
        else
        {
            pix = Global::CreateHeadShot(m_FriHeadShot).scaled(40,40);
        }
        HeadS->setPixmap(pix);
        HeadS->setFixedSize(42,42);

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
        layout->setContentsMargins(10,0,10,0);

        layout->addWidget(HeadS);
        layout->addWidget(MsgLab);
        layout->addStretch();
        break;
    }
    case itsPicture:
    {
        //头像Label
        QLabel * HeadS = new QLabel;
        QPixmap pix;
        //判断是自己的头像还是对方的
        if(isMe)
        {
            pix = Global::CreateHeadShot(Global::UserHeadShot()).scaled(40,40);
        }
        else
        {
            pix = Global::CreateHeadShot(m_FriHeadShot).scaled(40,40);
        }
        HeadS->setPixmap(pix);
        HeadS->setFixedSize(42,42);

        //图片Label
        QLabel * PicLab = new QLabel;
        //设置图片
        QImage img(Msg);

        //图片固定高度为230，将图片高度除以230得到图片缩小放大的倍数，再用来将图片宽度缩放到合适的比例
        double scaleNum = (double)img.height() / 230.00;
        double pWidth = (double)img.width() / scaleNum;
        img = img.scaled((int)pWidth,230);

        //设置图片并设置label固定大小
        PicLab->setPixmap(QPixmap::fromImage(img));
        PicLab->setFixedSize(pWidth,230);

        if(isMe)
        {
            layout->setDirection(QBoxLayout::RightToLeft);
        }
        else
        {
            layout->setDirection(QBoxLayout::LeftToRight);
        }

        layout->addWidget(HeadS);
        layout->addWidget(PicLab);
        layout->addStretch();

        break;
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

void ChatWindow::initShadow()
{
    //设置背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    //设置阴影边框
    QGraphicsDropShadowEffect * shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0,0);
    shadow->setColor(Qt::black);
    shadow->setBlurRadius(15);
    this->setGraphicsEffect(shadow);
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
    emit SendMsgToFri(m_targetAcc,itsMsg,SendMsg);
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

void ChatWindow::on_PicBtn_clicked()
{
    QString SendFileName = QFileDialog::getOpenFileName(this,"发送图片","","Picture Files(*.jpg;*.png)");
    if(SendFileName == "")
    {
        return;
    }
    qDebug() << "发送图片: " << SendFileName;
    //更新自己的聊天框同时发送文件
    FriendSendMsg(true,itsPicture,SendFileName);
    emit SendMsgToFri(m_targetAcc,itsPicture,SendFileName);
}
