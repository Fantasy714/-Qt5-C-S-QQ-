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
#include <QImage>

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
    MsgToJson(AddFri,myacc,targetacc,yanzheng,type);
}

void TcpThread::ChangeOnlineSta(int acc, QString onl)
{
    MsgToJson(ChangeOnlSta,acc,-1,onl);
}

void TcpThread::sendSmsToFri(int acc, int targetAcc, QString MsgType, QString Msg)
{
    MsgToJson(SendMsg,acc,targetAcc,Msg,MsgType);
}

void TcpThread::AskForUserData(QString isMe, int acc)
{
    MsgToJson(AskForData,acc,-1,"",isMe);
}

void TcpThread::ChangingUserDatas(int acc, QString datas)
{
    MsgToJson(UserChangeData,acc,-1,datas,"");
}

void TcpThread::ChangingHS(int acc, QString fileN)
{
    CutPhoto(acc,fileN);
    MsgToJson(UpdateHeadShot,acc);
}

void TcpThread::CutPhoto(int acc,QString path)
{
    QImage img;
    if(!img.load(path))
    {
        qDebug() << "图片加载失败: " << path;
        return;
    }

    int pwidth = img.width();
    int phigh = img.height();
    qDebug() << "图片高为:" << pwidth << "宽" << phigh;
    QImage cimg;
    if(pwidth == phigh)
    {
        cimg = img.copy();
        qDebug() << "图片宽高:" << cimg.width() << "," << cimg.height();
    }
    else if(pwidth > phigh)
    {
        qDebug() << "截取横屏图片";
        cimg = img.copy(QRect((pwidth - phigh)/2,0,phigh,phigh));
        qDebug() << "图片宽高:" << cimg.width() << "," << cimg.height();
    }
    else
    {
        qDebug() << "截取竖屏图片";
        cimg = img.copy(QRect(0,(phigh - pwidth)/2,pwidth,pwidth));
        qDebug() << "图片宽高:" << cimg.width() << "," << cimg.height();
    }

    //头像统一设置为350*350
    cimg = cimg.scaled(350,350,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    qDebug() << "图片宽高:" << cimg.width() << "," << cimg.height();

    //更新图片
    QString fileName = m_path + "/" + QString::number(acc) + "/" + QString::number(acc) + ".jpg";

    //删除原来的头像
    bool ok = QFile::remove(fileName);
    if(!ok)
    {
        qDebug() << "删除头像失败";
    }

    cimg.save(fileName);
}

void TcpThread::SendFile(QString fileName,quint32 type)
{
    QByteArray dataPackage;
    QDataStream out(&dataPackage,QIODevice::WriteOnly);
    QFile file(fileName);
    quint32 fileSize = file.size();

    //获取文件名
    QString fName = fileName.split("/").last();
    //发送的文件头包大小
    quint32 size = sizeof(type) + sizeof(m_TargetAcc) + sizeof(fileSize) + fName.size();

    //发送文件头数据
    out << quint16(FileInfoHead);
    out << quint32(0);
    out << size;
    out << type << m_TargetAcc << fileSize << fName;

    m_tcp->write(dataPackage);
    m_tcp->waitForBytesWritten();
    dataPackage.clear();

    //发送小文件
    if(fileSize < BufferSize)
    {
        //发送文件数据
        QByteArray fileData = file.readAll();
        file.close();

        out << quint16(FileEndDataHead);
        out << quint32(0);
        out << quint32(fileData.size());
        dataPackage.append(fileData);

        m_tcp->write(dataPackage);
        m_tcp->waitForBytesWritten();
    }
    else
    {
        quint32 lastPackSize = fileSize % BufferSize;
        int SendTimes = fileSize / BufferSize;
        if(lastPackSize == 0)
        {
            //如果文件大小刚好为BufferSize的整数倍
            //将普通数据包发送次数减一，最后一次改为发送包尾，将最后一次发送数据的大小设置为BufferSize
            SendTimes -= 1;
            lastPackSize = BufferSize;
            qDebug() << "总共需要发送" << SendTimes << "个数据包,文件尾数据包大小为: " << lastPackSize;
        }
        else
        {
            qDebug() << "总共需要发送" << SendTimes + 1 << "个数据包,文件尾数据包大小为: " << lastPackSize;
        }


        for(int i = 0; i < SendTimes; i++)
        {
            QByteArray fileData = file.read(BufferSize);

            out << quint16(FileDataHead);
            out << quint32(0);
            out << quint32(BufferSize);
            dataPackage.append(fileData);

            m_tcp->write(dataPackage);
            m_tcp->waitForBytesWritten();
            dataPackage.clear();
        }

        QByteArray fileEnd = file.read(lastPackSize);
        file.close();

        out << quint16(FileEndDataHead);
        out << quint32(0);
        out << quint32(lastPackSize);
        dataPackage.append(fileEnd);

        m_tcp->write(fileEnd);
        m_tcp->waitForBytesWritten();
    }
}

void TcpThread::SendJson(QByteArray jsonData)
{
    QByteArray dataPackage;
    QDataStream out(&dataPackage,QIODevice::WriteOnly);
    out << quint16(JsonDataHead);
    out << quint32(0);
    out << quint32(jsonData.size());
    dataPackage.append(jsonData);

    m_tcp->write(dataPackage);
    m_tcp->waitForBytesWritten();
}

void TcpThread::WriteToFile(QString fileName)
{
    //将数据存入文件中
    QFile file(fileName);
    file.open(QFile::WriteOnly);
    file.write(m_buffer.readAll());
    file.write(m_byteArray);
    file.close();

    m_buffer.close();
    m_fileSize = 0;
    m_infotype - 0;
    m_fileName = "";
    m_account = -1;
    m_recvFileSize = 0;
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
        qDebug() << "找回密码中...";
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
    if(m_tcp->bytesAvailable() == 0) //判断有没有数据;
    {
        qDebug() << "无数据或数据已读完";
        return;
    }
    //读取包头或若仍有未读取完毕的数据则跳过直接读取数据
    if(m_tcp->bytesAvailable() >= 10 && m_type == 0)
    {
        QByteArray bytearray = m_tcp->read(10);
        QDataStream in(&bytearray,QIODevice::ReadOnly);

        quint32 Empty;
        in >> m_type  >> Empty >> m_totalBytes;
    }
    else if(m_type == 0) //若未开始读取则直接返回
    {
        return;
    }
    //有数据且本次数据未读完
    while(m_tcp->bytesAvailable() && m_recvBytes < m_totalBytes)
    {
        m_byteArray.append(m_tcp->read(m_totalBytes - m_recvBytes));
        m_recvBytes = m_byteArray.size();
    }
    //数据包读取完毕
    if(m_recvBytes == m_totalBytes)
    {
        qDebug() << "数据包读取完毕";
        switch(m_type)
        {
        case JsonDataHead:
            emit ParseMsg(m_byteArray);
            break;
        case FileInfoHead:
            {
                QDataStream in(&m_byteArray,QIODevice::ReadOnly);
                in >> m_infotype >> m_account >> m_fileSize >> m_fileName;
                m_buffer.open(QIODevice::ReadWrite);
            }
            break;
        case FileDataHead:
            {
                m_recvFileSize += m_buffer.write(m_byteArray);
                qDebug() << "已接收的文件数据大小: " << m_recvFileSize;
            }
            break;
        case FileEndDataHead:
            {
                switch(m_infotype)
                {
                case SendMsg:
                    {
                        QString filePath = m_path + "/" + QString::number(m_account) + "/FileRecv/" + m_fileName;
                        WriteToFile(filePath);
                    }
                    break;
                case UpdateHeadShot:
                    {
                        QString filePath = m_path + "/" + QString::number(m_account) + "/" + m_fileName;
                        WriteToFile(filePath);
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        default:
            break;
        }

        m_byteArray.clear();
        m_type = 0;
        m_totalBytes = 0;
        m_recvBytes = 0;
    }
     //若数据未读取完则继续读取
    if(m_tcp->bytesAvailable())
    {
        ReadMsgFromServer();
    }
}

void TcpThread::ParseMsg(QByteArray data)
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
        /*
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
                *
                 * login.txt文件格式
                 * 用户昵称,头像图片文件名称,是否记住密码,若记住则为密码，否则为空
                 *
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

                if(!m_dir.mkdir(m_path + "/" + accountS + "/FileRecv"))
                {
                    qDebug() << "文件接收文件夹创建失败";
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
            else if(msgType == "成功删除好友")
            {
                int friacc = obj.value("friacc").toInt();
                qDebug() << "删除好友" << friacc;
                emit sendResultToMainInterFace(type,friacc,"","","","","",msgType);
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
        else if(msgType == "发送图片")
        {
            QString filePath = m_path + "/" + QString::number(m_account) + "/FileRecv/" + msg;
            qDebug() << "接收图片: " << filePath;
            QFile file(filePath);
            file.open(QFile::WriteOnly);
            file.write(filedata);
            file.close();
            sendResultToMainInterFace(type,acc,"","","","",filePath,msgType);
        }
        else
        {
            qDebug() << acc;
            sendResultToMainInterFace(type,acc,"","","","",msg,msgType);
        }
        break;
    }
    case AskForData:
    {
        int acc = obj.value("account").toInt();
        QString msgType = obj.value("msgtype").toString();
        QString Datas = obj.value("userdatas").toString();
        if(msgType == "请求自己的")
        {
            emit sendResultToMainInterFace(AskForData,-1,"","","",Datas,"",msgType);
        }
        //否则为请求好友的
        else
        {
            QString result = obj.value("result").toString();
            if(result == "需更新头像")
            {
                QFile file(m_alluserspath + "/" + QString::number(acc) + ".jpg");
                file.open(QFile::WriteOnly | QFile::Truncate);
                file.write(filedata);
                file.close();
            }
            emit sendResultToMainInterFace(AskForData,acc,"","",result,Datas,"",msgType);
        }

        break;
    }*/
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

void TcpThread::MsgToJson(InforType type,int acc,int targetacc,QString Msg,QString MsgType)
{
    QString fileName = "";
    int fileNums = -1;
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
        obj.insert("msgtype",MsgType);
        obj.insert("yanzheng",Msg);
        break;
    }
    case ChangeOnlSta:
    {
        qDebug() << "改变在线状态";
        obj.insert("account",acc);
        obj.insert("onlinestatus",Msg);
        break;
    }
    case SendMsg:
    {
        qDebug() << "发送信息";
        obj.insert("account",acc);
        obj.insert("targetacc",targetacc);
        obj.insert("msgtype",MsgType);
        if(MsgType == "发送图片")
        {
            fileName = Msg;
            //取出文件名称
            QString fName = Msg.split("/").last();
            obj.insert("msg",fName);

            fileNums = 1;
            m_Recvaccount = targetacc;
        }
        else
        {
            obj.insert("msg",Msg);
        }
        break;
    }
    case AskForData:
    {
        obj.insert("account",acc);
        obj.insert("msgtype",MsgType);
        if(MsgType == "请求好友的")
        {
            QFileInfo info(m_alluserspath + "/" + QString::number(acc) + ".jpg");
            qDebug() << "好友的头像大小为: " << info.size();
            obj.insert("headSize",info.size());
        }
        break;
    }
    case UserChangeData:
    {
        obj.insert("account",acc);
        obj.insert("userdatas",Msg);

        fileNums = 1;
        m_Recvaccount = acc;
        break;
    }
    case UpdateHeadShot:
    {
        fileName = m_path + "/" + QString::number(acc) + "/" + QString::number(acc) + ".jpg";
        QByteArray empty;
        qDebug() << "更新头像，bytearray大小: " << empty.size();
        SendToServer(empty,fileName,type,1);
        return;
    }
    }
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    SendToServer(data,fileName,type,fileNums);
}

void TcpThread::SendToServer(QByteArray jsondata, QString fileName,InforType type,int fileNums)
{
    //若文件名不为空则为发送文件
    if(fileName != "")
    {
        if(fileNums > 1)
        {
            //用?号分割文件名,因为?号不能作为文件名
            QStringList fileNames = fileName.split("?");
            for(auto fileName : fileNames)
            {
                SendFile(fileName,type);
            }
        }
        else
        {
            SendFile(fileName,type);
        }
    }
    QString data(jsondata);
    qDebug() << data << "发送数据..." << jsondata;
    if(jsondata.size() != 0)
    {
        SendJson(jsondata);
    }

    //发送完毕后清空
    m_TargetAcc = 0;
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
