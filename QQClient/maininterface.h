#ifndef MAININTERFACE_H
#define MAININTERFACE_H

#include <QMainWindow>
#include "login.h"
#include "account.h"
#include "tcpthread.h"
#include <QCoreApplication>
#include <QDir>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMouseEvent>
#include "findfriends.h"

namespace Ui {
class MainInterface;
}

class MainInterface : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainInterface(QWidget *parent = nullptr);
    ~MainInterface();
    void ShowAccount(bool); //显示注册界面
public slots:
    void GetResultFromSer(int acc,QString nickname,QString signature); //获取服务器返回的数据
    void initSystemIcon(); //初始化系统托盘
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason); //点击托盘图标
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void ShowFindFri(); //显示查找好友窗口
signals:
    void StartConnecting(); //连接服务器
    void MainInterfaceClose(); //主窗口退出
private slots:
    void on_CloseBtn_clicked();

    void on_MiniBtn_clicked();

private:
    Ui::MainInterface *ui;
    Login * m_log; //登录界面
    TcpThread * m_mytcp; //网络连接类
    Account * m_acc; //注册界面
    QThread * thread; //tcp线程
    QString m_path = QCoreApplication::applicationDirPath() + "/userdata"; //用户数据文件夹位置
    QDir m_dir; //操作文件目录
    int m_account; //账户
    QString m_headshot; //头像地址
    QString m_nickname; //昵称
    QString m_signature; //个性签名
    QSystemTrayIcon * m_sysIcon; //托盘功能
    QAction* m_quit; //退出
    QAction* m_show; //恢复
    QMenu* m_menu; //托盘菜单signature; //个性签名
    QPoint m_point; //记录鼠标点下位置
    bool isMainWidget = false; //记录点下时是否在主窗口上而非内部控件上
    FindFriends * m_FindFri; //查找好友窗口
};

#endif // MAININTERFACE_H
