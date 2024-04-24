#ifndef CHANGEDATA_H
#define CHANGEDATA_H

#include <QWidget>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include "global.h"
#include <QCoreApplication>

namespace Ui {
class ChangeData;
}

class ChangeData : public QWidget
{
    Q_OBJECT

public:
    explicit ChangeData(QStringList uD, QWidget *parent = nullptr);
    ~ChangeData();

signals:
    void CloseEdit(); //关闭更改窗口
    void ChangeUserDatas(QString); //更改用户资料

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void initShadow(); //初始化窗口边框阴影
    void paintEvent(QPaintEvent *event) override;

private slots:
    void TextChanged();

    void on_Save_clicked();

private:
    Ui::ChangeData *ui;

    QPoint m_point; //记录点击位置
    bool isPressed; //是否点下
    bool isChanged; //是否更改信息

    const QString m_path = QCoreApplication::applicationDirPath() + "/userdata";
    const QString m_alluserspath = QCoreApplication::applicationDirPath() + "/userdata/allusers";
    QString m_userpath = QCoreApplication::applicationDirPath() + "/userdata";
};

#endif // CHANGEDATA_H
