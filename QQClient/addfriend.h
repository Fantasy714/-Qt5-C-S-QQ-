#ifndef ADDFRIEND_H
#define ADDFRIEND_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class AddFriend;
}

class AddFriend : public QWidget
{
    Q_OBJECT

public:
    explicit AddFriend(QWidget *parent = nullptr);
    ~AddFriend();
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    Ui::AddFriend *ui;
    bool isMainWidget = false; //记录点下时是否在主窗口上而非内部控件上
    QPoint m_point; //记录鼠标点下位置
};

#endif // ADDFRIEND_H
