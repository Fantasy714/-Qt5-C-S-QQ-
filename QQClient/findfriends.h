#ifndef FINDFRIENDS_H
#define FINDFRIENDS_H

#include <QWidget>
#include <QMouseEvent>
#include <QMutex>
#include <QCloseEvent>

namespace Ui {
class FindFriends;
}

class FindFriends : public QWidget
{
    Q_OBJECT

public:
    static FindFriends* createFindFriends(); //单例模式
    ~FindFriends();
    FindFriends(FindFriends &) = delete; //单例模式(删除拷贝构造和赋值构造)
    FindFriends& operator=(const FindFriends&) = delete;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_MiniBtn_clicked();

private:
    Ui::FindFriends *ui;
    explicit FindFriends(QWidget *parent = nullptr); //单例模式
    static FindFriends * m_Fri; //单例模式
    static QMutex m_mutex; //锁
    bool isMainWidget = false; //记录点下时是否在主窗口上而非内部控件上
    QPoint m_point; //记录鼠标点下位置
};

#endif // FINDFRIENDS_H
