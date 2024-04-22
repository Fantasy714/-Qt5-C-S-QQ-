#include "workthread.h"
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFileInfo>

WorkThread::WorkThread(QReadWriteLock * mtx,QTcpSocket * tcp,QObject *parent) : QObject(parent), QRunnable()
{
    mutex = mtx;
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
        ParseMsg(jsondata,filedata);
    }
    if(m_tcp->bytesAvailable()) //若数据未读取完则继续读取
    {
        run();
    }
}

void WorkThread::ParseMsg(QByteArray data,QByteArray filedata)
{
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    int type = obj.value("type").toInt();
    int account;
    int targetacc;
    QString pwd;
    switch(type)
    {
    case FindPwd:
    {
        account = obj.value("account").toInt();
        qDebug() << "客户端找回密码,需找回密码账号为" << account;
        recvFind(account);
        break;
    }
    case Registration:
    {
        account = obj.value("account").toInt();
        pwd = obj.value("pwd").toString();
        qDebug() << "客户端注册,注册账号:" << account << "密码:" << pwd;
        recvRegistered(account,pwd);
        break;
    }
    case LoginAcc:
    {
        qDebug() << "客户端请求登录";
        isFirstLogin = obj.value("isfirstlogin").toBool();
        account = obj.value("account").toInt();
        pwd = obj.value("pwd").toString();
        CltLogin(account,pwd);
        break;
    }
    case SearchFri:
    {
        qDebug() << "用户查找好友中";
        account = obj.value("account").toInt();
        SearchingFri(account);
        break;
    }
    case AddFri:
    {
        qDebug() << "收到好友申请";
        account = obj.value("account").toInt();
        targetacc = obj.value("targetaccount").toInt();
        QString msgType = obj.value("msgtype").toString();
        QString yanzheng = obj.value("yanzheng").toString();
        CltAddFri(account,targetacc,msgType,yanzheng);
        break;
    }
    case ChangeOnlSta:
    {
        qDebug() << "改变在线状态";
        account = obj.value("account").toInt();
        QString onlsta = obj.value("onlinestatus").toString();
        CltChangeOnlSta(account,onlsta);
        break;
    }
    case SendMsg:
    {
        qDebug() << "转发信息";
        int acc = obj.value("account").toInt();
        int targetAcc = obj.value("targetacc").toInt();
        QString msgType = obj.value("msgtype").toString();
        QString msg = obj.value("msg").toString();
        ForwardInformation(acc,targetAcc,msgType,msg);
        break;
    }
    }
}

void WorkThread::ReplyToJson(int type, QString pwd, QString result,QString fileName,int acc,int targetacc,QString MsgType,QString Msg)
{
    QString sendFileName = "";
    //qDebug() << "开始回应客户端";
    QJsonObject obj;
    QString msgtype = "";
    obj.insert("type",type);
    switch(type)
    {
    case FindPwd:
    {
        obj.insert("pwd",pwd);
        break;
    }
    case Registration:
    {
        obj.insert("result",result);
        break;
    }
    case LoginAcc:
    {
        obj.insert("isfirstlogin",isFirstLogin);
        obj.insert("result",result);
        if(result == "登录成功")
        {
            obj.insert("signature",m_userDatas.at(ensignature));
            if(isFirstLogin)
            {
                //将信息类型设置为第一次登录
                msgtype = "第一次登录";
                //如果为第一次登录则发送账号昵称，头像和好友信息
                obj.insert("nickname",m_userDatas.at(ennickname));

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
        break;
    }
    case SearchFri:
    {
        obj.insert("result",result);
        //如果查找成功则添加用户信息及头像
        if(result == "查找成功")
        {
            sendFileName = fileName;

            //将需要的几个用户资料添加到字符串中并用@@隔开
            QString uData = QString::number(acc) + "@@" + m_userDatas.at(ennickname) + "@@" +
                    m_userDatas.at(ensex) + "@@" + m_userDatas.at(enage) + "@@" + m_userDatas.at(enlocation);

            obj.insert("userData",uData);
        }
        break;
    }
    case AddFri:
    {
        if(result == "该好友已下线")
        {
            obj.insert("result",result);
            obj.insert("targetaccount",targetacc);
        }
        else
        {
            msgtype = MsgType;
            if(msgtype == "发送好友申请")
            {
                sendFileName = fileName;

                //将需要的几个用户资料添加到字符串中并用@@隔开
                QString uData = QString::number(acc) + "@@" + m_userDatas.at(ennickname) + "@@" +
                        m_userDatas.at(ensex) + "@@" + m_userDatas.at(enage) + "@@" + m_userDatas.at(enlocation);

                obj.insert("userData",uData);
                obj.insert("msgtype",msgtype);
                obj.insert("yanzheng",Msg);
            }
            else if(msgtype == "成功删除好友")
            {
                obj.insert("friacc",acc);
                obj.insert("msgtype",msgtype);
            }
        }
        break;
    }
    case SendMsg:
    {
        obj.insert("acc",acc);
        obj.insert("msgType",MsgType);
        obj.insert("msg",Msg);
        if(MsgType == "添加好友成功")
        {
            obj.insert("userData",result);
        }
        break;
    }
    }
    QJsonDocument doc(obj);
    QByteArray reply = doc.toJson();
    //将发送数据的大小转换成大端后添加表头
    int len = qToBigEndian(reply.size());
    QByteArray senddata((char*)&len,4);
    senddata.append(reply);

    emit SendMsgToClt(m_tcp->peerPort(),type,acc,targetacc,senddata,sendFileName,msgtype);
}

void WorkThread::recvRegistered(int acc, QString pwd)
{
    QWriteLocker lock(mutex);
    QString iconName = sql.Addaccount(acc,pwd);
    //如果返回的头像名为空则为注册失败
    if(iconName == "")
    {
        qDebug() << "账号重复,注册失败!";
        ReplyToJson(Registration,"","注册失败");
        ThreadbackMsg("注册",acc,"注册失败");
    }
    else
    {
        qDebug() << "注册成功!";
        ReplyToJson(Registration,"","注册成功");
        ThreadbackMsg("注册",acc,"注册成功");
        //如果注册成功则创建以该用户账号为名的文件夹与好友列表文件和头像
        QString path = m_path + "/" + QString::number(acc);
        m_dir.mkdir(path);
        //创建好友列表文件
        QString pathJ = path + "/friends.json";
        QFile file(pathJ);
        file.open(QFile::WriteOnly);

        //创建默认好友分组
        QJsonArray jsarr;
        QJsonObject obj1;
        obj1.insert("name","我的好友");
        QJsonObject obj2;
        obj2.insert("name","朋友");
        QJsonObject obj3;
        obj3.insert("name","家人");
        QJsonObject obj4;
        obj4.insert("name","同学");
        jsarr.append(obj1);
        jsarr.append(obj2);
        jsarr.append(obj3);
        jsarr.append(obj4);

        QJsonDocument doc(jsarr);
        file.write(doc.toJson());

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
    QReadLocker lock(mutex);
    QString rtpwd = sql.FindPwd(acc);
    if(rtpwd == "")
    {
        qDebug() << "无该账户!密码找回失败!";
        ReplyToJson(FindPwd,"");
    }
    else
    {
        qDebug() << "密码找回成功!";
        ReplyToJson(FindPwd,rtpwd);
    }
    ThreadbackMsg("找回密码",acc,rtpwd);
}

void WorkThread::CltLogin(int acc, QString pwd)
{
    QWriteLocker lock(mutex);
    int result = sql.LoginVerification(acc,pwd);
    if(result == -1) //账号或密码错误
    {
        qDebug() << "账号密码错误";
        ReplyToJson(LoginAcc,"","账号密码错误");
        ThreadbackMsg("登录",acc,"账号或密码错误");
    }
    else if(result == 0) //重复登录
    {
        qDebug() << "重复登录";
        ReplyToJson(LoginAcc,"","重复登录");
        ThreadbackMsg("登录",acc,"重复登录");
    }
    else //登录成功
    {
        qDebug() << "登录成功";
        quint16 port = m_tcp->peerPort(); //传递该套接字端口号给主界面
        m_userDatas = sql.UserMessages(acc); //获取用户昵称和个性签名
        emit UserOnLine(acc,port);
        ReplyToJson(LoginAcc,"","登录成功","",acc);
        ThreadbackMsg("登录",acc,"登录成功");
    }
}

void WorkThread::SearchingFri(int acc)
{
    QReadLocker lock(mutex);
    QString res = sql.OnLineSta(acc);
    //如果返回的结果为空或离线则发送查找失败
    if(res == "离线" || res == "")
    {
        qDebug() << "查找好友失败，该账号不存在或未上线";
        ReplyToJson(SearchFri,"","查找失败");
    }
    //否则查找成功,返回用户信息
    else
    {
        m_userDatas = sql.UserMessages(acc);
        ReplyToJson(SearchFri,"","查找成功",m_path + "/" + QString::number(acc) + "/" + QString::number(acc) + ".jpg",acc);
    }
}

void WorkThread::CltAddFri(int acc, int targetacc, QString msgType,QString yanzheng)
{
    QReadLocker lock(mutex);
    QString res = sql.OnLineSta(targetacc);
    if(res == "离线")
    {
        qDebug() << "添加/删除好友失败，该账号已下线";
        ReplyToJson(AddFri,"","该好友已下线","",-1,targetacc);
        return;
    }
    //若为发送好友申请则需发送申请方头像和申请方资料
    QString FileName = "";
    if(msgType == "发送好友申请")
    {
        qDebug() << "正在添加用户头像及资料";
        FileName = m_path + "/" + QString::number(acc) + "/" + QString::number(acc) + ".jpg";
        m_userDatas = sql.UserMessages(acc);
        ReplyToJson(AddFri,"","",FileName,acc,targetacc,msgType,yanzheng);
    }
    else if(msgType == "同意好友申请")
    {
        //在数据库中更新这两个用户的好友列表
        sql.AddFriend(acc,targetacc);
        //向双方发送开始聊天信息及昵称个性签名信息
        m_userDatas = sql.UserMessages(acc);
        QString uD = m_userDatas.at(ennickname) + "@@" + m_userDatas.at(ensignature);
        ReplyToJson(SendMsg,"",uD,"",acc,targetacc,"添加好友成功","我们已经是好友啦，一起来聊天吧！");

        m_userDatas.clear();
        m_userDatas = sql.UserMessages(targetacc);
        uD = m_userDatas.at(ennickname) + "@@" + m_userDatas.at(ensignature);
        ReplyToJson(SendMsg,"",uD,"",targetacc,acc,"添加好友成功","我们已经是好友啦，一起来聊天吧！");
    }
    else if(msgType == "删除好友")
    {
        //在数据库中更新这两个用户的好友列表
        sql.DelFriend(acc,targetacc);

        ReplyToJson(AddFri,"","","",acc,targetacc,"成功删除好友");
        ReplyToJson(AddFri,"","","",targetacc,acc,"成功删除好友");
    }
}

void WorkThread::CltChangeOnlSta(int acc, QString onlsta)
{
    QWriteLocker lock(mutex);
    QString onlineSta = sql.OnLineSta(acc);
    if(onlineSta == "离线")
    {
        qDebug() << "用户掉线重连";
        emit UserOnLine(acc,m_tcp->peerPort());
        ThreadbackMsg("用户掉线重连",acc,"");
    }
    sql.ChangeOnlineSta(acc,onlsta);
}

void WorkThread::ForwardInformation(int acc, int targetacc, QString msgType, QString msg)
{
    ReplyToJson(SendMsg,"","","",acc,targetacc,msgType,msg);
}
