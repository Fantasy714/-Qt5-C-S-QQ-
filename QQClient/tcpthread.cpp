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

void TcpThread::LoginToServer(bool isfirst, int acc, QString pwd,bool isChecked)
{
    isFirstLogin = isfirst;
    m_account = acc;
    m_pwd = pwd;
    isRemember = isChecked;
    MsgToJson(LoginAcc);
}

void TcpThread::sendSearchFriMsgToSer(int acc)
{
    //设置目标账号
    MsgToJson(SearchFri,0,acc);
}

void TcpThread::sendFriAddMsgToSer(int myacc, int targetacc, QString type,QString yanzheng)
{
    qDebug() << "正在发送好友申请信息";
    MsgToJson(AddFri,myacc,targetacc,type,yanzheng);
}

void TcpThread::ChangeOnlineSta(int acc, QString onl)
{
    MsgToJson(ChangeOnlSta,acc,-1,onl);
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
    if(type == "注册")
    {
        MsgToJson(Registration,acc,0,pwd);
    }
    else
    {
        MsgToJson(FindPwd,acc,0,pwd);
    }
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
    int type = obj.value("type").toInt();
    switch(type)
    {
    case FindPwd:
    {
        qDebug() << "返回找回密码结果";
        QString pwd = obj.value("pwd").toString();
        emit sendResultToAccMsg("找回密码",pwd,"");
        break;
    }
    case Registration:
    {
        qDebug() << "返回注册结果";
        QString result = obj.value("result").toString();
        emit sendResultToAccMsg("注册","",result);
        break;
    }
    case LoginAcc:
    {
        qDebug() << "返回登录结果";
        bool isfirst = obj.value("isfirstlogin").toBool();
        QString result = obj.value("result").toString();
        if(result != "登录成功") // 如果不为登录成功则给注册界面发送信号
        {
            emit sendResultToLogin(result);
        }
        else //不然则为登录成功
        {
            qDebug() << "登录成功";
            QString signature = obj.value("signature").toString(); //获取用户个性签名
            QString nickname;
            m_file.setFileName(m_path + "/" + QString::number(m_account) + "/login.txt");
            if(!m_file.open(QFile::ReadWrite))
            {
                qDebug() << "文件不存在，等待文件初始化";
            }
            else //若打开成功则文件存在
            {
                QString loginD = m_file.readLine();
                m_file.close();
                QStringList loginDs = loginD.split("@@");
                /*
                 * login.txt文件格式
                 * 用户昵称,头像图片文件名称,是否记住密码,若记住则为密码，否则为空
                 */
                nickname = loginDs.at(0); //获取用户昵称
                //若文件中记录为记住则为真，否则为假
                bool fileRem = loginDs.at(2) == "记住" ? true : false;
                //若文件中记录与当前选择不相同才更改文件
                if(fileRem != isRemember)
                {
                    qDebug() << "正在更改配置文件" << "记住按钮选中状态" << isRemember;
                    QByteArray writeDatas;
                    QString loginData;
                    //昵称和头像名称不改变，只改变后两位是否记住和密码
                    if(isRemember)
                    {
                        loginData = loginDs.at(0) + "@@" + loginDs.at(1) + "@@" + "记住@@" + m_pwd;
                        qDebug() << m_pwd;
                    }
                    else
                    {
                        loginData = loginDs.at(0) + "@@" + loginDs.at(1) + "@@" + "不记住";
                    }
                    m_file.open(QFile::WriteOnly | QFile::Truncate);
                    writeDatas = loginData.toUtf8();
                    m_file.write(writeDatas);
                    m_file.close();
                }
            }
            if(isfirst) //若为第一次登录
            {
                qDebug() << "为第一次登录";

                //接收用户昵称
                QString mynickname = obj.value("nickname").toString();
                nickname = mynickname;

                QString accountS = QString::number(m_account);
                //创建用户数据文件夹
                if(!m_dir.mkdir(m_path + "/" + accountS))
                {
                    qDebug() << "文件夹创建失败";
                    return;
                }
                qDebug() << "文件夹创建成功，正在写入初始文件: " << accountS;

                //接收文件大小
                int size1 = obj.value("headshot_size").toInt();
                int size2 = obj.value("friends_size").toInt();

                qDebug() << size1 << ":" << size2;

                //读取文件
                QByteArray fileD1 = filedata.left(size1);
                QByteArray fileD2 = filedata.right(size2);

                QString fileN1 = m_path + "/" + accountS + "/" + accountS + ".jpg";
                QString fileN2 = m_path + "/" + accountS + "/friends.json";
                qDebug() << "头像名:" << fileN1 << "好友列表文件:" << fileN2;
                //创建用户头像
                m_file.setFileName(fileN1);
                m_file.open(QFile::WriteOnly);
                m_file.write(fileD1);
                m_file.close();

                //创建用户好友列表
                m_file.setFileName(fileN2);
                m_file.open(QFile::WriteOnly);
                m_file.write(fileD2);
                m_file.close();

                //创建登录界面初始化文件
                m_file.setFileName(m_path + "/" + accountS + "/login.txt");
                m_file.open(QFile::WriteOnly);
                QString msg = mynickname + "@@" + accountS + ".jpg@@";
                //若isRemember为真则是记住密码
                if(isRemember)
                {
                    msg += "记住@@";
                    msg += m_pwd;
                }
                else
                {
                    msg.append("不记住");
                }
                m_file.write(msg.toUtf8());
                m_file.close();
            }
            emit sendResultToMainInterFace(type,m_account,nickname,signature,"","");
        }
        break;
    }
    case SearchFri:
    {
        QString res = obj.value("result").toString();
        if(res == "查找失败")
        {
            qDebug() << "好友查找失败";
            emit sendResultToMainInterFace(type,-1,"","","查找失败","");
        }
        else
        {
            qDebug() << "查找成功";

            //获取用户信息
            QString uD = obj.value("userData").toString();
            //获取该用户账号
            QString friAcc = uD.split("@@").at(0);

            //若本地无该头像照片则将用户头像保存到本地
            QString friHdPath = m_alluserspath + "/" + friAcc + ".jpg";
            m_file.setFileName(friHdPath);
            if(!m_file.exists())
            {
                m_file.open(QFile::WriteOnly);
                m_file.write(filedata);
                m_file.close();
            }

            emit sendResultToMainInterFace(type,-1,"","","查找成功",uD);
        }
        break;
    }
    case AddFri:
    {
        QString result = obj.value("result").toString();
        qDebug() << result;
        if(result == "该好友已下线")
        {
            int targetAcc = obj.value("targetaccount").toInt();
            emit sendResultToMainInterFace(AddFri,targetAcc,"","",result,"");
        }
        else
        {
            QString msgType = obj.value("msgtype").toString();
            if(msgType == "发送好友申请")
            {
                //获取用户信息
                QString uD = obj.value("userData").toString();
                //获取该用户账号
                QString friAcc = uD.split("@@").at(0);
                //若本地无该头像照片则将用户头像保存到本地
                QString friHdPath = m_alluserspath + "/" + friAcc + ".jpg";
                m_file.setFileName(friHdPath);
                if(!m_file.exists())
                {
                    m_file.open(QFile::WriteOnly);
                    m_file.write(filedata);
                    m_file.close();
                }

                QString yanzheng = obj.value("yanzheng").toString();
                emit sendResultToMainInterFace(type,-1,"","","",uD,yanzheng,msgType);
            }
        }
        break;
    }
    case SendMsg:
    {
        //获取发送端账号和发送的信息及信息类型
        int acc = obj.value("acc").toInt();
        QString msgType = obj.value("msgType").toString();
        QString msg = obj.value("msg").toString();
        qDebug() << "收到来自: " << acc << " 的信息，信息类型为: " << msgType << " 信息内容: " << msg;

        //若为添加好友成功则取出好友昵称和个性签名
        if(msgType == "添加好友成功")
        {
            QString uD = obj.value("userData").toString();
            QStringList uDatas = uD.split("@@");
            //第一个信息为昵称，第二个为个性签名
            QString nickname = uDatas.at(0);
            QString signature = uDatas.at(1);

            sendResultToMainInterFace(type,acc,nickname,signature,"","",msg,msgType);
        }
        break;
    }
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

void TcpThread::MsgToJson(InforType type,int acc,int targetacc,QString Msg,QString yanzheng)
{
    QString fileName = "";
    QJsonObject obj;
    obj.insert("type",type);
    switch(type)
    {
    case Registration:
    {
        qDebug() << "注册中...";
        obj.insert("account",acc);
        obj.insert("pwd",Msg);
        break;
    }
    case FindPwd:
    {
        qDebug() << "找回密码中...";
        obj.insert("account",acc);
        break;
    }
    case LoginAcc:
    {
        qDebug() << "登录中...";
        obj.insert("isfirstlogin",isFirstLogin);
        obj.insert("account",m_account);
        obj.insert("pwd",m_pwd);
        break;
    }
    case SearchFri:
    {
        qDebug() << "查找好友中";
        obj.insert("account",targetacc);
        break;
    }
    case AddFri:
    {
        qDebug() << "发送好友申请信息";
        obj.insert("account",acc);
        obj.insert("targetaccount",targetacc);
        obj.insert("msgtype",Msg);
        obj.insert("yanzheng",yanzheng);
        break;
    }
    case ChangeOnlSta:
    {
        qDebug() << "改变在线状态";
        obj.insert("account",acc);
        obj.insert("onlinestatus",Msg);
        break;
    }
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
