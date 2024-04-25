#include "sendthread.h"
#include <QtEndian>
#include <QFile>
#include <QDataStream>

SendThread::SendThread(QObject *parent) : QObject(parent)
{

}

void SendThread::GetSendMsg(int type, int account, QString fileName)
{
    m_Infotype = (InforType)type;
    m_account = account;
    m_fileName = fileName;
}

void SendThread::SendReply(QTcpSocket * sock,QByteArray jsonData,int fileNum)
{
    //若文件名不为空则为发送文件
    if(m_fileName != "")
    {
        if(fileNum > 1)
        {
            //用?号分割文件名,因为?号不能作为文件名
            QStringList fileNames = m_fileName.split("?");
            for(auto fileName : fileNames)
            {
                SendFile(sock,fileName);
            }
        }
        else
        {
            SendFile(sock,m_fileName);
        }
    }
    if(jsonData.size() != 0)
    {
        SendJson(sock,jsonData);
    }

    qDebug() << sock->peerPort() << ":发送完毕" << jsonData;
    //发送完毕后清空
    m_Infotype = 0;
    m_fileName = "";
    m_account = -1;
}

void SendThread::SendFile(QTcpSocket * sock,QString fileName)
{
    qDebug() << "正在发送文件...";
    QByteArray dataPackage;
    QDataStream out(&dataPackage,QIODevice::WriteOnly);
    QFile file(fileName);
    quint32 fileSize = file.size();

    //获取文件名
    QString fName = fileName.split("/").last();
    //发送的文件头包大小 (QString用QDataStream序列化其大小会*2后+4)
    quint32 size = sizeof(m_Infotype) + sizeof(m_account) + sizeof(fileSize) + fName.size() * 2 + 4;

    //发送文件头数据
    out << quint16(FileInfoHead);
    out << quint32(0);
    out << size;
    out << m_Infotype << m_account << fileSize << fName;

    sock->write(dataPackage);
    sock->waitForBytesWritten();
    dataPackage.clear();

    file.open(QFile::ReadOnly);
    //发送小文件
    if(fileSize < BufferSize)
    {
        qDebug() << "发送小文件";
        //发送文件数据
        QByteArray fileData = file.readAll();
        file.close();

        /*
        将QDataStream对象所关联的QByteArray对象清空后，用QDataStream对象去继续写入数据到QByteArray对象中，
        结果并不如预期那样从位置0开始写入，而是从之前的位置开始写入，前面的数据呈现未定义状态（QBuffer有一样的问题）
        */
        QDataStream fileEndOut(&dataPackage,QIODevice::WriteOnly);

        fileEndOut << quint16(FileEndDataHead);
        fileEndOut << quint32(0);
        fileEndOut << quint32(fileData.size());
        dataPackage.append(fileData);

        sock->write(dataPackage);
        sock->waitForBytesWritten();
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
            QDataStream fileDataOut(&dataPackage,QIODevice::WriteOnly);

            fileDataOut << quint16(FileDataHead);
            fileDataOut << quint32(0);
            fileDataOut << quint32(BufferSize);
            dataPackage.append(fileData);

            sock->write(dataPackage);
            sock->waitForBytesWritten();
            dataPackage.clear();
        }

        QByteArray fileEnd = file.read(lastPackSize);
        file.close();

        QDataStream fileEndOut(&dataPackage,QIODevice::WriteOnly);
        fileEndOut << quint16(FileEndDataHead);
        fileEndOut << quint32(0);
        fileEndOut << quint32(lastPackSize);
        dataPackage.append(fileEnd);

        sock->write(dataPackage);
        sock->waitForBytesWritten();
    }
}

void SendThread::SendJson(QTcpSocket * sock,QByteArray jsonData)
{
    qDebug() << "正在发送Json数据";
    QByteArray dataPackage;
    QDataStream out(&dataPackage,QIODevice::WriteOnly);
    out << quint16(JsonDataHead);
    out << quint32(0);
    out << quint32(jsonData.size());
    dataPackage.append(jsonData);

    sock->write(dataPackage);
    sock->waitForBytesWritten();
}
