#include "addfriend.h"
#include "ui_addfriend.h"
#include <QPainter>
#include <QMessageBox>
#include <QDebug>
#include <QGraphicsDropShadowEffect>

AddFriend::AddFriend(bool type,QStringList gn, QStringList umsg, QString yanzheng, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFriend)
{
    ui->setupUi(this);

    m_type = type;

    //设置背景透明
    setAttribute(Qt::WA_TranslucentBackground);
    //设置阴影边框
    QGraphicsDropShadowEffect * shadow = new QGraphicsDropShadowEffect(ui->frame);
    shadow->setOffset(0,0);
    shadow->setColor(Qt::black);
    shadow->setBlurRadius(10);
    ui->frame->setGraphicsEffect(shadow);
    //安装事件过滤器，处理绘画事件
    ui->frame->installEventFilter(this);
    update();

    //true为发送好友申请界面，false为接收界面
    if(type == false)
    {
        //若有验证消息则添加验证消息
        if(yanzheng != "")
        {
            ui->Yanzheng->setPlainText(yanzheng);
        }
        //更改按钮文字及提示信息
        ui->YanzhengLab->setText("验证信息");
        ui->Yanzheng->setReadOnly(true);
        ui->OkBtn->setText("同意");
        ui->NoBtn->setText("拒绝");
    }

    //设置分组名
    ui->FriCombo->addItems(gn);

    //设置用户信息
    m_acc = umsg.at(enacc).toInt();
    m_nickname = umsg.at(ennickname);
    m_sex = umsg.at(ensex);
    m_age = umsg.at(enage);
    m_location = umsg.at(enlocation);

    //将用户信息显示到界面上
    QPixmap pix = Global::CreateHeadShot(Global::AppAllUserPath() + "/" + QString::number(m_acc) + ".jpg");
    ui->HeadshotLab->setPixmap(pix);
    ui->AccLab->setText(QString::number(m_acc));
    ui->NickNameLab->setText(m_nickname);
    ui->SexLab->setText("性别:" + m_sex);
    ui->AgeLab->setText("年龄:" + m_age);
    ui->LocLab->setText("所在地:" + m_location);

    //设置窗口图标
    setWindowIcon(QIcon(":/lib/QQ.png"));
    //设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);
    //设置退出该窗口时不退出主程序
    setAttribute(Qt::WA_QuitOnClose,false);

    //最小化
    connect(ui->MiniBtn,&QToolButton::clicked,this,&AddFriend::showMinimized);
    //若为收到好友申请则关闭等同忽略该好友申请
    connect(ui->CloseBtn,&QToolButton::clicked,this,[=](){
            emit CloseAddFriend();
    });
}

AddFriend::~AddFriend()
{
    delete ui;
}

void AddFriend::initShadow()
{
    QPainter painter(ui->frame);
    painter.fillRect(ui->frame->rect().adjusted(-10,-10,10,10),QColor(220,220,220));
}

bool AddFriend::eventFilter(QObject *w, QEvent *e)
{
    if((w == ui->frame) && (e->type() == QEvent::Paint)){
        initShadow();
        return true;
    }

    return QWidget::eventFilter(w,e);
}

void AddFriend::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        isPressed = true;
        m_point = e->globalPos() - pos();
    }
}

void AddFriend::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton && isPressed)
    {
        move(e->globalPos() - m_point);
    }
}

void AddFriend::mouseReleaseEvent(QMouseEvent *event)
{
    //释放时将bool值恢复false
    isPressed = false;
}

void AddFriend::on_NoBtn_clicked()
{
    emit CloseAddFriend();
}

void AddFriend::on_OkBtn_clicked()
{
    if(ui->OkBtn->text() == "完成")
    {
        QMessageBox::information(this,"提示","您的好友添加请求已发送，请等待对方确认");
        emit CloseAddFriend("完成",m_acc,ui->FriCombo->currentText(),ui->Yanzheng->toPlainText());
    }
    else
    {
        emit CloseAddFriend("同意",m_acc,ui->FriCombo->currentText());
    }
}
