#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include "qqsqldata.h"
#include "global.h"

class WorkThread : public QObject
{
    Q_OBJECT
public:
    explicit WorkThread(QObject *parent = nullptr);
    ~WorkThread();
public slots:
    void SplitDataPackAge(QByteArray,quint16); //拆分数据包
protected:
    //解析和转换
    void ParseMsg(quint16,QByteArray); //解析收到的数据
    void ReplyToJson(InforType type,QString Msg, QString MsgType = "",int acc = -1,int targetacc = -1,QString fileName = ""); //将回应信息转为json格式

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

    QString GetFileSize(const int &size); //获取文件大小字符串
    void CltAddFri(int acc,int targetacc,QString msgType,QString yanzheng); //处理好友申请信息
signals:
    void ThreadbackMsg(int type,int account,QString msg); //传消息回服务器
    void UserOnLine(int acc,quint16 sockport); //用户上线则发送该tcp套接字加入服务器在线用户哈希表中
    void SendMsgToClt(quint16 port,int type,int acc,int targetacc,QByteArray jsondata,QString msgtype,QString fileName); //发送信息给客户端
private:
    Qqsqldata sql; //连接数据库
    quint16 m_port; //客户端套接字端口号
    QStringList m_userDatas; //保存用户资料

    /* 文件操作 */
    QHash<quint16,int> FilePackAgeCount; //已接收数据包个数
    QHash<quint16,QFile*> m_RecvFiles; //操作文件,key为文件发送方端口号
    QHash<quint16,quint32> m_FileSizes; //保存文件大小
};

#endif // WORKTHREAD_H
