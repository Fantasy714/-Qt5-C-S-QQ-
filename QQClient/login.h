#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <QMouseEvent>
#include <QMovie>
#include <QSystemTrayIcon>
#include "account.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QHBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

class Login : public QWidget
{
    Q_OBJECT

public:
    Login(QWidget *parent = nullptr);
    ~Login();
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void InitSysTrayicon(); //初始化系统托盘
    void ShowWindow(bool fromAcc = false); //显示主界面
    void isConnectingWithServer(bool onl); //是否连接到服务器
    void initUserData(); //初始化用户数据
    void initComboBox(); //初始化账户comboBox
    void deleteUserData(QString acc); //删除账号数据
    void closeSystemIcon(); //关闭托盘
    QPixmap CreatePixmap(QString picPath); //返回圆形头像
public slots:
    void GetResultFromSer(QString result); //接收服务器传回的结果

signals:
    void ToAccount(bool); //转到注册界面
    void LoginClose(); //发送关闭信号给tcp线程
    void LoginToServer(bool isfirst,int acc,QString pwd,bool isRem); //登录

private slots:
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason); //点击托盘图标

    void on_CloseToolBtn_clicked();

    void on_MiniToolBtn_clicked();

    void on_FindBtn_clicked();

    void on_NewAcBtn_clicked();

    void on_pushButton_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_2_clicked();

    void on_AccountLine_textChanged(const QString &arg1);

private:
    Ui::Login *ui;
    bool isMainWidget = false; //记录点下时是否在主窗口上而非内部控件上
    QPoint m_point; //记录点击时位置
    QMovie * m_movie; //加载动态图
    QSystemTrayIcon * m_sysIcon; //托盘功能
    QAction* m_quit; //退出
    QAction* m_show; //恢复
    QMenu* m_menu; //托盘菜单
    bool Registering = false; //是否在注册界面
    bool isConnecting = false; //是否已连接服务器
    bool isFirstLogin = false; //是否是第一次登录
    QString m_path = QCoreApplication::applicationDirPath() + "/userdata"; //保存用户数据的文件夹地址
    QDir m_dir; //操作目录
    QFile m_file; //操作文件
    QVector<int> m_dataLoc; //存放添加入comboBox的用户数据标号
    QStringList m_Accs; //存放用户账号
    QStringList m_Names; //存放用户名
    QStringList m_Pwds; //存放用户密码
    QStringList m_Icons; //存放用户头像位置
    QList<bool> isRemember; //记录是否记住密码
};
#endif // LOGIN_H
