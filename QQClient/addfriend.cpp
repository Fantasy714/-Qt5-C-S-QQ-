#include "addfriend.h"
#include "ui_addfriend.h"
#include <QPainter>
#include <QMessageBox>

AddFriend::AddFriend(bool type,QStringList gn, QStringList umsg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddFriend)
{
    ui->setupUi(this);

    m_type = type;

    if(type == false)
    {
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
    QPixmap pix = CreatePixmap(m_alluserspath + "/" + QString::number(m_acc) + ".jpg");
    ui->HeadshotLab->setPixmap(pix);
    ui->AccLab->setText(QString::number(m_acc));
    ui->NickNameLab->setText(m_nickname);
    ui->SexLab->setText("性别:" + m_sex);
    ui->AgeLab->setText("年龄:" + m_age);
    ui->LocLab->setText("所在地:" + m_location);

    //设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);

    //最小化
    connect(ui->MiniBtn,&QToolButton::clicked,this,&AddFriend::showMinimized);
    //若为收到好友申请则关闭等同忽略该好友申请
    connect(ui->CloseBtn,&QToolButton::clicked,this,[=](){
            emit CloseAddFriend("关闭");
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

QPixmap AddFriend::CreatePixmap(QString picPath)
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

void AddFriend::on_NoBtn_clicked()
{
    if(ui->NoBtn->text() == "取消")
    {
        emit CloseAddFriend("关闭");
    }
}

void AddFriend::on_OkBtn_clicked()
{
    if(ui->OkBtn->text() == "完成")
    {
        QMessageBox::information(this,"提示","您的好友添加请求已发送，请等待对方确认");
        emit CloseAddFriend("完成",m_acc,ui->FriCombo->currentText());
    }
}
