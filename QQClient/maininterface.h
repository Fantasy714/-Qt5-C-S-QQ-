#ifndef MAININTERFACE_H
#define MAININTERFACE_H

#include <QWidget>
#include "login.h"
#include "account.h"
#include "tcpthread.h"
#include <QCoreApplication>
#include <QDir>

namespace Ui {
class MainInterface;
}

class MainInterface : public QWidget
{
    Q_OBJECT

public:
    explicit MainInterface(QWidget *parent = nullptr);
    ~MainInterface();
    void ShowAccount(bool); //显示注册界面
signals:
    void StartConnecting(); //连接服务器

private:
    Ui::MainInterface *ui;
    Login * m_log; //登录界面
    TcpThread * m_mytcp; //网络连接类
    Account * m_acc; //注册界面
    QThread * thread; //tcp线程
    QString m_path = QCoreApplication::applicationDirPath() + "/userdata"; //用户数据文件夹位置
    QDir m_dir; //操作文件目录
};

#endif // MAININTERFACE_H
