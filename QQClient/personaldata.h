#ifndef PERSONALDATA_H
#define PERSONALDATA_H

#include <QWidget>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include "global.h"

namespace Ui
{
    class PersonalData;
}

class PersonalData : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalData(bool isMe, int acc, QStringList UserData, QWidget* parent = nullptr);
    ~PersonalData();
signals:
    void ClosePerData(int); //关闭个人资料界面
    void ChangingData(QStringList); //更改个人资料
    void ChangingHeadShot(); //更改头像

protected:
    bool eventFilter(QObject* w, QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void initShadow(); //初始化窗口边框阴影
    void CutPhoto(QString path); //剪切图片

private slots:
    void on_CloseBtn_clicked(); //关闭

    void on_EditBtn_clicked(); //编辑

    void on_HeadShotBtn_clicked(); //点击头像

private:
    Ui::PersonalData* ui;

    QPoint m_point; //记录点击位置
    bool isPressed; //是否点下

    bool m_isMe; //是否是自己的资料
    int m_acc; //账号
    QStringList m_UserData; //用户资料
};

#endif // PERSONALDATA_H
