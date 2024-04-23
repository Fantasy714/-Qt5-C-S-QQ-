#include "personaldata.h"
#include "ui_personaldata.h"
#include <QPainter>

PersonalData::PersonalData(bool isMe,int acc,QStringList UserData,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PersonalData)
{
    ui->setupUi(this);
    isPressed = false;
    //设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);
    //若不是自己则不显示编辑按钮
    if(!isMe)
    {
        ui->EditBtn->hide();
    }

    m_acc = acc;
    m_UserData = UserData;

    //设置窗口信息
    setWindowIcon(QIcon(":/lib/QQ.png"));
    setWindowTitle(m_UserData.at(Dnickname) + "的资料");
    //初始化窗口边框阴影
    initShadow();

    //设置退出该窗口时不退出主程序
    setAttribute(Qt::WA_QuitOnClose,false);

    //设置头像
    QPixmap pix;
    if(isMe)
    {
        pix = CreatePixmap(m_path + "/" + QString::number(acc) + "/" + QString::number(acc) + ".jpg");
    }
    else
    {
        pix = CreatePixmap(m_alluserspath + "/" + QString::number(acc) + ".jpg");
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

QPixmap PersonalData::CreatePixmap(QString picPath)
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

void PersonalData::initShadow()
{
    //设置背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    //设置阴影边框
    QGraphicsDropShadowEffect * shadow = new QGraphicsDropShadowEffect(this);
    shadow->setOffset(0,0);
    shadow->setColor(Qt::black);
    shadow->setBlurRadius(10);
    this->setGraphicsEffect(shadow);
}

void PersonalData::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(this->rect().adjusted(10,10,-10,-10),QColor(220,220,220));
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
