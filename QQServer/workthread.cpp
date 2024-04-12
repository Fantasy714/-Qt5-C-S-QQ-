#include "workthread.h"
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFileInfo>

QReadWriteLock WorkThread::mutex;

WorkThread::WorkThread(QTcpSocket * tcp,QObject *parent) : QObject(parent), QRunnable()
{
    m_tcp = tcp;
}

void WorkThread::run()
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
        qDebug() << "数据包读取完毕";
        recvData(jsondata,filedata);
    }
    if(m_tcp->bytesAvailable()) //若数据未读取完则继续读取
    {
        run();
    }
}

void WorkThread::recvData(QByteArray data,QByteArray filedata)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    QString type = obj.value("type").toString();
    int account;
    QString pwd;
    bool isFirst;
    if(type == "找回密码")
    {
        account = obj.value("account").toInt();
        qDebug() << "客户端找回密码,需找回密码账号为" << account;
        recvFind(account);
    }
    else if(type == "注册")
    {
        account = obj.value("account").toInt();
        pwd = obj.value("pwd").toString();
        qDebug() << "客户端注册,注册账号:" << account << "密码:" << pwd;
        recvRegistered(account,pwd);
    }
    else if(type == "登录")
    {
        qDebug() << "客户端请求登录";
        isFirst = obj.value("isfirstlogin").toBool();
        isFirstLogin = isFirst;
        account = obj.value("account").toInt();
        pwd = obj.value("pwd").toString();
        CltLogin(account,pwd);
    }
}

void WorkThread::ReplyToJson(QString type, QString pwd, QString result,QString fileName,int acc)
{
    QString sendFileName;
    //qDebug() << "开始回应客户端";
    QJsonObject obj;
    obj.insert("type",type);
    if(type == "找回密码")
    {
        obj.insert("pwd",pwd);
    }
    else if(type == "注册")
    {
        obj.insert("result",result);
    }
    else if(type == "登录")
    {
        obj.insert("isfirstlogin",isFirstLogin);
        obj.insert("result",result);
        if(isFirstLogin)
        {
            //如果为第一次登录则发送账号资料，头像和好友信息

            //发送头像和好友信息
            QString fileN1 = m_path + "/" + QString::number(acc) + "/" + QString::number(acc) + ".jpg"; //头像
            QString fileN2 = m_path + "/" + QString::number(acc) + "/" + "friends.json"; //好友列表
            qDebug() << "filename1:" << fileN1 << "filename2:" << fileN2;

            //获取图片文件大小
            QFileInfo info(fileN1);
            int size1 = info.size();

            //获取好友列表文件大小
            info.setFile(fileN2);
            int size2 = info.size();

            //将两个文件的大小加入json数据中
            obj.insert("headshot_size",size1);
            obj.insert("friends_size",size2);

            //将待发送的文件名使用?（文件名无法以?命名）隔断传给发送函数
            sendFileName = fileN1 + "?" + fileN2;
            qDebug() << "要发送的文件名:" << sendFileName;
        }
    }
    QJsonDocument doc(obj);
    QByteArray reply = doc.toJson();
    //将发送数据的大小转换成大端后添加表头
    int len = qToBigEndian(reply.size());
    QByteArray senddata((char*)&len,4);
    senddata.append(reply);

    SendReply(senddata,sendFileName);
}

void WorkThread::SendReply(QByteArray jsondata, QString fileName)
{
    QByteArray data(jsondata);
    if(fileName != "")
    {
        qDebug() << "要发送文件";
        if(isFirstLogin)
        {
            qDebug() << "开始读取文件";
            //拆分开文件名
            QStringList fileNames = fileName.split("?");

            //获取两个文件的文件名
            QString fileN1 = fileNames.at(0);
            QString fileN2 = fileNames.at(1);

            //读取头像文件
            QFile file(fileN1);
            file.open(QFile::ReadOnly);
            QByteArray fileD1 = file.readAll();
            file.close();

            //读取好友列表文件
            file.setFileName(fileN2);
            file.open(QFile::ReadOnly);
            QByteArray fileD2 = file.readAll();
            file.close();

            //将两个文件添加入待发送数据中
            data.append(fileD1);
            data.append(fileD2);
        }
    }
    //给所有数据数据添加表头
    int size = qToBigEndian(data.size());
    QByteArray alldata((char*)&size,4);
    alldata.append(data);

    m_tcp->write(alldata);
    m_tcp->flush(); //将数据立刻发出
    qDebug() << "已发送信息:" << size;
}

void WorkThread::recvRegistered(int acc, QString pwd)
{
    QWriteLocker lock(&mutex);
    QString iconName = sql.Addaccount(acc,pwd);
    //如果返回的头像名为空则为注册失败
    if(iconName == "")
    {
        qDebug() << "账号重复,注册失败!";
        ReplyToJson("注册","","注册失败");
        ThreadbackMsg("注册",acc,"注册失败");
    }
    else
    {
        qDebug() << "注册成功!";
        ReplyToJson("注册","","注册成功");
        ThreadbackMsg("注册",acc,"注册成功");
        //如果注册成功则创建以该用户账号为名的文件夹与好友列表文件和头像
        QString path = m_path + "/" + QString::number(acc);
        m_dir.mkdir(path);
        //创建好友列表文件
        QString pathJ = path + "/friends.json";
        QFile file(pathJ);
        file.open(QFile::WriteOnly);
        file.close();
        //读取服务器随机分配的头像图片
        file.setFileName(iconName);
        file.open(QFile::ReadOnly);
        QByteArray pic = file.readAll();
        file.close();
        //创建用户头像
        file.setFileName(path + "/" + QString::number(acc) + ".jpg");
        file.open(QFile::WriteOnly);
        file.write(pic);
        file.close();
    }
}

void WorkThread::recvFind(int acc)
{
    QReadLocker lock(&mutex);
    QString rtpwd = sql.FindPwd(acc);
    if(rtpwd == "")
    {
        qDebug() << "无该账户!密码找回失败!";
        ReplyToJson("找回密码","");
    }
    else
    {
        qDebug() << "密码找回成功!";
        ReplyToJson("找回密码",rtpwd);
    }
    ThreadbackMsg("找回密码",acc,rtpwd);
}

void WorkThread::CltLogin(int acc, QString pwd)
{
    QWriteLocker lock(&mutex);
    int result = sql.LoginVerification(acc,pwd);
    if(result == -1) //账号或密码错误
    {
        qDebug() << "账号密码错误";
        ReplyToJson("登录","","账号密码错误");
        ThreadbackMsg("登录",acc,"账号或密码错误");
    }
    else if(result == 0) //重复登录
    {
        qDebug() << "重复登录";
        ReplyToJson("登录","","重复登录");
        ThreadbackMsg("登录",acc,"重复登录");
    }
    else //登录成功
    {
        qDebug() << "登录成功";
        quint16 port = m_tcp->peerPort(); //传递该套接字端口号给主界面
        emit UserOnLine(acc,port);
        sql.UserMessages(acc);
        ReplyToJson("登录","","登录成功","",acc);
        ThreadbackMsg("登录",acc,"登录成功");
    }
}
