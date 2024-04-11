#include "maininterface.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainInterface m;
    return a.exec();
}
