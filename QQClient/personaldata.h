#ifndef PERSONALDATA_H
#define PERSONALDATA_H

#include <QWidget>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QGraphicsDropShadowEffect>
#include <QPainter>

//保存用户资料标号
enum UserMsg { Dnickname = 0,Dsignature,Dsex,Dage,Dbirthday,Dlocation,Dblood_type,Dwork,Dsch_comp };

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

private:
    Ui::PersonalData *ui;

    QPoint m_point; //记录点击位置
    bool isPressed; //是否点下

    QString m_path = QCoreApplication::applicationDirPath() + "/userdata"; //用户数据文件夹位置
    QString m_alluserspath = m_path + "/allusers"; //存放好友头像文件夹位置

    int m_acc; //账号
    QStringList m_UserData; //用户资料
};

#endif // PERSONALDATA_H
