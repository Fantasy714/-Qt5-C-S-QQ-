#include "findfriends.h"
#include "ui_findfriends.h"
#include <QRegExpValidator>
#include <QDebug>

FindFriends* FindFriends::m_Fri = nullptr;
QMutex FindFriends::m_mutex;

FindFriends::FindFriends(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindFriends)
{
    ui->setupUi(this);
    ui->checkBox->setCheckable(false);

    //设置消息提示框样式
    MsgBox.setWindowIcon(QIcon(":/lib/QQ.png"));
    MsgBox.setIcon(QMessageBox::Information);
    MsgBox.setWindowTitle("提示");
    MsgBox.setStyleSheet("QMessageBox{background-color:rgb(235,242,249);}"
                         "QMessageBox QPushButton{background-color:rgb(235,242,250);border-style:solid;border-width:1px;border-color:rgb(0, 170, 255);min-width:70px;min-height:20px;}"
                         "QMessageBox QPushButton:hover{background-color:rgb(0, 170, 255);border-style:solid;border-width:1px;border-color:rgb(116, 186, 220);}"
                         "QPushButton:press{background-color:rgb(235,242,250);border-style:solid;border-width:1px;border-color:rgb(0, 170, 255);}");
    /*
    QPixmap pix(":/lib/gantanhao.png");
    pix = pix.scaled(60,60);
    MsgBox.setIconPixmap(pix);
    */

    //设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);

    //设置窗口图标
    setWindowIcon(QIcon(":/lib/QQ.png"));

    //查账文本栏只能输入数字
    ui->lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));

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

void FindFriends::GetReply(bool type)
{
    //返回正确为已存在该好友，返回错误为该账号未在线或不存在该账号
    if(type)
    {
        MsgBox.setText("您已添加该好友，不能重复添加!");
        MsgBox.exec();
        return;
    }
    else
    {
        MsgBox.setText("该账号未上线或不存在该账号，请确认后重新输入!");
        MsgBox.exec();
        return;
    }
}

void FindFriends::on_MiniBtn_clicked()
{
    this->hide();
}

void FindFriends::on_pushButton_clicked()
{
    //查看是否输入账号
    QString text = ui->lineEdit->text();
    if(text == "")
    {
        MsgBox.setText("未输入账号,请输入账号后再搜索");
        MsgBox.exec();
        return;
    }
    emit SearchingAcc(text);
    ui->lineEdit->clear();
}
