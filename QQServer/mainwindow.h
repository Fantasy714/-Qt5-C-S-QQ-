#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "qqsqldata.h"
#include "workthread.h"
#include "tcpthread.h"
#include "global.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
public slots:
    void ServerhasNewConnection(quint16); //服务器有新连接
    void CltDisConnected(quint16, int); //客户端断开连接
public slots:
    void getThreadMsg(int type, int account, QString msg); //从工作线程中获取客户端的请求内容
signals:
    void StartListen(); //开始监听
private:
    Ui::MainWindow* ui;

    Qqsqldata m_sqldata; //数据库

    QThread* m_TcpTd; //套接字线程
    TcpThread* m_TcpTask; //套接字线程任务类

    QThread* m_WorkTd; //工作线程
    WorkThread* m_WorkTask; //工作线程任务类
};
#endif // MAINWINDOW_H
