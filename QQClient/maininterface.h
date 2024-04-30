#ifndef MAININTERFACE_H
#define MAININTERFACE_H

#include <QWidget>
#include "login.h"
#include "account.h"
#include "tcpthread.h"
#include "addfriend.h"
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMouseEvent>
#include "findfriends.h"
#include <QTreeWidgetItem>
#include "chatwindow.h"
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include "personaldata.h"
#include "changedata.h"
#include "global.h"

namespace Ui {
class MainInterface;
}

class MainInterface : public QWidget
{
    Q_OBJECT

public:
    explicit MainInterface(QWidget *parent = nullptr);
    ~MainInterface();
public slots:
    void ShowAccount(bool); //显示注册界面

    /* 网络相关 */
    void GetResultFromSer(int type,int targetAcc,QString uData,QString Msg,QString MsgType); //获取服务器返回的数据
    void Reconnection(bool onl); //掉线时重新连接上线

    /* 好友添加相关 */
    void ShowFindFri(); //显示查找好友窗口
    void SearchingAcc(QString acc); //查找账号信息
    void AddFriendClosed(QString type,int targetacc,QString GpNa,QString yanzheng); //接收添加好友界面关闭信号

    /* 好友聊天相关 */
    ChatWindow* showFriChatWindow(int acc); //显示好友聊天窗口
    void SendMsgToFri(int,MsgType,QString); //发送信息给好友

    /* 更改用户信息 */
    void ChangingLoginFile(QString NkN = "", QString pwd = ""); //更改登录文件
    void ChangingHeadShot(); //修改头像

    /* 个人资料 */
    void ClosePerData(int); //关闭个人资料窗口
    void EditPersonalData(QStringList); //修改个人资料窗口
    void ChangeUserDatas(QString); //更改用户资料
    void CloseEdit(); //关闭编辑窗口
protected:
    /* 系统托盘 */
    void initSystemIcon(); //初始化系统托盘
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason); //点击托盘图标

    //界面相关
    void ChangeCurSor(const QPoint &p); //更改鼠标样式
    void initShadow(); //初始化窗口边框阴影
    bool eventFilter(QObject * w,QEvent * e) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    /* 好友列表 */
    void GetFriendsData(); //从本地数据文件中获取好友列表信息
    void InitTreeWidget(); //初始化好友列表TreeWidget
    void InitFriRitBtnMenu(); //初始化好友列表右键菜单
    void UpdateTreeWidget(); //更新好友列表TreeWidget
    QTreeWidgetItem *CreateTreeWidgetItem(QString fenzuming, int acc = -1); //创建好友列表节点指针
    void DelFri(); //删除好友
signals:
    void StartConnecting(); //连接服务器
    void MainInterfaceClose(); //主窗口退出
    void SendReplyToFindFri(bool type); //将回应发送回查找好友界面

    //发送信息给服务器
    void SendMsgToServer(int type,int acc = -1,int targetacc = -1,QString Msg = "",QString MsgType = "");
private slots:
    void on_CloseBtn_clicked(); //关闭

    void on_MiniBtn_clicked(); //最小化

    void onTreeWidgetClicked(QTreeWidgetItem * item); //单击好友栏

    void onTreeWidgetDoubleClicked(QTreeWidgetItem * item); //双击好友栏

    void onItemExpended(QTreeWidgetItem * item); //好友列表展开

    void onItemCollapsed(QTreeWidgetItem * item); //好友列表收起

    void FriRightBtnMenu(const QPoint &pos); //好友列表右键菜单

    void AddGroup(); //添加分组

    void ReNameGroup(); //重命名分组

    void RemoveGroup(); //删除分组

    void ItemNameChanged(QTreeWidgetItem * item); //分组名改变

    void on_HeadShotBtn_clicked(); //点击自己的头像

    void MoveFriend(); //移动好友

private:
    Ui::MainInterface *ui;

    TcpThread * m_mytcp; //网络连接类
    QThread * thread; //tcp线程
    bool isLogined = false; //是否已登录到主界面

    /* 窗口跟随鼠标移动拖动 */
    QPoint m_point; //记录鼠标点下位置
    bool isPressed = false; //记录是否点下鼠标
    Location m_loc; //记录鼠标当前位置
    int TabWidgetWidth; //记录TabWidget的宽度
    //全局路径，防止鼠标移动事件穿透
    QPixmap m_pixmap;
    QPainterPath m_globalPath;

    /* 其他界面 */
    Login * m_log; //登录界面
    Account * m_accClass; //注册界面
    FindFriends * m_FindFri; //查找好友界面
    QList<AddFriend*> m_addfri; //添加好友界面
    QHash<int,ChatWindow*> m_chatWindows; //聊天界面
    QHash<int,PersonalData*> m_PersonData; //个人资料界面
    ChangeData * m_editData = nullptr; //修改个人资料界面

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
    QAction * m_FriData; //查看好友资料
    QMenu * m_moveFri; //移动联系人
    QMap<QString,QAction*> m_movegroups; //可移动的分组
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

    /* 保存选择的好友item */
    QTreeWidgetItem * m_friItem = nullptr; //选择的好友item
};

#endif // MAININTERFACE_H
