#include "tcpthread.h"
#include <QHostAddress>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QtEndian>
#include <QTime>
#include <QCoreApplication>

TcpThread::TcpThread(QObject *parent) : QObject(parent)
{

}

void TcpThread::StartConnect()
{
    connectToServer();
    //等待套接字连接服务器
    m_tcp->waitForConnected(100);
    //如果未连接成功则启用自动重连
    if(isConnecting == false)
    {
        qDebug() << "正在尝试自动重连";
        if(m_timer == nullptr)
        {
            m_timer = new QTimer;
            connect(m_timer,&QTimer::timeout,this,&TcpThread::AutoConnect);
        }
        m_timer->start(3000);
    }
}

void TcpThread::GetClose()
{
    if(m_timer != nullptr)
    {
        if(m_timer->isActive())
        {
            qDebug() << "定时器已暂停";
            m_timer->stop();
        }
        delete m_timer;
    }
    m_tcp->close();
    m_tcp->deleteLater();
    qDebug() << "线程已退出";
}

void TcpThread::LoginToServer(bool isfirst, int acc, QString pwd)
{
    m_type = "登录";
    isFirstLogin = isfirst;
    m_account = acc;
    m_pwd = pwd;
    MsgToJson();
}

void TcpThread::connectToServer()
{
    qDebug() << "tcp套接字线程ID:" << QThread::currentThreadId();
    if(m_tcp == nullptr)
    {
        m_tcp = new QTcpSocket;
        connect(m_tcp,&QTcpSocket::connected,this,&TcpThread::ConnectSuccess);
        connect(m_tcp,&QTcpSocket::readyRead,this,&TcpThread::ReadMsgFromServer);
        connect(m_tcp,&QTcpSocket::disconnected,this,&TcpThread::DisconnectFromServer);
    }
    m_tcp->connectToHost(QHostAddress(address),port);
}

void TcpThread::recvAccMsg(QString type,int acc, QString pwd)
{
    m_type = type;
    m_account = acc;
    m_pwd = pwd;
    MsgToJson();
}

void TcpThread::ConnectSuccess()
{
    qDebug() << "连接服务器成功!连接服务器的线程ID为:" << QThread::currentThreadId();
    isConnecting = true;
    emit isConnectingWithServer(true);
    //第一次就连接成功则直接返回
    if(m_timer == nullptr)
    {
        return;
    }
    else if(m_timer->isActive()) //否则关闭定时器
    {
        m_timer->stop();
    }
}

void TcpThread::ReadMsgFromServer()
{
    unsigned int totalBytes = 0;
    unsigned int jsonBytes = 0;
    unsigned int recvBytes = 0;
    QByteArray jsondata;
    QByteArray filedata;

    if(m_tcp->bytesAvailable() == 0) //判断有没有数据;
    {
        qDebug() << "无数据或数据已读完";
        return;
    }
    if(m_tcp->bytesAvailable() >= 8)//读取包头
    {
        QByteArray head = m_tcp->read(4);
        totalBytes = qFromBigEndian(*(int*)head.data());
        qDebug() << "接收到数据的总长度:" << totalBytes;
        /*
         * 总长度构成
         * 有文件
         * json数据段长度___json数据段___文件数据长度
         * 无文件
         * json数据段长度___json数据段
         * 因此使用时需将totalBytes减去json数据段包头才为所有有效数据的总长
         */
        totalBytes = totalBytes - 4;
        qDebug() << "接收到数据的总长度减去json数据段包头长度:" << totalBytes;
        QByteArray jsonhead = m_tcp->read(4);
        jsonBytes = qFromBigEndian(*(int*)jsonhead.data());
        qDebug() << "接收到的json数据长度:" << jsonBytes;
    }
    else
    {
        return;
    }
    //如果有数据并且json数据段未读完
    while(m_tcp->bytesAvailable() && recvBytes < jsonBytes)
    {
        jsondata.append(m_tcp->read(jsonBytes - recvBytes));
        recvBytes = jsondata.size();
    }
    //如果还有数据则为文件数据，没有则总长与json数据段长相同，不进入该while循环
    while(m_tcp->bytesAvailable() && recvBytes < totalBytes)
    {
        filedata.append(m_tcp->read(totalBytes - recvBytes));
        recvBytes = filedata.size() + jsonBytes;
    }
    if(recvBytes == totalBytes)//数据包读取完毕
    {
        ParseMsg(jsondata,filedata);
    }
    if(m_tcp->bytesAvailable())
    {
        ReadMsgFromServer();
    }
}

void TcpThread::ParseMsg(QByteArray data,QByteArray filedata)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    QString type = obj.value("type").toString();
    if(type == "找回密码")
    {
        qDebug() << "返回找回密码结果";
        QString pwd = obj.value("pwd").toString();
        emit sendResultToAccMsg("找回密码",pwd,"");
    }
    else if(type == "注册")
    {
        qDebug() << "返回注册结果";
        QString result = obj.value("result").toString();
        emit sendResultToAccMsg("注册","",result);
    }
}

void TcpThread::DisconnectFromServer()
{
    qDebug() << "服务器断开连接";
    isConnecting = false;
    m_tcp->close();
    emit isConnectingWithServer(false);
    //掉线自动重连
    if(m_timer == nullptr)
    {
        m_timer = new QTimer;
        connect(m_timer,&QTimer::timeout,this,&TcpThread::AutoConnect);
    }
    m_timer->start(3000);
}

void TcpThread::MsgToJson()
{
    QString fileName = "";
    QJsonObject obj;
    obj.insert("type",m_type);
    if(m_type == "注册")
    {
        qDebug() << "注册中...";
        obj.insert("account",m_account);
        obj.insert("pwd",m_pwd);
    }
    else if(m_type == "找回密码")
    {
        qDebug() << "找回密码中...";
        obj.insert("account",m_account);
    }
    else if(m_type == "登录")
    {
        qDebug() << "登录中...";
        obj.insert("isfirstlogin",isFirstLogin);
        obj.insert("account",m_account);
        obj.insert("pwd",m_pwd);
    }
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();
    //将发送数据的大小转换成大端后添加表头
    int len = qToBigEndian(data.size());
    QByteArray senddata((char*)&len,4);
    senddata.append(data);

    SendToServer(senddata,fileName);
}

void TcpThread::SendToServer(QByteArray jsondata, QString fileName)
{
    QByteArray data(jsondata);
    if(fileName != "")
    {
        qDebug() << "要发送文件";
    }
    //给所有数据数据添加表头
    int size = qToBigEndian(data.size());
    QByteArray alldata((char*)&size,4);
    alldata.append(data);

    m_tcp->write(alldata);
    qDebug() << "已发送信息";
}

void TcpThread::AutoConnect()
{
    connectToServer();
    m_tcp->waitForConnected(100);
    if(isConnecting == true)
    {
        m_timer->stop();
    }
}
