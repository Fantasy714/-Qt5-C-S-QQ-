#include "tcpthread.h"
#include <QHostAddress>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QtEndian>
#include <QImage>
#include <QDir>

TcpThread::TcpThread(QObject* parent) : QObject(parent)
{
    m_InfoType = 0;
}

void TcpThread::StartConnect()
{
    connectToServer();
    //等待套接字连接服务器
    m_tcp->waitForConnected(100);

    //如果未连接成功则启用自动重连
    if(Global::isConnecting == false)
    {
        qDebug() << "正在尝试自动重连";

        if(m_timer == nullptr)
        {
            m_timer = new QTimer;
            connect(m_timer, &QTimer::timeout, this, &TcpThread::AutoConnect);
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

    if(m_file.isOpen())
    {
        m_file.close();
        m_file.remove();
        qDebug() << "已退出并删除未完成接收的文件";
    }

    qDebug() << "套接字线程已退出";
}

void TcpThread::LoginToServer()
{
    MsgToJson(LoginAcc);
}

void TcpThread::SendFile(QString fileName, quint32 type, int Recvaccount)
{
    qDebug() << "正在发送文件...";
    QByteArray dataPackage;
    QDataStream out(&dataPackage, QIODevice::WriteOnly);
    QFile file(fileName);
    qint64 fileSize = file.size();

    //获取文件名
    QString fName = fileName.split("/").last();
    //发送的文件头包大小 (QString用QDataStream序列化其大小会*2后+4)
    quint32 size = sizeof(type) + sizeof(Recvaccount) + sizeof(fileSize) + fName.size() * 2 + 4;

    //发送文件头数据
    out << quint16(FileInfoHead);
    out << quint32(0);
    out << size;
    out << type << Recvaccount << fileSize << fName;
    dataPackage.resize(BufferSize);

    m_tcp->write(dataPackage);
    m_tcp->waitForBytesWritten();
    dataPackage.clear();

    file.open(QFile::ReadOnly);

    //发送小文件
    if(fileSize < NoHeadBufSize)
    {
        qDebug() << "发送小文件";
        //发送文件数据
        QByteArray fileData = file.readAll();
        file.close();

        /*
        将QDataStream对象所关联的QByteArray对象清空后，用QDataStream对象去继续写入数据到QByteArray对象中，
        结果并不如预期那样从位置0开始写入，而是从之前的位置开始写入，前面的数据呈现未定义状态（QBuffer有一样的问题）
        */
        QDataStream fileEndOut(&dataPackage, QIODevice::WriteOnly);

        fileEndOut << quint16(FileEndDataHead);
        fileEndOut << quint32(0);
        fileEndOut << quint32(fileData.size());
        dataPackage.append(fileData);
        dataPackage.resize(BufferSize);

        m_tcp->write(dataPackage);
        m_tcp->waitForBytesWritten();
    }
    else
    {
        qint64 lastPackSize = fileSize % NoHeadBufSize; //包尾大小
        int SendTimes = fileSize / NoHeadBufSize; //总需发送

        if(lastPackSize == 0)
        {
            //如果文件大小刚好为BufferSize的整数倍
            //将普通数据包发送次数减一，最后一次改为发送包尾，将最后一次发送数据的大小设置为BufferSize
            SendTimes -= 1;
            lastPackSize = NoHeadBufSize;
            qDebug() << "文件总大小:" << fileSize << "总共需要发送" << SendTimes << "个数据包,文件尾数据包大小为: " << lastPackSize;
        }
        else
        {
            qDebug() << "文件总大小:" << fileSize << "总共需要发送" << SendTimes + 1 << "个数据包,文件尾数据包大小为: " << lastPackSize;
        }

        //若为发送文件需发送当前发送进度
        if(type == SendFileToFri)
        {
            //进度相关变量
            double lastsize = 0; //上次发送数据量
            double nowsize = 0; //本次发送数据量
            QTime lasttime = QTime::currentTime(); //上次发送时间
            QTime nowtime; //本次发送时间

            //发送文件数据包
            for(int i = 0; i < SendTimes; i++)
            {
                QByteArray fileData = file.read(NoHeadBufSize);
                QDataStream fileDataOut(&dataPackage, QIODevice::WriteOnly);

                fileDataOut << quint16(FileDataHead);
                fileDataOut << quint32(0);
                fileDataOut << quint32(NoHeadBufSize);
                dataPackage.append(fileData);

                m_tcp->write(dataPackage);
                m_tcp->waitForBytesWritten();
                dataPackage.clear();

                nowtime = QTime::currentTime();

                if(lasttime.msecsTo(nowtime) >= 1000) //每秒钟发送一次
                {
                    if(isCloseing) //不加此标志若退出时正在发送大文件程序会崩溃
                    {
                        return;
                    }

                    nowsize = i * NoHeadBufSize; //获取当前已发送大小
                    double speed = nowsize - lastsize; //当前已发送大小减去上次已发送大小即为每秒传输速度
                    double progress = nowsize / fileSize * 100; //当前已发送大小除文件总大小即为已发送百分比
                    double resttime = (fileSize - nowsize) / speed; //文件总大小减去当前已发送大小为剩余未发送大小，除以每秒速度则为剩下需发送的时间秒数
                    QTime Zero(0, 0);
                    QTime restT = Zero.addSecs((int)resttime);
                    emit SendProgressInfo(Recvaccount, fileName, (qint64)speed, (int)progress, restT, true);
                    qDebug() << "当前传输速度: " << speed / 1024 / 1024 << "MB/S, " << "进度: " << (int)progress << " ,剩余传输时间: " << resttime;
                    //将当前时间和大小更新
                    lastsize = nowsize;
                    lasttime = nowtime;
                }
            }
        }
        else
        {
            //发送文件数据包
            for(int i = 0; i < SendTimes; i++)
            {
                QByteArray fileData = file.read(NoHeadBufSize);
                QDataStream fileDataOut(&dataPackage, QIODevice::WriteOnly);

                fileDataOut << quint16(FileDataHead);
                fileDataOut << quint32(0);
                fileDataOut << quint32(NoHeadBufSize);
                dataPackage.append(fileData);

                m_tcp->write(dataPackage);
                m_tcp->waitForBytesWritten();
                dataPackage.clear();
            }
        }

        QByteArray fileEnd = file.readAll();
        file.close();

        QDataStream BfileEndOut(&dataPackage, QIODevice::WriteOnly);

        BfileEndOut << quint16(FileEndDataHead);
        BfileEndOut << quint32(0);
        BfileEndOut << quint32(fileEnd.size());
        dataPackage.append(fileEnd);

        //若数据包大小小于BufferSize,需将数据包填充至BufferSize
        if(dataPackage.size() < BufferSize)
        {
            dataPackage.resize(BufferSize);
        }

        int size = m_tcp->write(dataPackage);
        m_tcp->waitForBytesWritten();
        qDebug() << "包尾大小: " << size << "实际数据大小: " << fileEnd.size();

        //文件发送完成
        if(type == SendFileToFri)
        {
            //传输完成
            emit SendProgressInfo(Recvaccount, fileName, -1, 100, QTime(0, 0), true);
        }
    }
}

void TcpThread::SendJson(QByteArray jsonData)
{
    QByteArray dataPackage;
    QDataStream out(&dataPackage, QIODevice::WriteOnly);
    out << quint16(JsonDataHead);
    out << quint32(0);
    out << quint32(jsonData.size());
    dataPackage.append(jsonData);
    dataPackage.resize(BufferSize);

    m_tcp->write(dataPackage);
    m_tcp->waitForBytesWritten();
}

void TcpThread::connectToServer()
{
    //qDebug() << "套接字线程的线程ID为:" << QThread::currentThreadId();
    if(m_tcp == nullptr)
    {
        m_tcp = new QTcpSocket;
        connect(m_tcp, &QTcpSocket::connected, this, &TcpThread::ConnectSuccess);
        connect(m_tcp, &QTcpSocket::readyRead, this, &TcpThread::ReadMsgFromServer);
        connect(m_tcp, &QTcpSocket::disconnected, this, &TcpThread::DisconnectFromServer);
    }

    m_tcp->connectToHost(QHostAddress(address), port);
}

void TcpThread::recvAccMsg(int type, int acc, QString pwd)
{
    MsgToJson((InforType)type, acc, 0, pwd);
}

void TcpThread::ConnectSuccess()
{
    qDebug() << "连接服务器成功!连接服务器的线程ID为:" << QThread::currentThreadId();
    Global::isConnecting = true;
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
    if(m_tcp->bytesAvailable())
    {
        QByteArray RecvData = m_tcp->readAll();
        int dataSize = RecvData.size();
        //qDebug() << "当前已存储数据大小: " << m_buffer.size() << "此次读取数据大小: " << dataSize;

        //刚好接收到数据包大小数据，且之前无未组成数据包的数据
        if(dataSize == BufferSize && m_buffer.size() == 0)
        {
            SplitDataPackAge(RecvData);
            //qDebug() << "直接收到数据包，处理...";
            return;
        }
        //刚好收到数据包大小数据但已保存有数据则拼接处理后再拼接
        else if(dataSize == BufferSize && m_buffer.size() > 0)
        {
            int MergeSize = BufferSize - m_buffer.size();
            QByteArray MergeData = RecvData.left(MergeSize);
            RecvData.remove(0, MergeSize);

            m_buffer.append(MergeData);

            if(m_buffer.size() == BufferSize)
            {
                SplitDataPackAge(m_buffer);
                m_buffer.clear();
                //qDebug() << "已拼接数据，处理中...";
            }

            m_buffer.append(RecvData);
            //qDebug() << "拼接后余下数据为: " << m_buffer.size();
            return;
        }

        //两次数据拼接后刚好够数据包大小
        if((m_buffer.size() + dataSize) == BufferSize)
        {
            m_buffer.append(RecvData);
            SplitDataPackAge(m_buffer);
            m_buffer.clear();
            //qDebug() << "拼接后刚好够数据包大小";
            return;
        }

        //两次数据相加不够一个数据包大小,直接拼接
        if((m_buffer.size() + dataSize) < BufferSize)
        {
            m_buffer.append(RecvData);
            //qDebug() << "拼接";
            return;
        }

        //若两次数据相加超过单个数据包大小则分包处理
        if((m_buffer.size() + dataSize) > BufferSize)
        {
            //若无已保存数据且本次读取数据大于单个数据包大小则直接分包处理
            if((dataSize > BufferSize) && (m_buffer.size() == 0))
            {
                while(dataSize / BufferSize)
                {
                    QByteArray package = RecvData.left(BufferSize);
                    RecvData.remove(0, BufferSize);

                    SplitDataPackAge(package);
                    dataSize = RecvData.size();

                    if((dataSize / BufferSize == 0) && (dataSize != 0))
                    {
                        m_buffer.append(RecvData);
                    }
                }

                return;
            }

            //若本次读取数据大于单包大小且有已保存数据则先合并之前保存的数据后再分包处理
            if((dataSize > BufferSize) && (m_buffer.size() > 0))
            {
                int MergeSize = BufferSize - m_buffer.size();
                QByteArray MergeData = RecvData.left(MergeSize);
                RecvData.remove(0, MergeSize);

                m_buffer.append(MergeData);

                if(m_buffer.size() == BufferSize)
                {
                    SplitDataPackAge(m_buffer);
                    m_buffer.clear();
                    //qDebug() << "已拼接数据，处理中...";
                }

                dataSize = RecvData.size();

                while(dataSize / BufferSize)
                {
                    QByteArray package = RecvData.left(BufferSize);
                    RecvData.remove(0, BufferSize);

                    SplitDataPackAge(package);
                    dataSize = RecvData.size();

                    if((dataSize / BufferSize == 0) && (dataSize != 0))
                    {
                        m_buffer.append(RecvData);
                    }
                }

                return;
            }
        }
    }
    else
    {
        return;
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
            emit sendResultToAccMsg(type, pwd, "");
            break;
        }

        case Registration:
        {
            qDebug() << "返回注册结果";
            QString result = obj.value("result").toString();
            emit sendResultToAccMsg(type, "", result);

            break;
        }

        case LoginAcc:
        {
            qDebug() << "返回登录结果";
            QString isfirst = obj.value("isfirstlogin").toString();
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
                QFile file(Global::UserLoginFile());

                if(isfirst == "第一次登录") //若为第一次登录
                {
                    qDebug() << "为第一次登录";

                    //接收用户昵称
                    QString mynickname = obj.value("nickname").toString();
                    nickname = mynickname;

                    QString accountS = QString::number(Global::UserAccount());

                    //创建登录界面初始化文件
                    file.open(QFile::WriteOnly);
                    QString msg = mynickname + "##" + accountS + ".jpg##";

                    //若isRemember为真则是记住密码
                    if(Global::isRemember)
                    {
                        msg += "记住##";
                        msg += Global::UserPwd();
                    }
                    else
                    {
                        msg.append("不记住");
                    }

                    file.write(msg.toUtf8());
                    file.close();
                }
                else
                {
                    file.open(QFile::ReadWrite);

                    QString loginD = file.readLine();
                    file.close();
                    QStringList loginDs = loginD.split("##");
                    /*
                     * login.txt文件格式
                     * 用户昵称,头像图片文件名称,是否记住密码,若记住则为密码，否则为空
                     */
                    nickname = loginDs.at(0); //获取用户昵称
                    //若文件中记录为记住则为真，否则为假
                    bool fileRem = loginDs.at(2) == "记住" ? true : false;

                    //若文件中记录与当前选择不相同才更改文件
                    if(fileRem != Global::isRemember)
                    {
                        qDebug() << "正在更改配置文件" << "记住按钮选中状态" << Global::isRemember;
                        QByteArray writeDatas;
                        QString loginData;

                        //昵称和头像名称不改变，只改变后两位是否记住和密码
                        if(Global::isRemember)
                        {
                            loginData = loginDs.at(0) + "##" + loginDs.at(1) + "##" + "记住##" + Global::UserPwd();
                        }
                        else
                        {
                            loginData = loginDs.at(0) + "##" + loginDs.at(1) + "##" + "不记住";
                        }

                        file.open(QFile::WriteOnly | QFile::Truncate);
                        writeDatas = loginData.toUtf8();
                        file.write(writeDatas);
                        file.close();
                    }
                }

                Global::InitUserNameAndSig(nickname, signature);
                emit sendResultToMainInterFace(type, -1);
            }

            break;
        }

        case SearchFri:
        {
            QString res = obj.value("result").toString();

            if(res == "查找失败")
            {
                qDebug() << "好友查找失败";
                emit sendResultToMainInterFace(type, -1, "", "", "查找失败");
            }
            else
            {
                qDebug() << "查找成功";

                //获取用户信息
                QString uD = obj.value("userData").toString();

                emit sendResultToMainInterFace(type, -1, uD, "", "查找成功");
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
                emit sendResultToMainInterFace(type, targetAcc, "", "", result);
            }
            else
            {
                QString msgType = obj.value("msgtype").toString();

                if(msgType == "发送好友申请")
                {
                    //获取用户信息
                    QString uD = obj.value("userData").toString();
                    //获取该用户账号
                    QString friAcc = uD.split("##").at(0);

                    QString yanzheng = obj.value("yanzheng").toString();
                    emit sendResultToMainInterFace(type, -1, uD, yanzheng, msgType);
                }
                else if(msgType == "成功删除好友")
                {
                    int friacc = obj.value("friacc").toInt();
                    qDebug() << "删除好友" << friacc;
                    emit sendResultToMainInterFace(type, friacc, "", "", msgType);
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

                emit sendResultToMainInterFace(type, acc, uD, msg, msgType);
            }
            else if(msgType == "发送图片")
            {
                QString filePath = Global::UserFileRecvPath() + msg;
                qDebug() << "收到图片: " << filePath;
                emit sendResultToMainInterFace(type, acc, "", filePath, msgType);
            }
            else
            {
                qDebug() << acc;
                emit sendResultToMainInterFace(type, acc, "", msg, msgType);
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
                emit sendResultToMainInterFace(type, -1, Datas, "", msgType);
            }
            //否则为请求好友的
            else
            {
                QString result = obj.value("result").toString();
                emit sendResultToMainInterFace(type, acc, Datas, result, msgType);
            }

            break;
        }

        case SendFileToFri:
        {
            int acc = obj.value("friacc").toInt();
            QString msgType = obj.value("msgType").toString();
            QString fileInfo = obj.value("fileInfo").toString();
            qDebug() << "文件接收信息: " << fileInfo;
            emit sendResultToMainInterFace(type, acc, "", fileInfo, msgType);
            break;
        }
    }
}

void TcpThread::DisconnectFromServer()
{
    qDebug() << "服务器断开连接";
    Global::isConnecting = false;
    m_tcp->close();
    emit isConnectingWithServer(false);

    //掉线自动重连
    if(m_timer == nullptr)
    {
        m_timer = new QTimer;
        connect(m_timer, &QTimer::timeout, this, &TcpThread::AutoConnect);
    }

    m_timer->start(3000);
}

void TcpThread::MsgToJson(int type, int acc, int targetacc, QString Msg, QString MsgType)
{
    QString fileName = "";
    int fileNums = -1;
    int Recvaccount = -1;
    QJsonObject obj;
    obj.insert("type", type);

    switch(type)
    {
        case Registration:
        {
            qDebug() << "注册中...";
            obj.insert("account", acc);
            obj.insert("pwd", Msg);
            break;
        }

        case FindPwd:
        {
            qDebug() << "找回密码中...";
            obj.insert("account", acc);
            break;
        }

        case LoginAcc:
        {
            qDebug() << "登录中...";
            obj.insert("isfirstlogin", Global::isFirstLogin);
            obj.insert("account", Global::UserAccount());
            obj.insert("pwd", Global::UserPwd());
            break;
        }

        case SearchFri:
        {
            qDebug() << "查找好友中";
            obj.insert("account", targetacc);
            break;
        }

        case AddFri:
        {
            qDebug() << "发送好友申请信息";
            obj.insert("account", acc);
            obj.insert("targetaccount", targetacc);
            obj.insert("msgtype", MsgType);
            obj.insert("yanzheng", Msg);
            break;
        }

        case ChangeOnlSta:
        {
            qDebug() << "改变在线状态";
            obj.insert("account", acc);
            obj.insert("onlinestatus", Msg);
            break;
        }

        case SendMsg:
        {
            qDebug() << "发送信息";
            obj.insert("account", acc);
            obj.insert("targetacc", targetacc);
            obj.insert("msgtype", MsgType);

            if(MsgType == "发送图片")
            {
                fileName = Msg;
                //取出文件名称
                QString fName = Msg.split("/").last();
                obj.insert("msg", fName);

                fileNums = 1;
                Recvaccount = targetacc;
            }
            else
            {
                obj.insert("msg", Msg);
            }

            break;
        }

        case AskForData:
        {
            obj.insert("account", acc);
            obj.insert("msgtype", MsgType);

            if(MsgType == "请求好友的")
            {
                QFileInfo info(Global::AppAllUserPath() + "/" + QString::number(acc) + ".jpg");
                qDebug() << "好友的头像大小为: " << info.size();
                obj.insert("headSize", info.size());
            }

            break;
        }

        case UserChangeData:
        {
            obj.insert("account", acc);
            obj.insert("userdatas", Msg);
            break;
        }

        case UpdateHeadShot:
        {
            fileName = Global::UserHeadShot();
            QByteArray empty;
            Recvaccount = acc;
            SendToServer(empty, fileName, (InforType)type, 1, Recvaccount);
            return;
        }

        case SendFileToFri:
        {
            qDebug() << "发送信息";
            obj.insert("account", acc);
            obj.insert("targetacc", targetacc);
            obj.insert("msgtype", MsgType);

            if(MsgType == "发送文件") //若为发送文件则添加文件发送信息
            {
                //取出文件名称
                QString fName = Msg.split("/").last();
                obj.insert("fileName", fName);

                fileName = Msg;
                fileNums = 1;
                Recvaccount = targetacc;
            }
            else //否则为接收，直接发送json信息
            {
                obj.insert("fileName", Msg);
            }

            break;
        }
    }

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    SendToServer(data, fileName, (InforType)type, fileNums, Recvaccount);
}

void TcpThread::SendToServer(QByteArray jsondata, QString fileName, InforType type, int fileNums, int RecvAccount)
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
                SendFile(fileName, type, RecvAccount);
            }
        }
        else
        {
            SendFile(fileName, type, RecvAccount);
        }
    }

    if(jsondata.size() != 0)
    {
        SendJson(jsondata);
    }
}

void TcpThread::AutoConnect()
{
    //qDebug() << "断线重连中，请等待...";
    connectToServer();
    m_tcp->waitForConnected(100);

    if(Global::isConnecting == true)
    {
        m_timer->stop();
    }
}

void TcpThread::SplitDataPackAge(QByteArray data)
{
    //取出包头并从数据包中删除包头
    QByteArray head = data.left(10);
    data.remove(0, 10);
    QDataStream headD(&head, QIODevice::ReadOnly);

    //取出包头内容
    quint16 headType;
    quint32 Empty;
    quint32 totalBytes;
    headD >> headType >> Empty >> totalBytes;
    //qDebug() << "接收到数据的包头类型" << headType << "总字节数:" << totalBytes;

    switch(headType)
    {
        case JsonDataHead:
            emit ParseMsg(data.left(totalBytes));
            break;

        case FileInfoHead:
        {
            QByteArray filehead = data.left(totalBytes);
            QDataStream fHead(&filehead, QIODevice::ReadOnly);
            quint32 infotype;
            int RecvAccount;
            QString fName;
            fHead >> infotype >> RecvAccount >> m_fileSize >> fName;
            qDebug() << "接收类型: " << infotype << "接收方账号: " << RecvAccount
                     << "接收文件大小: " << m_fileSize << "接收文件名称: " << fName;
            FilePackAgeCount = 0;

            QString SaveFilePath;

            switch(infotype)
            {
                case LoginAcc: //第一次登录才接收文件
                {
                    qDebug() << "为第一次登录";

                    QString accountS = QString::number(RecvAccount);

                    QDir dir;

                    //若无该账号本地文件夹则创建
                    if(!dir.exists(Global::AppWorkPath() + "/" + accountS))
                    {
                        //创建用户数据文件夹
                        if(!dir.mkdir(Global::AppWorkPath() + "/" + accountS))
                        {
                            qDebug() << "文件夹创建失败";
                            return;
                        }

                        if(!dir.mkdir(Global::AppWorkPath() + "/" + accountS + "/FileRecv"))
                        {
                            qDebug() << "文件接收文件夹创建失败";
                            return;
                        }

                        qDebug() << "文件夹创建成功，正在写入初始文件: " << accountS;
                    }

                    SaveFilePath = Global::AppWorkPath() + "/" + accountS + "/" + fName;
                    qDebug() << "接收文件地址: " << SaveFilePath;
                    m_file.setFileName(SaveFilePath);
                    m_file.open(QFile::WriteOnly);
                }
                break;

                case SearchFri:
                {
                    SaveFilePath = Global::AppAllUserPath() + "/" + fName;
                    m_file.setFileName(SaveFilePath);
                    m_file.open(QFile::WriteOnly | QFile::Truncate);
                }
                break;

                case AddFri:
                {
                    SaveFilePath = Global::AppAllUserPath() + "/" + fName;
                    m_file.setFileName(SaveFilePath);
                    m_file.open(QFile::WriteOnly | QFile::Truncate);
                }
                break;

                case SendMsg:
                {
                    SaveFilePath = Global::UserFileRecvPath() + fName;
                    m_file.setFileName(SaveFilePath);
                    m_file.open(QFile::WriteOnly | QFile::Truncate);
                }
                break;

                case AskForData:
                {
                    SaveFilePath = Global::AppAllUserPath() + "/" + fName;
                    m_file.setFileName(SaveFilePath);
                    m_file.open(QFile::WriteOnly | QFile::Truncate);
                }
                break;

                case SendFileToFri:
                {
                    m_InfoType = SendFileToFri; //将类型设置为发送文件类型
                    m_SendAcc = RecvAccount; //获取发送方账号
                    //初始化进度相关变量
                    m_lastsize = 0;
                    m_nowsize = 0;
                    m_lastT = QTime::currentTime();

                    SaveFilePath = Global::IsFileExist(Global::UserFileRecvPath() + fName);
                    m_file.setFileName(SaveFilePath);
                    m_file.open(QFile::WriteOnly | QFile::Truncate);
                }
                break;

                default:
                    break;
            }
        }
        break;

        case FileDataHead:
        {
            FilePackAgeCount++;
            m_file.write(data);

            if(m_InfoType == SendFileToFri)
            {
                m_nowT = QTime::currentTime();

                if(m_lastT.msecsTo(m_nowT) >= 1000) //每秒发送一次
                {
                    m_nowsize = FilePackAgeCount * NoHeadBufSize; //获取当前已接收大小
                    double speed = m_nowsize - m_lastsize; //当前接收大小减去上次接收大小即为每秒传输速度
                    double progress = m_nowsize / m_fileSize * 100; //当前接收大小除文件总大小即为百分比
                    double restT = (m_fileSize - m_nowsize) / speed; //文件总大小减去当前接收大小，除速度即为剩余下载时间
                    QTime Zero(0, 0);
                    QTime restTime = Zero.addSecs((int)restT);
                    qDebug() << "当前传输速度: " << speed / 1024 / 1024 << "MB/S, " << "进度: " << (int)progress << " ,剩余传输时间: " << restTime;
                    emit SendProgressInfo(m_SendAcc, m_file.fileName(), (int)speed, (int)progress, restTime, false);
                    m_lastsize = m_nowsize;
                    m_lastT = m_nowT;
                }
            }

            /*
            if(FilePackAgeCount % 1000 == 0)
            {
                qDebug() << "已接收的文件数据大小: " << m_file.size()
                         << "文件总大小: " << m_fileSize << "已接收数据包个数: " << FilePackAgeCount;
            }
            */
        }
        break;

        case FileEndDataHead:
        {
            qDebug() << "接收文件包尾数据: " << totalBytes << "已接收文件总大小: " << m_file.size() << "已接收数据包个数: " << FilePackAgeCount + 1;
            m_file.write(data.left(totalBytes));

            if(m_InfoType == SendFileToFri)
            {
                //发送文件接收完成信号并重置文件接收相关变量
                emit SendProgressInfo(m_SendAcc, m_file.fileName(), -1, 100, QTime(0, 0), false);
                m_InfoType = 0;
            }

            m_file.close();
        }
        break;

        default:
            break;
    }
}
