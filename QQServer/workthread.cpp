#include "workthread.h"
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QFileInfo>
#include <QDir>
#include <QThread>

WorkThread::WorkThread(QObject *parent) : QObject(parent)
{

}

void WorkThread::ParseMsg(quint16 port,QByteArray data)
{
    m_port = port;
    QJsonObject obj = QJsonDocument::fromJson(data).object();
    int type = obj.value("type").toInt();
    int account;
    int targetacc;
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
        QString pwd = obj.value("pwd").toString();
        qDebug() << "客户端注册,注册账号:" << account << "密码:" << pwd;
        recvRegistered(account,pwd);
        break;
    }
    case LoginAcc:
    {
        qDebug() << "客户端请求登录";
        bool isFirst = obj.value("isfirstlogin").toBool();
        account = obj.value("account").toInt();
        QString pwd = obj.value("pwd").toString();
        CltLogin(account,pwd,isFirst);
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
        if(msgType == "发送图片")
        {
            QString fileN = obj.value("msg").toString();
            QString filePath = Global::UserFilePath(targetAcc) + fileN;
            qDebug() << "转发图片中...文件路径: " << filePath << "文件名: " << fileN;
            ForwardInformation(acc,targetAcc,msgType,fileN,filePath);
            break;
        }
        QString msg = obj.value("msg").toString();
        ForwardInformation(acc,targetAcc,msgType,msg);
        break;
    }
    case AskForData:
    {
        qDebug() << "请求个人资料";
        int acc = obj.value("account").toInt();
        QString isSelf = obj.value("msgtype").toString();
        if(isSelf == "请求自己的")
        {
            AskForUserData(acc,isSelf);
        }
        //若请求的好友的则需查看该用户本地的好友头像是否和服务器端好友大小相同
        //相同即为好友未更改头像，不相同则需更新好友头像
        else
        {
            int size = obj.value("headSize").toInt();
            AskForUserData(acc,isSelf,size);
        }
        break;
    }
    case UserChangeData:
    {
        qDebug() << "更改用户资料";
        int acc = obj.value("account").toInt();
        QString datas = obj.value("userdatas").toString();
        ChangingUserDatas(acc,datas);
        break;
    }
    }
}

void WorkThread::ReplyToJson(InforType type,QString Msg,QString MsgType, int acc,int targetacc, QString fileName)
{
    //qDebug() << "开始回应客户端";
    QJsonObject obj;
    obj.insert("type",type);
    switch(type)
    {
    case FindPwd:
    {
        obj.insert("pwd",Msg);
        break;
    }
    case Registration:
    {
        obj.insert("result",Msg);
        break;
    }
    case LoginAcc:
    {
        obj.insert("isfirstlogin",MsgType);
        obj.insert("result",Msg);
        if(Msg == "登录成功")
        {
            obj.insert("signature",m_userDatas.at(ensignature));
            if(MsgType == "第一次登录")
            {
                //如果为第一次登录则发送账号昵称，头像和好友信息
                obj.insert("nickname",m_userDatas.at(ennickname));
            }
        }
        break;
    }
    case SearchFri:
    {
        obj.insert("result",Msg);
        //如果查找成功则添加用户信息及头像
        if(Msg == "查找成功")
        {
            //将需要的几个用户资料添加到字符串中并用@@隔开
            QString uData = QString::number(targetacc) + "@@" + m_userDatas.at(ennickname) + "@@" +
                    m_userDatas.at(ensex) + "@@" + m_userDatas.at(enage) + "@@" + m_userDatas.at(enlocation);

            obj.insert("userData",uData);
        }
        break;
    }
    case AddFri:
    {
        if(Msg == "该好友已下线")
        {
            obj.insert("result",Msg);
            obj.insert("targetaccount",targetacc);
        }
        else
        {
            if(MsgType == "发送好友申请")
            {
                //将需要的几个用户资料添加到字符串中并用@@隔开
                QString uData = QString::number(acc) + "@@" + m_userDatas.at(ennickname) + "@@" +
                        m_userDatas.at(ensex) + "@@" + m_userDatas.at(enage) + "@@" + m_userDatas.at(enlocation);

                obj.insert("userData",uData);
                obj.insert("msgtype",MsgType);
                obj.insert("yanzheng",Msg);
            }
            else if(MsgType == "成功删除好友")
            {
                obj.insert("friacc",acc);
                obj.insert("msgtype",MsgType);
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
            qDebug() << acc << "添加" << targetacc << "成功!";
            QStringList userDatas = sql.UserMessages(acc);
            QString uD = userDatas.at(ennickname) + "@@" + userDatas.at(ensignature);
            obj.insert("userData",uD);
        }
        break;
    }
    case AskForData:
    {

        if(MsgType == "请求自己的")
        {
            obj.insert("account",acc);
            obj.insert("msgtype",MsgType);
            obj.insert("userdatas",Msg);
        }
        else
        {
            obj.insert("account",acc);
            obj.insert("userdatas",Msg);

            //将拼接的查找类型和是否更新头像结果拆分
            QStringList res = MsgType.split("@@");
            QString isSelf = res.at(0);
            QString result = res.at(1);

            obj.insert("msgtype",isSelf);
            obj.insert("result",result);

            //将信息类型赋值为,需更新头像/不更新头像
            MsgType = result;
            qDebug() << MsgType;
        }
        break;
    }
    }
    QJsonDocument doc(obj);
    QByteArray reply = doc.toJson();

    int IntType = (int)type;
    emit SendMsgToClt(m_port,IntType,acc,targetacc,reply,MsgType,fileName);
    qDebug() << "发出发送回应信号";
}

void WorkThread::recvRegistered(int acc, QString pwd)
{
    QString iconName = sql.Addaccount(acc,pwd);
    //如果返回的头像名为空则为注册失败
    if(iconName == "")
    {
        qDebug() << "账号重复,注册失败!";
        ReplyToJson(Registration,"注册失败");
        ThreadbackMsg(Registration,acc,"注册失败");
    }
    else
    {
        qDebug() << "注册成功!";
        ReplyToJson(Registration,"注册成功");
        ThreadbackMsg(Registration,acc,"注册成功");
        //如果注册成功则创建以该用户账号为名的文件夹与好友列表文件和头像
        QString path = Global::UserPath(acc);
        QDir dir;
        dir.mkdir(path);
        //创建文件接收文件夹
        dir.mkdir(path + "/FileRecv");
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
        file.setFileName(Global::UserHeadShot(acc));
        file.open(QFile::WriteOnly);
        file.write(pic);
        file.close();
    }
}

void WorkThread::recvFind(int acc)
{
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
    ThreadbackMsg(FindPwd,acc,rtpwd);
}

void WorkThread::CltLogin(int acc, QString pwd,bool isFirst)
{
    int result = sql.LoginVerification(acc,pwd);
    if(result == -1) //账号或密码错误
    {
        qDebug() << "账号密码错误";
        ReplyToJson(LoginAcc,"账号密码错误");
        ThreadbackMsg(LoginAcc,acc,"账号或密码错误");
    }
    else if(result == 0) //重复登录
    {
        qDebug() << "重复登录";
        ReplyToJson(LoginAcc,"重复登录");
        ThreadbackMsg(LoginAcc,acc,"重复登录");
    }
    else //登录成功
    {
        qDebug() << "登录成功";
        m_userDatas = sql.UserMessages(acc); //获取用户昵称和个性签名
        emit UserOnLine(acc,m_port);
        ThreadbackMsg(LoginAcc,acc,"登录成功");
        if(isFirst)
        {
            //发送头像和好友信息
            QString fileN1 = Global::UserHeadShot(acc); //头像
            QString fileN2 = Global::UserPath(acc) + "/" + "friends.json"; //好友列表
            qDebug() << "filename1:" << fileN1 << "filename2:" << fileN2;

            //用?分割文件地址
            QString fPath = fileN1 + "?" + fileN2;

            ReplyToJson(LoginAcc,"登录成功","第一次登录",acc,-1,fPath);
            return;
        }
        ReplyToJson(LoginAcc,"登录成功","",acc);
    }
}

void WorkThread::SearchingFri(int acc)
{
    QString res = sql.OnLineSta(acc);
    //如果返回的结果为空或离线则发送查找失败
    if(res == "离线" || res == "")
    {
        qDebug() << "查找好友失败，该账号不存在或未上线";
        ReplyToJson(SearchFri,"查找失败");
    }
    //否则查找成功,返回用户信息
    else
    {
        m_userDatas = sql.UserMessages(acc);
        ReplyToJson(SearchFri,"查找成功","",-1,acc,Global::UserHeadShot(acc));
    }
}

void WorkThread::CltAddFri(int acc, int targetacc, QString msgType,QString yanzheng)
{
    QString res = sql.OnLineSta(targetacc);
    if(res == "离线")
    {
        qDebug() << "添加/删除好友失败，该账号已下线";
        ReplyToJson(AddFri,"该好友已下线","",-1,targetacc);
        return;
    }
    //若为发送好友申请则需发送申请方头像和申请方资料
    if(msgType == "发送好友申请")
    {
        qDebug() << "正在添加用户头像及资料";
        m_userDatas = sql.UserMessages(acc);
        ReplyToJson(AddFri,yanzheng,msgType,acc,targetacc,Global::UserHeadShot(acc));
    }
    else if(msgType == "同意好友申请")
    {
        //在数据库中更新这两个用户的好友列表
        sql.AddFriend(acc,targetacc);
        ReplyToJson(SendMsg,"我们已经是好友啦，一起来聊天吧！","添加好友成功",acc,targetacc);
        ReplyToJson(SendMsg,"我们已经是好友啦，一起来聊天吧！","添加好友成功",targetacc,acc);
    }
    else if(msgType == "删除好友")
    {
        //在数据库中更新这两个用户的好友列表
        sql.DelFriend(acc,targetacc);

        ReplyToJson(AddFri,"","成功删除好友",acc,targetacc);
        ReplyToJson(AddFri,"","成功删除好友",targetacc,acc);
    }
}

void WorkThread::CltChangeOnlSta(int acc, QString onlsta)
{
    QString onlineSta = sql.OnLineSta(acc);
    if(onlineSta == "离线")
    {
        qDebug() << "用户掉线重连";
        emit UserOnLine(acc,m_port);
        //ThreadbackMsg("用户掉线重连",acc,"");
    }
    sql.ChangeOnlineSta(acc,onlsta);
}

void WorkThread::ForwardInformation(int acc, int targetacc, QString msgType, QString msg,QString filePath)
{
    ReplyToJson(SendMsg,msg,msgType,acc,targetacc,filePath);
}

void WorkThread::AskForUserData(int acc, QString isSelf, int HeadShotSize)
{
    m_userDatas = sql.UserMessages(acc);
    QString datas;
    for(auto uD : m_userDatas)
    {
        datas.append(uD);
        datas.append("@@");
    }
    qDebug() << "返回个人资料: " << datas;

    if(isSelf == "请求自己的")
    {
        ReplyToJson(AskForData,datas,isSelf,acc,-1);
    }
    else
    {
        QString hsPath = Global::UserHeadShot(acc);
        QFileInfo info(hsPath);
        int fHs = info.size();
        qDebug() << "该用户本地的好友头像大小: " << HeadShotSize << "服务器保存的好友头像大小: " << fHs;
        if(fHs != HeadShotSize) //头像大小不同即为好友已更改头像
        {
            qDebug() << "需更新好友头像";
            ReplyToJson(AskForData,datas,isSelf + "@@" + "需更新头像",acc,-1,hsPath);
            return;
        }
        ReplyToJson(AskForData,datas,isSelf + "@@" + "不更新头像",acc,-1);
    }
}

void WorkThread::ChangingUserDatas(int acc, QString datas)
{
    QStringList UserDatas = datas.split("@@");

    bool changed = sql.ChangeUserMessages(acc,UserDatas);
    if(!changed)
    {
        qDebug() << "用户资料更改失败!";
    }
}
