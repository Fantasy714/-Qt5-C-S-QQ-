#ifndef ADDFRIEND_H
#define ADDFRIEND_H

#include <QWidget>
#include <QMouseEvent>
#include "global.h"

namespace Ui {
class AddFriend;
}

class AddFriend : public QWidget
{
    Q_OBJECT

public:
    explicit AddFriend(bool type,QStringList gn, QStringList umsg,QString yanzheng = "", QWidget *parent = nullptr);
    ~AddFriend();
protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
signals:
    void CloseAddFriend(QString type = "",int acc = -1, QString GpNa = "",QString yanzheng = ""); //向主窗口发送好友申请操作信息

private slots:
    void on_NoBtn_clicked();

    void on_OkBtn_clicked();

private:
    Ui::AddFriend *ui;

    //添加好友窗口用户资料编号
    enum umsg { enacc = 0,ennickname,ensex,enage,enlocation };
    bool m_type; //true为发送好友申请界面，false为接收界面
    bool isMainWidget = false; //记录点下时是否在主窗口上而非内部控件上
    QPoint m_point; //记录鼠标点下位置

    /* 想添加的用户信息 */
    int m_acc; //账号
    QString m_nickname; //昵称
    QString m_sex; //性别
    QString m_age; //年龄
    QString m_location; //所在地
};

#endif // ADDFRIEND_H
