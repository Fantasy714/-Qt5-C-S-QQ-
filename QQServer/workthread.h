#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QObject>
#include <QReadWriteLock>
#include <QTcpSocket>
#include "qqsqldata.h"
#include <QCoreApplication>
#include "global.h"


class WorkThread : public QObject
{
    Q_OBJECT
public:
    explicit WorkThread(QObject *parent = nullptr);
public slots:
    void ParseMsg(quint16,QByteArray); //解析收到的数据
protected:
    void ReplyToJson(InforType type,QString Msg, QString MsgType = "",int acc = -1,int targetacc = -1,QString fileName = ""); //将信息转换为json

    /* 操作用户未登录 */
    void recvRegistered(int,QString); //查找注册结果
    void recvFind(int); //查找找回密码结果
    void CltLogin(int,QString,bool); //客户端登录

    /* 操作用户已登录 */
    //直接返回请求结果
    void SearchingFri(int acc); //查找好友
    void CltChangeOnlSta(int acc,QString onlsta); //用户改变在线状态
    void AskForUserData(int acc,QString isSelf,int HeadShotSize = -1); //请求个人资料
    void ChangingUserDatas(int acc,QString datas); //修改个人资料

    //需转发信息给其他客户端
    void CltAddFri(int acc,int targetacc,QString msgType,QString yanzheng); //处理好友申请信息
    void ForwardInformation(int acc,int targetacc, QString msgType, QString msg,QString filePath = ""); //转发信息给其他客户端
signals:
    void ThreadbackMsg(InforType type,int account,QString msg,int target = 0); //从线程传消息回服务器
    void UserOnLine(int acc,quint16 sockport); //用户上线则发送该tcp套接字加入服务器在线用户哈希表中
    void SendMsgToClt(quint16 port,int type,int acc,int targetacc,QByteArray jsondata,QString msgtype,QString fileName); //发送信息给客户端
private:
    Qqsqldata sql; //连接数据库
    quint16 m_port; //tcp套接字端口号

    const QString m_path = QCoreApplication::applicationDirPath() + "/usersdata"; //用户数据文件夹

    QStringList m_userDatas; //保存用户资料
};

#endif // WORKTHREAD_H
