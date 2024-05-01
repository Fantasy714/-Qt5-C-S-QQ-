#include "mainwindow.h"
#include "global.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Global::GetWorkPath(QApplication::applicationDirPath() + "/usersdata"); //用户数据文件夹
    MainWindow w;
    w.show();
    return a.exec();
}
