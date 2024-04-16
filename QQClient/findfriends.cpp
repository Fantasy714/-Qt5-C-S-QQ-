#include "findfriends.h"
#include "ui_findfriends.h"

FindFriends* FindFriends::m_Fri = nullptr;
QMutex FindFriends::m_mutex;

FindFriends::FindFriends(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindFriends)
{
    ui->setupUi(this);

    //设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);
    //设置窗口图标
    setWindowIcon(QIcon(":/lib/QQ.png"));

    connect(ui->MiniBtn,&QToolButton::clicked,this,&FindFriends::showMinimized);
    connect(ui->CloseBtn,&QToolButton::clicked,this,&FindFriends::hide);
}

FindFriends *FindFriends::createFindFriends()
{
    if(m_Fri == nullptr)
    {
        m_mutex.lock();
        if(m_Fri == nullptr)
        {
            m_Fri = new FindFriends;
        }
        m_mutex.unlock();
    }
    return m_Fri;
}

FindFriends::~FindFriends()
{
    delete ui;
}

void FindFriends::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        isMainWidget = true;
        m_point = e->globalPos() - pos();
    }
}

void FindFriends::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton && isMainWidget)
    {
        move(e->globalPos() - m_point);
    }
}

void FindFriends::mouseReleaseEvent(QMouseEvent *event)
{
    //释放时将bool值恢复false
    isMainWidget = false;
    event->accept();
}

void FindFriends::closeEvent(QCloseEvent *event)
{
    //关闭事件改为隐藏此窗口
    hide();
    event->ignore();
}

void FindFriends::on_MiniBtn_clicked()
{
    this->hide();
}
