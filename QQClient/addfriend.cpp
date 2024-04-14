#include "addfriend.h"
#include "ui_addfriend.h"

AddFriend::AddFriend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFriend)
{
    ui->setupUi(this);

    //设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);

    //最小化和关闭按钮
    connect(ui->MiniBtn,&QToolButton::clicked,this,&AddFriend::showMinimized);
    //关闭等同忽略该好友申请
    connect(ui->CloseBtn,&QToolButton::clicked,this,[=](){

    });
}

AddFriend::~AddFriend()
{
    delete ui;
}

void AddFriend::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        isMainWidget = true;
        m_point = e->globalPos() - pos();
    }
}

void AddFriend::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton && isMainWidget)
    {
        move(e->globalPos() - m_point);
    }
}

void AddFriend::mouseReleaseEvent(QMouseEvent *event)
{
    //释放时将bool值恢复false
    isMainWidget = false;
    event->accept();
}
