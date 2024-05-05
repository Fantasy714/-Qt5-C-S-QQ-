#include "global.h"
#include <QDir>
#include <QDebug>
#include <QPainter>

bool Global::isFirstLogin = false;
bool Global::isRemember = false;
bool Global::isConnecting = false;
QString Global::WorkPath = "";
QString Global::AllUserPath = "";

QString Global::LoginUserPath = "";
QString Global::LoginUserFileRecvPath = "";

int Global::m_UserAccount = -1;
QString Global::m_UserPwd = "";
QString Global::m_UserHeadShot = "";
QString Global::m_nickName = "";
QString Global::m_signature = "";
QString Global::scrollbarStyle = "QScrollBar{width:8px;background:white;margin:0px,0px,0px,0px;}"
                                 "QScrollBar::handle{width:8px;background:rgb(235,235,235);border-radius:4px;height:20px;}"
                                 "QScrollBar::handle:hover{width:8px;background:rgb(139,139,139);border-radius:4px;height:20px;}";

void Global::GetWorkPath(QString path)
{
    WorkPath = path;
    AllUserPath = path + "/allusers";
}

QString Global::AppWorkPath()
{
    return WorkPath;
}

QString Global::AppAllUserPath()
{
    return AllUserPath;
}

bool Global::CreateWorkPath()
{
    QDir dir;

    if(!dir.exists(WorkPath))
    {
        qDebug() << "未创建用户数据文件夹";

        if(!dir.mkdir(WorkPath))
        {
            qDebug() << "创建用户数据文件夹失败";
            return false;
        }
    }

    if(!dir.exists(AllUserPath))
    {
        qDebug() << "allusers数据文件夹未创建";

        if(!dir.mkdir(AllUserPath))
        {
            qDebug() << "allusers数据文件夹创建失败!";
            return false;
        }
    }

    return true;
}

void Global::InitLoginUserInfo(int account, QString pwd)
{
    LoginUserPath = WorkPath + "/" + QString::number(account);
    LoginUserFileRecvPath = LoginUserPath + "/FileRecv/";

    m_UserAccount = account;
    m_UserPwd = pwd;
    m_UserHeadShot = LoginUserPath + "/" + QString::number(account) + ".jpg";
}

void Global::InitUserNameAndSig(QString name, QString sig)
{
    m_nickName = name;
    m_signature = sig;
}

QString Global::UserPath()
{
    return LoginUserPath;
}

QString Global::UserLoginFile()
{
    return LoginUserPath + "/login.txt";
}

QString Global::UserHeadShot()
{
    return m_UserHeadShot;
}

QString Global::UserFileRecvPath()
{
    return LoginUserFileRecvPath;
}

int Global::UserAccount()
{
    return m_UserAccount;
}

QString Global::UserPwd()
{
    return m_UserPwd;
}

QString Global::UserNickName()
{
    return m_nickName;
}

QString Global::UserSignature()
{
    return m_signature;
}

QPixmap Global::CreateHeadShot(QString picPath)
{
    QPixmap src(picPath);
    QPixmap pix(src.width(), src.height());

    //设置图片透明
    pix.fill(Qt::transparent);

    QPainter painter(&pix);
    //设置图片边缘抗锯齿，指示引擎应使用平滑像素图变换算法绘制图片
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    //设置圆形半径，取图片较小边长作为裁切半径
    int radius = src.width() > src.height() ? src.height() / 2 : src.width() / 2;
    //绘制裁切区域的大小
    path.addEllipse(src.rect().center(), radius, radius);
    //设置裁切区域
    painter.setClipPath(path);
    //把源图片的内容绘制到创建的pixmap上，非裁切区域内容不显示
    painter.drawPixmap(pix.rect(), src);

    return pix;
}

QString Global::IsFileExist(QString filepath)
{
    QFileInfo info;

    if(!info.exists(filepath)) //不存在直接返回
    {
        return filepath;
    }
    else
    {
        int num = 1;
        //存在添加后缀
        QString fileName = filepath.split(".").first(); //文件名
        QString fileBack = filepath.split(".").last(); //文件后缀名

        while(true)
        {
            QString FileP = fileName + "(" + QString::number(num) + ")" + "." + fileBack;

            if(!info.exists(FileP))
            {
                qDebug() << "接收重复文件,文件名为: " << FileP;
                return FileP;
            }

            num++;
        }
    }
}
