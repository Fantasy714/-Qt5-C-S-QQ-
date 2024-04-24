#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QObject>
#include <QReadWriteLock>
#include <QTcpSocket>
#include "qqsqldata.h"
#include <QCoreApplication>
#include <QDir>
#include "global.h"


class WorkThread : public QObject
{
    Q_OBJECT
public:
    explicit WorkThread(QObject *parent = nullptr);
    void ParseMsg(QByteArray); //解析收到的数据
    void ReplyToJson(InforType type,QString pwd,QString result = "", QString fileName = "", int acc = 0, int targetacc = 0,QString MsgType = "",QString Msg = ""); //将信息转换为json
    void recvRegistered(int,QString); //查找注册结果
    void recvFind(int); //查找找回密码结果
    void CltLogin(int,QString); //客户端登录
    void SearchingFri(int acc); //查找好友
    void CltAddFri(int acc,int targetacc,QString msgType,QString yanzheng); //处理好友申请信息
    void CltChangeOnlSta(int acc,QString onlsta); //用户改变在线状态
    void ForwardInformation(int acc,int targetacc, QString msgType, QString msg,QString filePath = ""); //转发信息给其他客户端
    void AskForUserData(int acc,QString isSelf,int HeadShotSize = -1); //请求个人资料
    void ChangingUserDatas(int acc,QString datas);
signals:
    void ThreadbackMsg(InforType type,int account,QString msg,int target = 0); //从线程传消息回服务器
    void UserOnLine(int acc,quint16 sockport); //用户上线则发送该tcp套接字加入服务器在线用户哈希表中
    void SendMsgToClt(quint16 port,InforType type,int acc,int targetacc,QByteArray jsondata,QString fileName,QString msgtype); //发送信息给客户端
private:
    Qqsqldata sql; //连接数据库
    QTcpSocket * m_tcp; //tcp套接字

    QDir m_dir; //操作文件目录

    bool isFirstLogin = false; //是否是第一次登录
    QStringList m_userDatas; //保存用户资料

    const QString m_path = QCoreApplication::applicationDirPath() + "/usersdata"; //用户文件夹
};

#endif // WORKTHREAD_H
