#include "global.h"
#include <QDir>
#include <QDebug>

QString Global::WorkPath = "";

void Global::GetWorkPath(QString path)
{
    WorkPath = path;
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

    return true;
}

QString Global::UserPath(int account)
{
    return WorkPath + "/" + QString::number(account);
}

QString Global::UserHeadShot(int account)
{
    return UserPath(account) + "/" + QString::number(account) + ".jpg";
}

QString Global::UserFilePath(int account)
{
    return UserPath(account) + "/FileRecv/";
}
