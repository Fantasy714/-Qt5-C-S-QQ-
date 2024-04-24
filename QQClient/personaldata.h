#ifndef PERSONALDATA_H
#define PERSONALDATA_H

#include <QWidget>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include "global.h"

namespace Ui {
class PersonalData;
}

class PersonalData : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalData(bool isMe,int acc,QStringList UserData,QWidget *parent = nullptr);
    ~PersonalData();
signals:
    void ClosePerData(int);
    void ChangingData(QStringList);
    void ChangingHeadShot(QString);

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    QPixmap CreatePixmap(QString picPath); //返回圆形头像
    void initShadow(); //初始化窗口边框阴影
    void paintEvent(QPaintEvent *event) override;

private slots:
    void on_CloseBtn_clicked();

    void on_EditBtn_clicked();

    void on_HeadShotBtn_clicked();

private:
    Ui::PersonalData *ui;

    QPoint m_point; //记录点击位置
    bool isPressed; //是否点下

    bool m_isMe; //是否是自己的资料
    int m_acc; //账号
    QStringList m_UserData; //用户资料

    const QString m_path = QCoreApplication::applicationDirPath() + "/userdata";
    const QString m_alluserspath = QCoreApplication::applicationDirPath() + "/userdata/allusers";
    QString m_userpath = QCoreApplication::applicationDirPath() + "/userdata";
};

#endif // PERSONALDATA_H
