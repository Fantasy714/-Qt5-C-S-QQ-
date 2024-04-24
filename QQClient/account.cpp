#include "account.h"
#include "ui_account.h"
#include <QRegExpValidator>
#include <QFont>
#include <QMessageBox>
#include <QDebug>

Account* Account::m_Acc = nullptr;
QMutex Account::m_mutex;

Account::Account(bool find,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Account)
{
    ui->setupUi(this);
    //账户AccountLine只能输入数字
    ui->lineEdit->setValidator(new QRegExpValidator(QRegExp("[0-9]+$")));
    //密码PwdLineEdit只允许输入字母和数字
    ui->lineEdit_2->setValidator(new QRegExpValidator(QRegExp("[a-zA-z0-9]+$")));
    //初始化背景动态图
    m_movie = new QMovie(":/lib/find.gif");
    m_movie->setScaledSize(QSize(800,600));
    m_movie->setSpeed(85);
    ui->AcMovieLabel->setMovie(m_movie);
    m_movie->start();
    //是找回密码则更改界面
    isFind(find);
}

Account* Account::GetAccount(bool find)
{
    //二重锁懒汉模式
    if(m_Acc == nullptr)
    {
        m_mutex.lock();
        if(m_Acc == nullptr)
        {
            m_Acc = new Account(find);
        }
        m_mutex.unlock();
    }
    else
    {
        //如果指针不为空但与此次请求的界面不同则重新构造，相同则直接返回
        if(m_Acc->m_find != find);
        {
            m_mutex.lock();
            if(m_Acc != nullptr)
            {
                delete m_Acc;
                m_Acc = new Account(find);
            }
            m_mutex.unlock();
        }
    }
    return m_Acc;
}

void Account::isFind(bool find)
{
    //若为找回密码则更改界面
    if(find == true)
    {
        ui->label_2->hide();
        ui->lineEdit_2->hide();
        ui->label_3->setText("找回密码");
        ui->pushButton->setText("找回");
    }
    m_find = find;
}

Account::~Account()
{
    delete m_movie;
    delete ui;
}

void Account::closeEvent(QCloseEvent *event)
{
    this->hide();
    BacktoLogin(true);
    //忽略要关闭这个窗口的事件.当前窗口不会被关闭
    event->ignore();
}

void Account::recvResultFromTcp(QString type, QString pwd, QString result)
{
    if(type == "找回密码")
    {
        if(pwd == "")
        {
            QMessageBox::warning(this,"警告","该账号不存在请重新输入!");
        }
        else
        {
            QMessageBox::information(this,"密码已找回","您的密码为:" + pwd + ",请牢记您的密码");
        }
        //服务器回应后恢复按钮
        ui->pushButton->setEnabled(true);
        ui->pushButton->setStyleSheet("font-size:15px;color:white;border-style:none;border-radius:8px 8px;background-color:rgb(0, 170, 255);");
        return;
    }
    else
    {
        if(result == "注册成功")
        {
            QMessageBox::information(this,"注册成功","您的账号注册成功!");
        }
        else
        {
            QMessageBox::warning(this,"注册失败","注册失败！该账号已存在，请重新输入新的账号或点击找回密码。");
        }
        ui->pushButton->setEnabled(true);
        ui->pushButton->setStyleSheet("font-size:15px;color:white;border-style:none;border-radius:8px 8px;background-color:rgb(0, 170, 255);");
        return;
    }
}

void Account::DisConnectedFromSer(bool onl)
{
    //如果信息为断开连接且用户当前停留在注册界面则提示并返回
    if(onl == false && this->isVisible())
    {
        QMessageBox::warning(this,"警告","您已断开连接!");
        this->hide();
        BacktoLogin(true);
    }
}

void Account::on_pushButton_2_clicked()
{
    this->hide();
    BacktoLogin(true);
}

void Account::on_pushButton_clicked()
{
    //检查账号密码合法性
    if(ui->lineEdit->text().isEmpty())
    {
        QMessageBox::warning(this,"警告","请输入账号!");
        return;
    }
    if(ui->lineEdit->text().length() > 9)
    {
        QMessageBox::warning(this,"警告","账号位数不得超过9位!");
        return;
    }
    if(m_find == false)
    {
        if(ui->lineEdit_2->text().isEmpty())
        {
            QMessageBox::warning(this,"警告","请输入密码!");
            return;
        }
    }
    //获取账号密码并发送，如果为找回密码则只发送账号
    int acc = ui->lineEdit->text().toInt();
    QString pwd = ui->lineEdit_2->text();
    if(m_find)
    {
        AccountReq("找回密码",acc);
        //在服务器返回结果前关闭按钮
        ui->pushButton->setEnabled(false);
        ui->pushButton->setStyleSheet("font-size:15px;color:black;border-style:none;border-radius:8px 8px;background-color:rgb(170,170,170);");
        qDebug() << "找回密码";
    }
    else
    {
        AccountReq("注册",acc,pwd);
        ui->pushButton->setEnabled(false);
        ui->pushButton->setStyleSheet("font-size:15px;color:black;border-style:none;border-radius:8px 8px;background-color:rgb(170,170,170);");
        qDebug() << "注册";
    }
}
