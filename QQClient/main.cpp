#include "maininterface.h"
#include "global.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Global::GetWorkPath(QApplication::applicationDirPath() + "/userdata");
    MainInterface m;
    return a.exec();
}
