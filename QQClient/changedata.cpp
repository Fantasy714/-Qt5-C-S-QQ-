#include "changedata.h"
#include "ui_changedata.h"
#include <QDate>
#include <QRegExpValidator>
#include <QDebug>
#include <QMessageBox>

ChangeData::ChangeData(QStringList uD, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChangeData)
{
    ui->setupUi(this);

    //初始未更改
    isChanged = false;

    //设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);

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

    //初始化窗口信息
    setWindowTitle("修改资料");
    setWindowIcon(QIcon(":/lib/QQ.png"));
    //设置退出该窗口时不退出主程序
    setAttribute(Qt::WA_QuitOnClose,false);

    //初始化用户资料
    ui->nickname->setText(uD.at(Dnickname));
    ui->sex->addItem("男");
    ui->sex->addItem("女");
    ui->sex->setCurrentText(uD.at(Dsex));
    QDate date = QDate::fromString(uD.at(Dbirthday),"yyyy-MM-dd");
    ui->birthday->setDate(date );
    ui->signature->setText(uD.at(Dsignature));

    ui->location->setText(uD.at(Dlocation));
    ui->work->setText(uD.at(Dwork));
    ui->school->setText(uD.at(Dsch_comp));

    ui->blood->addItem("A型");
    ui->blood->addItem("B型");
    ui->blood->addItem("O型");
    ui->blood->addItem("AB型");
    ui->blood->addItem("未知");
    ui->blood->setCurrentText(uD.at(Dblood_type));

    //限制用户输入#
    QRegExp rx = QRegExp("[^#]*");
    QRegExpValidator* validator = new QRegExpValidator(rx);
    ui->nickname->setValidator(validator);
    ui->signature->setValidator(validator);
    ui->work->setValidator(validator);
    ui->school->setValidator(validator);
    ui->location->setValidator(validator);

    connect(ui->MiniBtn,&QToolButton::clicked,this,&ChangeData::showMinimized);
    connect(ui->Close,&QToolButton::clicked,this,&ChangeData::CloseEdit);
    connect(ui->CloseBtn,&QPushButton::clicked,this,&ChangeData::CloseEdit);

    //若用户改变资料则保存按钮变为可用
    connect(ui->nickname,&QLineEdit::textChanged,this,&ChangeData::TextChanged);
    connect(ui->sex,&QComboBox::currentTextChanged,this,&ChangeData::TextChanged);
    connect(ui->birthday,&QDateEdit::dateChanged,this,&ChangeData::TextChanged);
    connect(ui->signature,&QLineEdit::textChanged,this,&ChangeData::TextChanged);
    connect(ui->work,&QLineEdit::textChanged,this,&ChangeData::TextChanged);
    connect(ui->school,&QLineEdit::textChanged,this,&ChangeData::TextChanged);
    connect(ui->blood,&QComboBox::currentTextChanged,this,&ChangeData::TextChanged);
    connect(ui->location,&QLineEdit::textChanged,this,&ChangeData::TextChanged);

    //未更改时不能点击保存
    ui->Save->setEnabled(false);
}

ChangeData::~ChangeData()
{
    delete ui;
}

bool ChangeData::eventFilter(QObject *w, QEvent *e)
{
    if((w == ui->frame) && (e->type() == QEvent::Paint))
    {
        initShadow();
        return true;
    }

    return QWidget::eventFilter(w,e);
}

void ChangeData::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        if(e->pos().y() < 35 || e->pos().y() > 690)
        {
            isPressed = true;
        }
        m_point = e->globalPos() - this->frameGeometry().topLeft();
    }
}

void ChangeData::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton && isPressed)
    {
        move(e->globalPos() - m_point);
    }
}

void ChangeData::mouseReleaseEvent(QMouseEvent *event)
{
    isPressed = false;
}

void ChangeData::initShadow()
{
    QPainter painter(ui->frame);
    painter.fillRect(ui->frame->rect().adjusted(-10,-10,10,10),QColor(220,220,220));
}

void ChangeData::TextChanged()
{
    qDebug() << "已更改资料";
    isChanged = true;
    ui->Save->setEnabled(true);

    //更改资料后取消绑定
    disconnect(ui->nickname,&QLineEdit::textChanged,this,&ChangeData::TextChanged);
    disconnect(ui->sex,&QComboBox::currentTextChanged,this,&ChangeData::TextChanged);
    disconnect(ui->birthday,&QDateEdit::dateChanged,this,&ChangeData::TextChanged);
    disconnect(ui->signature,&QLineEdit::textChanged,this,&ChangeData::TextChanged);
    disconnect(ui->work,&QLineEdit::textChanged,this,&ChangeData::TextChanged);
    disconnect(ui->school,&QLineEdit::textChanged,this,&ChangeData::TextChanged);
    disconnect(ui->blood,&QComboBox::currentTextChanged,this,&ChangeData::TextChanged);
    disconnect(ui->location,&QLineEdit::textChanged,this,&ChangeData::TextChanged);
}

void ChangeData::on_Save_clicked()
{
    //输入有效性检查
    if(ui->nickname->text().length() > 15)
    {
        QMessageBox::warning(this,"警告","昵称不能超过15个字!");
        return;
    }
    if(ui->signature->text().length() > 25)
    {
        QMessageBox::warning(this,"警告","个性签名不能超过25个字!");
        return;
    }

    QString uDatas = ui->nickname->text() + "##";
    uDatas += ui->signature->text() + "##";
    uDatas += ui->sex->currentText() + "##";
    QString birthday = ui->birthday->text();
    //当前年份减去生日年份,计算用户年龄
    int age = QDate::currentDate().toString("yyyy/MM/dd").split("/").at(0).toInt() - birthday.split("/").at(0).toInt();
    uDatas += QString::number(age) + "##";
    uDatas += birthday + "##";
    uDatas += ui->location->text() + "##";
    uDatas += ui->blood->currentText() + "##";
    uDatas += ui->work->text() + "##";
    uDatas += ui->school->text();

    qDebug() << "Datas: " << uDatas;
    emit ChangeUserDatas(uDatas);
    emit CloseEdit();
}
