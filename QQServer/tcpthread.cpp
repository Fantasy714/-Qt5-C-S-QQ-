#include "tcpthread.h"
#include <QThread>
#include <QDataStream>
#include <QFile>

TcpThread::TcpThread(QObject* parent) : QObject(parent)
{

}

TcpThread::~TcpThread()
{
    for(auto onl : m_onlines)
    {
        int acc = m_onlines.key(onl);
        m_sql.ChangeOnlineSta(acc, "离线");
    }

    m_onlines.clear();

    for(auto sock : m_sockets)
    {
        sock->close();
        sock->deleteLater();
    }

    m_sockets.clear();

    m_serv->close();
    m_serv->deleteLater();
    qDebug() << "套接字线程退出";
}

void TcpThread::StartListen()
{
    //服务器开始监听
    m_serv = new QTcpServer;
    m_serv->listen(QHostAddress::Any, m_port);
    connect(m_serv, &QTcpServer::newConnection, this, &TcpThread::ServerhasNewConnection);
}

void TcpThread::ServerhasNewConnection()
{
    //获取套接字后加入以该套接字端口号为key的哈希表中
    QTcpSocket* sock = m_serv->nextPendingConnection();
    quint16 port = sock->peerPort();
    //qDebug() << "服务器分配的端口号为:" << port;
    emit NewCltConnected(port);
    connect(sock, &QTcpSocket::readyRead, this, &TcpThread::ReadDataFromClt);
    connect(sock, &QTcpSocket::disconnected, this, &TcpThread::CltDisconnected);
    m_sockets.insert(port, sock);
}

void TcpThread::CltDisconnected()
{
    QTcpSocket* sock = (QTcpSocket*)sender();
    int acc = m_onlines.key(sock);
    qDebug() << acc << "用户退出";

    //从在线用户哈希表中删除
    if(acc != 0)
    {
        m_onlines.remove(acc);
    }

    emit CltDisConnected(sock->peerPort(), acc);
    m_sockets.remove(sock->peerPort());
    sock->close();
    sock->deleteLater();
}

void TcpThread::UserOnLine(int acc, quint16 sockport)
{
    QTcpSocket* thissock = m_sockets[sockport];
    m_onlines.insert(acc, thissock);
    qDebug() << "账号" << acc << "成功加入在线用户哈希表";
}

void TcpThread::ReadDataFromClt()
{
    QTcpSocket* sock = (QTcpSocket*)sender();

    //读取包头或若仍有未读取完毕的数据则跳过直接读取数据
    if(sock->bytesAvailable())
    {
        quint16 port = sock->peerPort();
        QByteArray RecvData = sock->readAll();
        int dataSize = RecvData.size();
        //qDebug() << "当前已存储数据大小: " << m_buffer.size() << "此次读取数据大小: " << dataSize;

        //刚好接收到数据包大小数据，且之前无未组成数据包的数据
        if(dataSize == BufferSize && m_buffer.size() == 0)
        {
            emit RecvFinished(RecvData, port);
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
                emit RecvFinished(m_buffer, port);
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
            emit RecvFinished(m_buffer, port);
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

                    emit RecvFinished(package, port);
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
                    emit RecvFinished(m_buffer, port);
                    m_buffer.clear();
                    //qDebug() << "已拼接数据，处理中...";
                }

                dataSize = RecvData.size();

                while(dataSize / BufferSize)
                {
                    QByteArray package = RecvData.left(BufferSize);
                    RecvData.remove(0, BufferSize);

                    emit RecvFinished(package, port);
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

void TcpThread::SendReply(quint16 port, int type, int acc, int targetacc, QByteArray jsonData, QString msgtype, QString fileName)
{
    //发送文件数量
    int fileNum = 0;
    //发送的账号,接收端依据账号和信息类型判断文件存放的文件夹
    int sendAcc = -1;
    QTcpSocket* sock;

    //若需要转发则要获取目标客户端套接字
    if(msgtype == "发送好友申请" || msgtype == "成功删除好友" || type == SendMsg || msgtype == "发送文件")
    {
        sock = m_onlines[targetacc];
    }
    else
    {
        sock = m_sockets[port];

        if(msgtype == "第一次登录") //第一次登录需发送头像和好友json文件
        {
            sendAcc = acc;
            fileNum = 2;
        }

        if(msgtype == "接收文件")
        {
            //此处将账号设置为好友账号，用于接收文件时发送进度信号给对应的好友窗口
            sendAcc = targetacc;
        }
    }

    if(sock == nullptr)
    {
        qDebug() << "好友未上线";
        return;
    }

    if(msgtype == "需更新头像")
    {
        sendAcc = acc;
    }

    //若文件名不为空则为发送文件
    if(fileName != "")
    {
        if(fileNum > 1)
        {
            //用?号分割文件名,因为?号不能作为文件名
            QStringList fileNames = fileName.split("?");

            for(auto fileName : fileNames)
            {
                SendFile(sock, fileName, type, sendAcc);
            }
        }
        else
        {
            SendFile(sock, fileName, type, sendAcc);
        }
    }

    if(jsonData.size() != 0)
    {
        SendJson(sock, jsonData);
    }
}

void TcpThread::SendFile(QTcpSocket* sock, QString fileName, int infotype, int RecvAccount)
{
    QByteArray dataPackage;
    QDataStream out(&dataPackage, QIODevice::WriteOnly);
    //获取文件总大小
    QFile file(fileName);
    qint64 fileSize = file.size();

    //获取文件名
    QString fName = fileName.split("/").last();
    qDebug() << "正在发送文件, 文件名: " << fName << ", 接收方账号: " << RecvAccount;
    //发送的文件头包大小 (QString用QDataStream序列化其大小会*2后+4)
    quint32 size = sizeof(infotype) + sizeof(RecvAccount) + sizeof(fileSize) + fName.size() * 2 + 4;

    //发送文件头数据
    out << quint16(FileInfoHead);
    out << quint32(0);
    out << size;
    out << infotype << RecvAccount << fileSize << fName;
    dataPackage.resize(BufferSize);

    sock->write(dataPackage);
    sock->waitForBytesWritten();
    dataPackage.clear();

    file.open(QFile::ReadOnly);

    //发送小文件
    if(fileSize < NoHeadBufSize)
    {
        qDebug() << "发送小文件";
        //读取文件数据
        QByteArray fileData = file.readAll();
        file.close();

        /*
        将QDataStream对象所关联的QByteArray对象清空后，用QDataStream对象去继续写入数据到QByteArray对象中，
        结果并不如预期那样从位置0开始写入，而是从之前的位置开始写入，前面的数据呈现未定义状态
        */
        QDataStream fileEndOut(&dataPackage, QIODevice::WriteOnly);

        fileEndOut << quint16(FileEndDataHead);
        fileEndOut << quint32(0);
        fileEndOut << quint32(fileData.size());
        dataPackage.append(fileData);
        dataPackage.resize(BufferSize);

        sock->write(dataPackage);
        sock->waitForBytesWritten();
    }
    else
    {
        qint64 lastPackSize = fileSize % NoHeadBufSize;
        int SendTimes = fileSize / NoHeadBufSize;

        if(lastPackSize == 0)
        {
            //如果文件大小刚好为BufferSize的整数倍
            //将普通文件数据包发送次数减一，最后一次改为发送文件包尾，将最后一次发送数据的大小设置为数据包的包大小
            SendTimes -= 1;
            lastPackSize = NoHeadBufSize;
            qDebug() << "文件总大小: " << fileSize << "总共需要发送" << SendTimes << "个数据包,文件尾数据包大小为: " << lastPackSize;
        }
        else
        {
            qDebug() << "文件总大小: " << fileSize << "总共需要发送" << SendTimes + 1 << "个数据包,文件尾数据包大小为: " << lastPackSize;
        }

        for(int i = 0; i < SendTimes; i++)
        {
            QByteArray fileData = file.read(NoHeadBufSize);
            QDataStream fileDataOut(&dataPackage, QIODevice::WriteOnly);

            fileDataOut << quint16(FileDataHead);
            fileDataOut << quint32(0);
            fileDataOut << quint32(NoHeadBufSize);
            dataPackage.append(fileData);

            sock->write(dataPackage);
            sock->waitForBytesWritten();
            dataPackage.clear();
        }

        QByteArray fileEnd = file.read(lastPackSize);
        file.close();

        QDataStream fileEndOut(&dataPackage, QIODevice::WriteOnly);
        fileEndOut << quint16(FileEndDataHead);
        fileEndOut << quint32(0);
        fileEndOut << quint32(fileEnd.size());
        dataPackage.append(fileEnd);

        if(dataPackage.size() < BufferSize)
        {
            dataPackage.resize(BufferSize);
        }

        sock->write(dataPackage);
        sock->waitForBytesWritten();
    }
}

void TcpThread::SendJson(QTcpSocket* sock, QByteArray jsonData)
{
    qDebug() << "正在发送Json数据";
    QByteArray dataPackage;
    QDataStream out(&dataPackage, QIODevice::WriteOnly);
    out << quint16(JsonDataHead);
    out << quint32(0);
    out << quint32(jsonData.size());
    dataPackage.append(jsonData);
    dataPackage.resize(BufferSize);

    sock->write(dataPackage);
    sock->waitForBytesWritten();
}
