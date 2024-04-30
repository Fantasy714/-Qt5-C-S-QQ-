#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QDateTime>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include "global.h"
#include <QProgressBar>
#include <QLabel>
#include <QTime>

//信息类型
enum MsgType { itsTime = 500, itsMsg, itsPicture, itsFile, RecvFile };

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
public slots:
    void FriendSendMsg(bool isMe,MsgType MsgType,QString Msg); //添加消息进消息框中
    void GetProgressInfo(int friAcc,QString fileName,int speed,int value,QTime RestTime,bool isMe); //接收文件收发进度信息
    void SendOrRecvFile(bool isMe,QString fileName); //发送或接收文件
protected:
    void initShadow(); //初始化窗口边框阴影
    bool eventFilter(QObject * w,QEvent * e) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    QWidget* CreateWidget(bool isMe,MsgType MsgType,QString Msg); //创建自定义item
    int returnItemHeight(MsgType MsgType,int wLgh = -1,int heightPic = -1); //返回item高度 /* 若传入图片则wLgh为宽，heightPic为高,文字wLgh为文字长度 */
    void ChangeCurSor(const QPoint &p); //更改鼠标样式
    QString GetFileSize(const int &size); //获取文件大小字符串
    QPixmap JudgeFileBack(QString fileName); //判断文件后缀，返回对应的文件类型图片
    void ChangefileListSta(bool isRecvOrSend); //文件接收栏状态,true为正在接收
signals:
    void SendMsgToFri(int,MsgType,QString); //向朋友发送信息

private slots:
    void on_SendBtn_clicked(); //发送文字

    void on_BigBtn_clicked(); //最大化

    void on_PicBtn_clicked(); //发送图片

    void on_FileBtn_clicked(); //发送文件

private:
    Ui::ChatWindow *ui;
    bool isPressed = false; //记录鼠标是否按下
    QPoint m_point; //记录鼠标点下位置
    Location m_loc; //记录鼠标当前位置

    //全局路径，防止鼠标移动事件穿透
    QPixmap m_pixmap;
    QPainterPath m_globalPath;

    bool isMaxed = false; //是最大化状态，默认不是

    int m_targetAcc; //好友账号
    QString m_nickname; //好友昵称
    QString m_FriHeadShot; //好友的头像地址

    QDateTime LastMsgTime = QDateTime::fromString("2023-11-25 18:00:00", "yyyy-MM-dd hh:mm:ss"); //最后一次发送消息的时间，初始化为较早值，这样第一次消息便直接输出时间

    QMap<QString,QListWidgetItem*> m_fileItems; //保存文件收发界面item
    QMap<QString,QProgressBar*> m_progressBars; //保存进度条
    QMap<QString,QLabel*> m_speedLab; //保存速度label
    QMap<QString,QLabel*> m_resttimeLab;  //保存剩余时间label
};

#endif // CHATWINDOW_H
