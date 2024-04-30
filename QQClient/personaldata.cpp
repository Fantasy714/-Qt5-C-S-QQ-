#include "personaldata.h"
#include "ui_personaldata.h"
#include <QPainter>
#include <QFileDialog>
#include <QDebug>

PersonalData::PersonalData(bool isMe,int acc,QStringList UserData,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PersonalData)
{
    ui->setupUi(this);
    isPressed = false;
    //设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);
    //若不是自己则不显示编辑按钮且不能点击头像
    if(!isMe)
    {
        m_isMe = isMe;
        ui->EditBtn->hide();
    }
    else
    {
        m_isMe = true;
    }

    m_acc = acc;
    m_UserData = UserData;

    //设置窗口信息
    setWindowIcon(QIcon(":/lib/QQ.png"));
    setWindowTitle(m_UserData.at(Dnickname) + "的资料");

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

    /* 目前需要显示的信息并不多,需隐藏滚动条 */
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //设置退出该窗口时不退出主程序
    setAttribute(Qt::WA_QuitOnClose,false);

    //设置头像
    QPixmap pix;
    if(isMe)
    {
        pix = Global::CreateHeadShot(Global::UserHeadShot());
    }
    else
    {
        pix = Global::CreateHeadShot(Global::AppAllUserPath() + "/" + QString::number(acc) + ".jpg");
    }
    ui->HeadShotBtn->setIcon(QIcon(pix));
    ui->HeadShotBtn->setIconSize(QSize(65,65));

    //设置昵称签名
    ui->nameLab->setText(m_UserData.at(Dnickname));
    ui->signatureLab->setText(m_UserData.at(Dsignature));

    //设置资料
    ui->acc->setText(QString::number(m_acc));
    ui->nickname->setText(m_UserData.at(Dnickname));
    QString sex_age_birthday = m_UserData.at(Dsex) + "  " + m_UserData.at(Dage) + "  " + m_UserData.at(Dbirthday);
    ui->sex_age_birthday->setText(sex_age_birthday);

    ui->Loc->setText(m_UserData.at(Dlocation));
    ui->blood->setText(m_UserData.at(Dblood_type));
    ui->work->setText(m_UserData.at(Dwork));
    ui->school->setText(m_UserData.at(Dsch_comp));

    connect(ui->MiniBtn,&QToolButton::clicked,this,&PersonalData::showMinimized);
}

PersonalData::~PersonalData()
{
    delete ui;
}

bool PersonalData::eventFilter(QObject *w, QEvent *e)
{
    if((w == ui->frame) && (e->type() == QEvent::Paint))
    {
        initShadow();
        return true;
    }

    return QWidget::eventFilter(w,e);
}

void PersonalData::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        isPressed = true;
        m_point = e->globalPos() - this->frameGeometry().topLeft();
    }
}

void PersonalData::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton && isPressed)
    {
        move(e->globalPos() - m_point);
    }
}

void PersonalData::mouseReleaseEvent(QMouseEvent *event)
{
    isPressed = false;
}

void PersonalData::initShadow()
{
    QPainter painter(ui->frame);
    painter.fillRect(ui->frame->rect().adjusted(-10,-10,10,10),QColor(220,220,220));
}

void PersonalData::CutPhoto(QString path)
{
    QImage img;
    if(!img.load(path))
    {
        qDebug() << "图片加载失败: " << path;
        return;
    }

    int pwidth = img.width();
    int phigh = img.height();
    qDebug() << "图片高为:" << pwidth << "宽" << phigh;
    QImage cimg;
    if(pwidth == phigh)
    {
        cimg = img.copy();
        qDebug() << "图片宽高:" << cimg.width() << "," << cimg.height();
    }
    else if(pwidth > phigh)
    {
        cimg = img.copy(QRect((pwidth - phigh)/2,0,phigh,phigh));
        qDebug() << "截取横屏图片,图片宽高:" << cimg.width() << "," << cimg.height();
    }
    else
    {
        cimg = img.copy(QRect(0,(phigh - pwidth)/2,pwidth,pwidth));
        qDebug() << "截取竖屏图片,图片宽高:" << cimg.width() << "," << cimg.height();
    }

    //头像统一设置为350*350
    cimg = cimg.scaled(350,350,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    qDebug() << "图片宽高:" << cimg.width() << "," << cimg.height();

    //更新图片
    QString fileName = Global::UserHeadShot();

    //删除原来的头像
    bool ok = QFile::remove(fileName);
    if(!ok)
    {
        qDebug() << "删除头像失败";
    }

    cimg.save(fileName);
}

void PersonalData::on_CloseBtn_clicked()
{
    emit ClosePerData(m_acc);
}

void PersonalData::on_EditBtn_clicked()
{
    emit ChangingData(m_UserData);
    emit ClosePerData(m_acc);
}

void PersonalData::on_HeadShotBtn_clicked()
{
    if(m_isMe)
    {
        QString hsFile = QFileDialog::getOpenFileName(this,"更改头像","","Picture Files(*.jpg;*.jpeg;*.png)");
        if(hsFile == "")
        {
            qDebug() << "未选择图像";
            return;
        }
        //qDebug() << "修改头像; " << hsFile;
        CutPhoto(hsFile);
        emit ChangingHeadShot();
        emit ClosePerData(m_acc);
    }
    else
    {
        //qDebug() << "点击好友头像";
    }
}
