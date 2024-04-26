#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QDateTime>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include "global.h"

//信息类型
enum MsgType { itsTime = 500, itsMsg, itsPicture, itsFile };

//记录鼠标位置
enum Location {
    Top_Left,
    Top,
    Top_Right,
    Right,
    Bottom_Right,
    Bottom,
    Bottom_Left,
    Left,
    Center
};

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWindow(int targetAcc,QString nickN,QWidget *parent = nullptr);
    ~ChatWindow();
    void FriendSendMsg(bool isMe,MsgType MsgType,QString Msg); //添加消息进消息框中
protected:
    void initShadow(); //初始化窗口边框阴影
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    QWidget* CreateWidget(bool isMe,MsgType MsgType,QString Msg); //创建自定义item
    int returnItemHeight(MsgType MsgType,int wLgh = -1); //返回item高度
    void ChangeCurSor(const QPoint &p); //更改鼠标样式
signals:
    void SendMsgToFri(int,MsgType,QString); //向朋友发送信息

private slots:
    void on_SendBtn_clicked();

    void on_BigBtn_clicked();

    void on_PicBtn_clicked();

private:
    Ui::ChatWindow *ui;
    bool isPressed = false; //记录鼠标是否按下
    QPoint m_point; //记录鼠标点下位置
    Location m_loc; //记录鼠标当前位置

    bool isMaxed = false; //是最大化状态，默认不是

    int m_targetAcc; //好友账号
    QString m_nickname; //好友昵称
    QString m_FriHeadShot; //好友的头像地址

    QDateTime LastMsgTime = QDateTime::fromString("2023-11-25 18:00:00", "yyyy-MM-dd hh:mm:ss"); //最后一次发送消息的时间，初始化为较早值，这样第一次消息便直接输出时间
};

#endif // CHATWINDOW_H
