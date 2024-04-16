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
#include <QTreeWidgetItem>

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
    void GetResultFromSer(QString type,int acc,QString nickname,QString signature); //获取服务器返回的数据
    void initSystemIcon(); //初始化系统托盘
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason); //点击托盘图标
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void ShowFindFri(); //显示查找好友窗口
    QPixmap CreatePixmap(QString picPath); //返回圆形头像
    void GetFriendsData(); //从本地数据文件中获取好友列表信息
    void UpdateTreeWidget(); //更新好友列表TreeWidget
    QTreeWidgetItem* CreateTreeWidgetItem(QString fenzuming, int acc = -1);
signals:
    void StartConnecting(); //连接服务器
    void MainInterfaceClose(); //主窗口退出
private slots:
    void on_CloseBtn_clicked();

    void on_MiniBtn_clicked();

private:
    Ui::MainInterface *ui;
    TcpThread * m_mytcp; //网络连接类
    QThread * thread; //tcp线程

    /* 窗口跟随鼠标移动 */
    QPoint m_point; //记录鼠标点下位置
    bool isMainWidget = false; //记录点下时是否在主窗口上而非内部控件上

    /* 用户文件操作 */
    QString m_path = QCoreApplication::applicationDirPath() + "/userdata"; //用户数据文件夹位置
    QString m_alluserspath = m_path + "/allusers"; //存放好友头像文件夹位置
    QString m_userpath; //存放登录用户文件夹位置
    QDir m_dir; //操作文件目录

    /* 其他界面 */
    Login * m_log; //登录界面
    Account * m_accClass; //注册界面
    FindFriends * m_FindFri; //查找好友窗口

    /* 登录用户信息 */
    int m_account; //账户
    QString m_headshot; //头像地址
    QString m_nickname; //昵称
    QString m_signature; //个性签名

    /* 托盘 */
    QSystemTrayIcon * m_sysIcon; //托盘功能
    QAction* m_quit; //退出
    QAction* m_show; //恢复
    QMenu* m_menu; //托盘菜单signature; //个性签名

    /* 存储好友信息 */
    QStringList m_groupNames; //存放分组名称
    QMultiMap<QString,int> m_friends; //保存好友信息,Key为分组名，value为账号
    QHash<int,QString> m_frinicknames; //保存好友昵称
    QHash<int,QString> m_frisignatures; //保存好友个性签名

    /* 结点类型 */
    enum itemtype { engroup = 1001, enfriend };

    /* 好友列表右键菜单 */
    QMenu * m_frimenu; //好友右键菜单
    QAction * m_Chat; //与好友聊天
    QAction * m_delete; //删除好友
};

#endif // MAININTERFACE_H
