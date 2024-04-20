#ifndef MAININTERFACE_H
#define MAININTERFACE_H

#include <QMainWindow>
#include "login.h"
#include "account.h"
#include "tcpthread.h"
#include "addfriend.h"
#include <QCoreApplication>
#include <QDir>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMouseEvent>
#include "findfriends.h"
#include <QTreeWidgetItem>
#include "chatwindow.h"

namespace Ui {
class MainInterface;
}

class MainInterface : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainInterface(QWidget *parent = nullptr);
    ~MainInterface();
public slots:
    void ShowAccount(bool); //显示注册界面
    void GetResultFromSer(int type,int acc,QString nickname,QString signature,QString result,QString uData,QString Msg,QString MsgType); //获取服务器返回的数据
    void initSystemIcon(); //初始化系统托盘
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason); //点击托盘图标
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void ShowFindFri(); //显示查找好友窗口
    QPixmap CreatePixmap(QString picPath); //返回圆形头像
    void GetFriendsData(); //从本地数据文件中获取好友列表信息
    void InitTreeWidget(); //初始化好友列表TreeWidget
    void UpdateTreeWidget(); //更新好友列表TreeWidget
    QTreeWidgetItem *CreateTreeWidgetItem(QString fenzuming, int acc = -1); //返回节点指针
    void InitFriRitBtnMenu(); //初始化好友列表右键菜单
    void SearchingAcc(QString acc); //查找账号信息
    void AddFriendClosed(QString type,int acc,QString GpNa,QString yanzheng); //接收添加好友界面关闭信号
    void Reconnection(bool onl); //掉线时重新连接上线
signals:
    void StartConnecting(); //连接服务器
    void MainInterfaceClose(); //主窗口退出
    void SendReplyToFindFri(bool type); //将回应发送回查找好友界面
    void sendSearchFriMsgToSer(int acc); //向服务器发送查找好友信息
    void sendFriAddMsgToSer(int myacc, int targetacc,QString type,QString yanzheng = ""); //发送好友申请信息给服务器
    void ChangeOnlineSta(int acc, QString onl); //改变在线状态
private slots:
    void on_CloseBtn_clicked();

    void on_MiniBtn_clicked();

    void onTreeWidgetClicked(QTreeWidgetItem * item); //单击好友栏

    void onTreeWidgetDoubleClicked(QTreeWidgetItem * item); //双击好友栏

    void onItemExpended(QTreeWidgetItem * item); //好友列表展开

    void onItemCollapsed(QTreeWidgetItem * item); //好友列表收起

    void FriRightBtnMenu(const QPoint &pos); //好友列表右键菜单

    void AddGroup(); //添加分组

    void ReNameGroup(); //重命名分组

    void RemoveGroup(); //删除分组

    void ItemNameChanged(QTreeWidgetItem * item); //分组名改变

private:
    Ui::MainInterface *ui;

    //保存信息类型
    enum InforType { Registration = 1125, FindPwd, LoginAcc, SearchFri, AddFri, ChangeOnlSta, SendMsg };

    TcpThread * m_mytcp; //网络连接类
    QThread * thread; //tcp线程
    bool isLogined = false; //是否已登录到主界面

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
    FindFriends * m_FindFri; //查找好友界面
    QList<AddFriend*> m_addfri; //添加好友界面
    QHash<int,ChatWindow*> m_chatWindows; //聊天界面

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

    QMap<int,QString> m_waitFriReply; //存放添加好友的分组信息，等待好友同意后加入该分组

    /* 结点类型 */
    enum itemtype { engroup = 1001, enfriend };

    /* 好友列表右键菜单 */
    QMenu * m_frimenu; //好友右键菜单
    QAction * m_Chat; //与好友聊天
    QAction * m_delete; //删除好友

    /* 好友分组右键菜单 */
    QMenu * m_grpmenu; //分组右键菜单
    QAction * m_addgrp; //添加分组
    QAction * m_renamegrp; //重命名分组
    QAction * m_removegrp; //删除分组

    /* 存储更改分组时的信息 */
    QTreeWidgetItem * m_chaggrpitem = nullptr; //要更改的分组item
    QString m_chagName; //要更改的分组名
    bool isaddgrp; //是否是添加好友分组
    bool FriIsChanged = false; //好友列表是否已更改
};

#endif // MAININTERFACE_H
