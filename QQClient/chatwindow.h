#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QListWidgetItem>
#include <QDateTime>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(int acc,int targetAcc,QString nickN,QWidget *parent = nullptr);
    ~ChatWindow();
    void FriendSendMsg(bool isMe,QString MsgType,QString Msg); //添加消息进消息框中
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    QPixmap CreatePixmap(QString picPath); //返回圆形头像
    QWidget* CreateWidget(bool isMe,QString MsgType, QString Msg); //创建消息Widget

private slots:
    void on_SendBtn_clicked();

private:
    Ui::ChatWindow *ui;
    bool isMainWidget = false; //记录点下时是否在主窗口上而非内部控件上
    QPoint m_point; //记录鼠标点下位置

    int m_account; //自己的账号
    int m_targetAcc; //好友账号
    QString m_nickname; //好友昵称

    QString m_Mypath; //用户数据文件夹位置
    QString m_MyHeadShot; //自己的头像地址
    QString m_FriHeadShot; //好友的头像地址

    QDateTime LastMsgTime = QDateTime::fromString("2023-11-25 18:00:00", "yyyy-MM-dd hh:mm:ss"); //最后一次发送消息的时间，初始化为较早值，这样第一次消息便直接输出时间
};

#endif // CHATWINDOW_H
