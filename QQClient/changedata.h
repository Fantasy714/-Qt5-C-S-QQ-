#ifndef CHANGEDATA_H
#define CHANGEDATA_H

#include <QWidget>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include "global.h"

namespace Ui
{
    class ChangeData;
}

class ChangeData : public QWidget
{
    Q_OBJECT

public:
    explicit ChangeData(QStringList uD, QWidget* parent = nullptr);
    ~ChangeData();

signals:
    void CloseEdit(); //关闭更改窗口
    void ChangeUserDatas(QString); //更改用户资料

protected:
    bool eventFilter(QObject* w, QEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void initShadow(); //初始化窗口边框阴影

private slots:
    void TextChanged(); //文本改变

    void on_Save_clicked(); //保存

private:
    Ui::ChangeData* ui;

    QPoint m_point; //记录点击位置
    bool isPressed = false; //是否点下
    bool isChanged; //是否更改信息
};

#endif // CHANGEDATA_H
