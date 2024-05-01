#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QProcess>
#include <QFileInfo>
#include <QMovie>
#include <QScrollBar>

#define Margin 6 //边框

#define MinWindowMaxWordsWidth 562 //窗口处于最小宽度时单行容纳最多字数时的宽度
#define MinWindowMaxWords 45 //窗口处于最小宽度时单行可容纳最多字数
#define BaseHeight 46 //文字气泡基础高度
#define InsertHeight 13 //每增加一行文字气泡需要的增加的高度

#define MinWindowMaxPicWidth 360 //最宽图片宽度
#define WindowDefaultWidth 950 //窗口默认宽度
#define FileListWidgetWidth 280 //文件收发栏宽度

ChatWindow::ChatWindow(int targetAcc,QString nickN,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);

    //初始化全局路径，全屏闭合回路
    m_pixmap = QPixmap(this->width(),this->height());
    m_globalPath.lineTo(m_pixmap.width(), 0);
    m_globalPath.lineTo(m_pixmap.width(), m_pixmap.height());
    m_globalPath.lineTo(0, m_pixmap.height());
    m_globalPath.lineTo(0, 0);

    //初始化好友信息
    m_targetAcc = targetAcc;
    m_nickname = nickN;

    //初始化好友头像位置及自己的头像及用户文件夹位置
    m_FriHeadShot = QCoreApplication::applicationDirPath() + "/userdata/allusers/" + QString::number(targetAcc) + ".jpg";

    //设置无边框窗口
    setWindowFlags(Qt::FramelessWindowHint);

    //设置背景透明
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    //设置阴影边框
    QGraphicsDropShadowEffect * shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setOffset(0,0);
    shadow->setColor(Qt::black);
    shadow->setBlurRadius(10);
    ui->frame->setGraphicsEffect(shadow);

    //安装事件过滤器，处理绘画事件
    ui->frame->installEventFilter(this);
    update();

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

    //设置分割界面初始比例
    ui->splitter->setStretchFactor(0,3);
    ui->splitter->setStretchFactor(1,1);
\
    //绑定最大小化及关闭按钮的信号槽
    connect(ui->MiniBtn,&QToolButton::clicked,this,&ChatWindow::showMinimized);
    connect(ui->CloseBtn,&QToolButton::clicked,this,&ChatWindow::hide);
    connect(ui->CloseBtn_2,&QPushButton::clicked,this,&ChatWindow::hide);

    ui->ChatList->verticalScrollBar()->setStyleSheet(Global::scrollbarStyle);
    ui->listWidget->verticalScrollBar()->setStyleSheet(Global::scrollbarStyle);

    //初始化文件接收栏状态
    ChangefileListSta(false);
    m_loc = Center;
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
    Q_UNUSED(event);
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
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(QColor(0,0,0,1)); //窗口全透明无法接收鼠标移动事件
    painter.drawPath(m_globalPath); //绘制全局路径
}

int ChatWindow::returnItemHeight(MsgType MsgType,int wLgh,int heightPic)
{
    //判断信息类型
    switch(MsgType)
    {
    case itsTime:
        return 20;
    case itsMsg:
    {
        //根据发送信息的字数判断item的高度
        if(wLgh <= MinWindowMaxWords)
        {
            return BaseHeight;
        }
        else
        {
            int multiple = wLgh / MinWindowMaxWords;

            //若刚好等于最小可容纳字数
            if(wLgh % MinWindowMaxWords == 0)
            {
                return BaseHeight + (multiple - 1) * InsertHeight;
            }
            return BaseHeight + multiple * InsertHeight;
        }
        break;
    }
    case itsPicture:
    {
        if(wLgh <= MinWindowMaxPicWidth)
        {
            //若图片小于头像高度则不返回图片高度
            return (heightPic + 10) < 50 ? 50 : (heightPic + 10);
        }
        else
        {
            //若图片过大则将图片设置到合适的大小
            double scaleMul = (double)wLgh / (double)MinWindowMaxPicWidth;
            double pHeight = (double)heightPic / (double)scaleMul;
            return (int)pHeight + 10;
        }
    }
        break;
    case itsFile:
        return 110;
        break;
    default:
        break;
    }

    return 0;
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
        if(Msg.length() < MinWindowMaxWords)
        {
            //未达到单行容纳字数上限时设置宽尽量小
            MsgLab->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
        }
        else
        {
            //自动换行并设置最小宽度
            MsgLab->setWordWrap(true);
            MsgLab->setMinimumWidth(MinWindowMaxWordsWidth);
        }

        //判断是好友还是自己设置对应的label样式
        if(isMe)
        {
            MsgLab->setStyleSheet("QLabel{background-color:rgb(18,183,245);color:white;padding-top:5px;padding-bottom:5px;"
                                  "padding-left:8px;padding-right:8px;border-style:none;border-radius:8px;}");
            //好友信息在左边，自己在右边
            layout->setDirection(QHBoxLayout::RightToLeft);
        }
        else
        {
            MsgLab->setStyleSheet("QLabel{background-color:rgb(229,229,229);color:black;padding-top:5px;padding-bottom:5px;"
                                  "padding-left:8px;padding-right:8px;border-style:none;border-radius:8px;}");
            layout->setDirection(QHBoxLayout::LeftToRight);
        }

        //设置四周边距
        layout->setContentsMargins(10,2,10,2);

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
        qDebug() << img.width() << img.height();

        //若图片宽度未超过设定宽度则直接设置
        if(img.width() > MinWindowMaxPicWidth)
        {
            //若图片过大则将图片设置到合适的大小
            double scaleMul = (double)img.width() / (double)MinWindowMaxPicWidth;
            double pHeight = (double)img.height() / (double)scaleMul;
            img = img.scaled(MinWindowMaxPicWidth,(int)pHeight);
            qDebug() << img.width() << img.height();
        }

        if(Msg.split(".").last() == "gif")
        {
            QMovie * mov = new QMovie(Msg);
            mov->setScaledSize(QSize(img.width(),img.height()));
            PicLab->setMovie(mov);
            mov->start();
        }
        else
        {
            PicLab->setPixmap(QPixmap::fromImage(img));
        }

        if(isMe)
        {
            layout->setDirection(QBoxLayout::RightToLeft);
        }
        else
        {
            layout->setDirection(QBoxLayout::LeftToRight);
        }

        layout->setContentsMargins(10,5,10,5);
        layout->addWidget(HeadS);
        layout->addWidget(PicLab);
        layout->addStretch();

        break;
    }
    case itsFile:
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

        //接收文件widget
        QWidget * FileWidget = new QWidget(this);
        FileWidget->setFixedSize(300,100);
        FileWidget->setStyleSheet(".QWidget{border-style:solid;border-color:rgb(207,219,226);border-width:1px;}");

        //文件layout
        QVBoxLayout * FileLayout = new QVBoxLayout;
        FileLayout->setContentsMargins(1,1,1,1);

        //文件信息widget
        QWidget * fileInfoWidget = new QWidget(this);
        //fileInfoWidget->setFixedHeight(70);
        fileInfoWidget->setStyleSheet(".QWidget{border-style:solid;border-color:rgb(207,219,226);"
                                      "border-top-width:0px;border-left-width:0px;border-right-width:0px;border-bottom-width:1px;}");

        //信息总layout
        QHBoxLayout * infoHL = new QHBoxLayout;
        infoHL->setContentsMargins(10,1,2,1);

        //文件图标
        QLabel * IconL = new QLabel;

        //获取文件对应的文件类型图片
        QPixmap filepix = JudgeFileBack(Msg);
        IconL->setPixmap(filepix);

        IconL->setFixedSize(40,55);
        IconL->setScaledContents(true);
        IconL->setStyleSheet("QLabel{border-style:solid;border-width:1px;border-color:rgb(234,234,234);}");

        //文件名和文件提示信息Label
        QVBoxLayout * FNamesL = new QVBoxLayout;
        FNamesL->setSpacing(1);
        FNamesL->setContentsMargins(3,5,0,5);
        QString fName = Msg.split("/").last();
        QLabel * Fname = new QLabel;
        //文件名过长则显示省略号
        if(fName.length() > 15)
        {
            QString fif = fName.left(4) + "..." + fName.right(6);
            Fname->setText(fif);
        }
        else
        {
            Fname->setText(fName);
        }
        Fname->setToolTip(fName);
        QLabel * tips = new QLabel;
        tips->setStyleSheet("QLabel{color:rgb(136,136,136);}");

        if(isMe)
        {
            tips->setText("成功发送在线文件");
        }
        else
        {
            tips->setText("成功接收在线文件");
        }

        //获取文件大小
        QFileInfo info(Msg);
        qint64 fileSize = info.size();
        QString sizeStr = GetFileSize(fileSize);
        tips->setText(tips->text() + sizeStr);

        //文件名和文件提示
        FNamesL->addWidget(Fname);
        FNamesL->addWidget(tips);

        //文件信息总layout
        infoHL->addWidget(IconL);
        infoHL->addLayout(FNamesL);

        fileInfoWidget->setLayout(infoHL);

        //按钮组layout
        QHBoxLayout * btnLay = new QHBoxLayout;
        btnLay->setSpacing(5);
        btnLay->setContentsMargins(0,5,10,10);

        //打开和打开文件夹按钮
        QPushButton * Open = new QPushButton("打开");
        QPushButton * OpenDir = new QPushButton("打开文件夹");
        //设置光标样式
        Open->setCursor(QCursor(Qt::PointingHandCursor));
        OpenDir->setCursor(QCursor(Qt::PointingHandCursor));

        Open->setStyleSheet("QPushButton{background-color:transparent;color:rgb(18,183,245);border-style:solid;"
                            "border-top-width:0px;border-bottom-width:0px;border-left-width:0px;border-right-width:0px;border-color:rgb(18,183,245)}"
                            "QPushButton:hover{background-color:transparent;color:rgb(18,183,245);border-style:solid;"
                            "border-top-width:0px;border-bottom-width:2px;border-left-width:0px;border-right-width:0px;border-color:rgb(18,183,245)}");

        OpenDir->setStyleSheet("QPushButton{background-color:transparent;color:rgb(18,183,245);border-style:solid;"
                                "border-top-width:0px;border-bottom-width:0px;border-left-width:0px;border-right-width:0px;border-color:rgb(18,183,245)}"
                                "QPushButton:hover{background-color:transparent;color:rgb(18,183,245);border-style:solid;"
                                "border-top-width:0px;border-bottom-width:2px;border-left-width:0px;border-right-width:0px;border-color:rgb(18,183,245)}");

        //打开文件
        connect(Open,&QPushButton::clicked,this,[=](){
           if(!QDesktopServices::openUrl(QUrl("file:///" + Msg)))
           {
               qDebug() << "打开文件: " << Msg << "失败";
           }
        });

        //打开文件目录
        connect(OpenDir,&QPushButton::clicked,this,[=](){
            QStringList pieces = Msg.split("/");
            pieces.removeLast();
            QString fpath = "";
            for(auto path : pieces)
            {
                fpath.append(path);
                fpath.append("/");
            }
            QDesktopServices::openUrl(QUrl("file:///" + fpath));
        });

        //设置按钮组layout
        btnLay->addStretch();
        btnLay->addWidget(Open);
        btnLay->addWidget(OpenDir);

        //设置总layout
        FileLayout->addWidget(fileInfoWidget);
        FileLayout->addLayout(btnLay);

        //文件widget设置layout
        FileWidget->setLayout(FileLayout);

        if(isMe)
        {
            layout->setDirection(QBoxLayout::RightToLeft);
        }
        else
        {
            layout->setDirection(QBoxLayout::LeftToRight);
        }

        layout->addWidget(HeadS);
        layout->addWidget(FileWidget);
        layout->addStretch();

        break;
    }
    default:
        break;
    }
    widget->setLayout(layout);
    return widget;
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

QString ChatWindow::GetFileSize(const qint64 &size)
{
    int integer = 0; //整数
    int decimal = 0; //小数
    QString unit = "B";

    qint64 RealSize = size; //换算后大小
    qint64 dSize = size; //取小数位

    integer = RealSize;

    if(RealSize > 1024)
    {
        dSize = RealSize * 1000 / 1024;
        integer = dSize / 1000; //整数位
        decimal = dSize % 1000; //小数位
        RealSize /= 1024;
        unit = "KB";
        if(RealSize > 1024)
        {
            dSize = RealSize * 1000 / 1024;
            integer = dSize / 1000;
            decimal = dSize % 1000;
            RealSize /= 1024;
            unit = "MB";
            if(RealSize > 1024)
            {
                dSize = RealSize * 1000 / 1024;
                integer = dSize / 1000;
                decimal = dSize % 1000;
                RealSize /= 1024;
                unit = "GB";
            }
        }
    }

    QString dec = "";
    decimal /= 10;
    //保留两位小数
    if(decimal < 10){
        dec = "0" + QString::number(decimal);
    }else{
        dec = QString::number(decimal);
    }

    QString FileSize = "(" + QString::number(integer) + "." + dec + unit + ")";
    return FileSize;
}

QPixmap ChatWindow::JudgeFileBack(QString fileName)
{
    QString fileBack = fileName.split(".").last();
    QPixmap filepix;
    //判断是什么文件
    if(fileBack == "txt")
    {
        filepix = QPixmap(":/lib/fileIcons/txt.png");
    }
    else if(fileBack == "doc" || fileBack == "docx")
    {
        filepix = QPixmap(":/lib/fileIcons/word.png");
    }
    else if(fileBack == "ppt" || fileBack == "pptx")
    {
        filepix = QPixmap(":/lib/fileIcons/ppt.png");
    }
    else if(fileBack == "xls" || fileBack == "xlsx")
    {
        filepix = QPixmap(":/lib/fileIcons/excel.png");
    }
    else if(fileBack == "pdf")
    {
        filepix = QPixmap(":/lib/fileIcons/PDF.png");
    }
    else if(fileBack == "mp3" || fileBack == "wma" || fileBack == "wav" || fileBack == "acc" || fileBack == "flac")
    {
        filepix = QPixmap(":/lib/fileIcons/mp3.png");
    }
    else if(fileBack == "mp4" || fileBack == "avi" || fileBack == "rmvb" || fileBack == "mkv" || fileBack == "h264" || fileBack == "flv")
    {
        filepix = QPixmap(":/lib/fileIcons/mp4.png");
    }
    else if(fileBack == "rar" || fileBack == "zip" || fileBack == "7z")
    {
        filepix = QPixmap(":/lib/fileIcons/zip.png");
    }
    else if(fileBack == "bmp" || fileBack == "jpg" || fileBack == "jpeg" || fileBack == "png" || fileBack == "gif")
    {
        filepix = QPixmap(":/lib/fileIcons/photo.png");
    }
    else
    {
        filepix = QPixmap(":/lib/fileIcons/WindowsWhite.png");
    }
    return filepix;
}

void ChatWindow::ChangefileListSta(bool isRecvOrSend)
{
    //正在接收或发送文件
    if(isRecvOrSend)
    {
        ui->listWidget->show();
        //更改窗口宽度和最小宽度
        this->resize(this->width() + FileListWidgetWidth,this->height());
        this->setMinimumWidth(WindowDefaultWidth);
    }
    else
    {
        ui->listWidget->hide();
        this->setMinimumWidth(WindowDefaultWidth - FileListWidgetWidth);
        this->resize(this->width() - FileListWidgetWidth,this->height());
    }
}

void ChatWindow::FriendSendMsg(bool isMe,MsgType MsgType, QString Msg)
{
    if(MsgType == itsFile)
    {
        //从文件收发ListWidget中移除已完成发送/接收的文件item
        QListWidgetItem *fItem = m_fileItems.value(Msg);
        m_fileItems.remove(Msg);
        ui->listWidget->removeItemWidget(fItem);
        delete fItem;

        m_progressBars.remove(Msg);
        m_speedLab.remove(Msg);
        m_resttimeLab.remove(Msg);

        //当无收发文件时清空listWidget并隐藏
        if(ui->listWidget->count() == 0)
        {
            //ui->listWidget->clear();
            ChangefileListSta(false);
        }

        //获取文件后缀名
        QString fileBack = Msg.split(".").last();

        //判断该文件是否是图像文件
        if(fileBack == "bmp" || fileBack == "jpg" || fileBack == "jpeg" || fileBack == "png" || fileBack == "gif")
        {
            FriendSendMsg(isMe,itsPicture,Msg);
            return;
        }
    }

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
        QString TimeStr = curTime.toString("yyyy/MM/dd hh:mm:ss");
        QWidget * TimeWgt = CreateWidget(true,itsTime,TimeStr);

        //将widget添加入聊天框中
        ui->ChatList->setItemWidget(TimeItem,TimeWgt);
    }

    QListWidgetItem * item = new QListWidgetItem(ui->ChatList);
    if(MsgType == itsPicture)
    {
        QSize size = item->sizeHint();
        QImage img(Msg);
        item->setSizeHint(QSize(size.width(),returnItemHeight(MsgType,img.width(),img.height())));
    }
    else
    {
        QSize size = item->sizeHint();
        item->setSizeHint(QSize(size.width(),returnItemHeight(MsgType,Msg.length())));
    }

    QWidget * widget = CreateWidget(isMe,MsgType,Msg);
    ui->ChatList->setItemWidget(item,widget);
}

void ChatWindow::GetProgressInfo(int friAcc, QString fileName, qint64 speed, int value, QTime RestTime,bool isMe)
{
    //首先查看接收对象是否为本窗口，查看好友账号是否一致
    if(friAcc == m_targetAcc)
    {
        //接收到value值为一百则为收发成功
        if(value == 100)
        {
            FriendSendMsg(isMe,itsFile,fileName);
            return;
        }
        else
        {
            //设置速度和剩余时间
            QLabel * speedL = m_speedLab.value(fileName);
            //小于10M设置速度为灰色
            if((speed / 1024 / 1024) < 10)
            {
                speedL->setStyleSheet("QLabel{color:rgb(127,127,127);}");
            }
            else //否则为黄色
            {
                speedL->setStyleSheet("QLabel{color:rgb(247,159,62);}");
            }
            QString spS = GetFileSize(speed);
            //将头尾的括号删除
            spS.remove(0,1);
            spS.remove(spS.length()-1,1);
            QString spStr = spS + "/S";
            speedL->setText(spStr);

            //设置剩余时间
            QString resttime = RestTime.toString("hh:mm:ss");
            m_resttimeLab.value(fileName)->setText(resttime);
            //设置进度
            m_progressBars.value(fileName)->setValue(value);
            m_progressBars.value(fileName)->update();

            return;
        }
    }
    else
    {
        return;
    }
}

void ChatWindow::SendOrRecvFile(bool isMe, QString fileName)
{
    QWidget * widget = new QWidget(this);
    QListWidgetItem * item = new QListWidgetItem(ui->listWidget);
    item->setSizeHint(QSize(-1,80));
    //总布局
    QHBoxLayout * layout = new QHBoxLayout;
    layout->setContentsMargins(5,1,5,1);
    layout->setSpacing(5);

    //文件传输信息部分
    QVBoxLayout * vlay = new QVBoxLayout;
    vlay->setContentsMargins(2,0,0,0);
    vlay->setSpacing(2);

    //文件信息和收发图标
    QHBoxLayout * infolay = new QHBoxLayout;
    infolay->setContentsMargins(0,0,0,0);

    //收发类型图标
    QLabel * Icon = new QLabel;
    QPixmap pix;
    if(isMe)
    {
        pix = QPixmap(":/lib/fileIcons/send.png");
    }
    else
    {
        pix = QPixmap(":/lib/fileIcons/Recv.png");
    }
    Icon->setPixmap(pix);
    Icon->setScaledContents(true);
    Icon->setFixedSize(20,20);

    //文件名称和大小
    QLabel * finfo = new QLabel;
    if(isMe)
    {
        //若为自己发送的则参数为文件路径
        QFileInfo info(fileName);
        QString fSize = GetFileSize(info.size());
        QString fName = fileName.split("/").last();
        finfo->setToolTip(fName);

        //文件长度过长显示省略号
        QString fif;
        if(fName.length() > 10)
        {
            fif = fName.left(4) + "..." + fName.right(6);
            fif += fSize;
        }
        else
        {
            fif = fName;
            fif += fSize;
        }
        finfo->setText(fif);

    }
    else
    {
        //若为好友发送的则参数为文件名 + ?(文件大小)
        QString fName = fileName.split("?").first();
        QString fSize = fileName.split("?").last();
        finfo->setToolTip(fName);

        QString fif;
        if(fName.length() > 10)
        {
            fif = fName.left(4) + "..." + fName.right(6);
            fif += fSize;
        }
        else
        {
            fif = fName;
            fif += fSize;
        }
        finfo->setText(fif);
    }
    finfo->setFixedHeight(30);

    infolay->addWidget(Icon);
    infolay->addWidget(finfo);

    //进度条
    QProgressBar * bar = new QProgressBar;
    bar->setTextVisible(false);
    bar->setRange(0,100);
    bar->setValue(0);
    bar->setStyleSheet("QProgressBar{min-height:6px; max-height:6px; border-radius:3px; background:rgb(200,202,205);}"
                       "QProgressBar::chunk{border-radius:3px;background:qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:0,"
                       "stop:0 rgb(15,80,148),stop:0.25 rgb(4,126,214),stop:0.5 rgb(54,192,242),stop:0.75 rgb(37,198,253),stop:1 rgb(80,209,97));}");

    vlay->addLayout(infolay);
    vlay->addWidget(bar);

    //速度和剩余时间布局
    QHBoxLayout * speedInfo = new QHBoxLayout;
    speedInfo->setContentsMargins(0,0,5,0);
    //速度
    QLabel * speedL = new QLabel;
    speedL->setFixedSize(100,20);

    //剩余时间
    QLabel * restL = new QLabel;
    restL->setStyleSheet("QLabel{color:rgb(127,127,127);}");
    restL->setFixedSize(100,20);

    if(isMe)
    {
        speedInfo->addWidget(speedL);
        speedInfo->addWidget(restL);
        speedInfo->addStretch();

        vlay->addLayout(speedInfo);
    }
    else
    {
        //如果是好友发送的则未接收前速度label设置为当前时间
        QString CurTime = QTime::currentTime().toString("hh:mm");
        speedL->setStyleSheet("QLabel{color:rgb(127,127,127);}");
        speedL->setText(CurTime);

        //接收方需添加接收按钮
        QPushButton * recv = new QPushButton("接收");
        recv->setStyleSheet("QPushButton{background-color:transparent;border-style:none;color:rgb(18,183,245);}"
                            "QPushButton:hover{background-color:transparent;border-style:none;color:black;}");
        //设置光标样式
        recv->setCursor(QCursor(Qt::PointingHandCursor));

        connect(recv,&QPushButton::clicked,this,[=](){
           recv->hide();
           QString fileN = fileName.split("?").first();
           //获取该文件接收路径
           QString filePath = Global::IsFileExist(Global::UserFileRecvPath() + fileN);
           //保存该item
           m_progressBars.insert(filePath,bar);
           m_speedLab.insert(filePath,speedL);
           m_resttimeLab.insert(filePath,restL);
           m_fileItems.insert(filePath,item);

           emit SendMsgToFri(m_targetAcc,RecvFile,fileN);
        });

        speedInfo->addWidget(speedL);
        speedInfo->addWidget(restL);
        speedInfo->addWidget(recv);

        vlay->addLayout(speedInfo);
    }

    //文件类型icon
    QLabel * fIcon = new QLabel;
    QPixmap fpix;
    if(isMe)
    {
        fpix = JudgeFileBack(fileName);

        //保存该item
        m_progressBars.insert(fileName,bar);
        m_speedLab.insert(fileName,speedL);
        m_resttimeLab.insert(fileName,restL);
        m_fileItems.insert(fileName,item);
    }
    else
    {
        QString fileN = fileName.split("?").first();
        fpix = JudgeFileBack(fileN);
    }
    fIcon->setPixmap(fpix);
    fIcon->setScaledContents(true);
    fIcon->setFixedSize(40,55);

    layout->addWidget(fIcon);
    layout->addLayout(vlay);

    widget->setLayout(layout);

    ui->listWidget->setItemWidget(item,widget);

    if(ui->listWidget->isHidden())
    {
        ChangefileListSta(true);
    }
}

void ChatWindow::initShadow()
{
    QPainter painter(ui->frame);
    painter.fillRect(ui->frame->rect().adjusted(-10,-10,10,10),QColor(220,220,220));
}

bool ChatWindow::eventFilter(QObject *w, QEvent *e)
{
    if((w == ui->frame) && (e->type() == QEvent::Paint))
    {
        initShadow();
        return true;
    }

    return QWidget::eventFilter(w,e);
}

void ChatWindow::on_SendBtn_clicked()
{
    QString str = ui->textEdit->toPlainText();
    if(str == "")
    {
        QMessageBox::information(this,"提示","请输入要发送的信息");
        return;
    }
    if(str.length() > 300)
    {
        QMessageBox::warning(this,"警告","单次发送信息不能超过300字");
        return;
    }
    QString SendMsg = str.simplified(); //去除换行符
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
    QString SendFileName = QFileDialog::getOpenFileName(this,"打开","","Picture Files(*.bmp;*.jpg;*.jpeg;*.png;*.gif)");
    if(SendFileName == "")
    {
        return;
    }
    qDebug() << "发送图片: " << SendFileName;
    //更新自己的聊天框同时发送文件
    emit SendMsgToFri(m_targetAcc,itsPicture,SendFileName);
    FriendSendMsg(true,itsPicture,SendFileName);
}

void ChatWindow::on_FileBtn_clicked()
{
    QString SendFileName = QFileDialog::getOpenFileName(this,"打开","","All (*.*)");
    if(SendFileName == "")
    {
        return;
    }
    /* 关闭大小限制，开启就设置此处文件大小即可
    QFileInfo info(SendFileName);
    long long int size = info.size();
    if(size / 1024 / 1024 > 500)
    {
        QMessageBox::warning(this,"警告","文件过大，请选择小于500MB的文件");
        return;
    }
    */
    qDebug() << "发送文件" << SendFileName;
    //更新聊天框并发送文件
    emit SendMsgToFri(m_targetAcc,itsFile,SendFileName);
    SendOrRecvFile(true,SendFileName);
}
