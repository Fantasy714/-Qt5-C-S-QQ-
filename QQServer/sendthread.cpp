#include "sendthread.h"
#include <QtEndian>
#include <QFile>
#include <QDataStream>

SendThread::SendThread(QObject *parent) : QObject(parent)
{

}

void SendThread::GetSendMsg(QTcpSocket *sock, InforType type, int account,  QByteArray jsonData, QString fileName, int fileNum)
{
    m_tcp = sock;
    m_Infotype = type;
    m_jsonData = jsonData;
    m_fileName = fileName;
    m_fileNum = fileNum;
    m_account = account;
}

void SendThread::SendReply()
{
    //若文件名不为空则为发送文件
    if(m_fileName != "")
    {
        if(m_fileNum > 1)
        {
            //用?号分割文件名,因为?号不能作为文件名
            QStringList fileNames = m_fileName.split("?");
            for(auto fileName : fileNames)
            {
                SendFile(fileName);
            }
        }
        else
        {
            SendFile(m_fileName);
        }
    }
    if(m_jsonData.size() != 0)
    {
        SendJson();
    }

    //发送完毕后清空
    m_Infotype = 0;
    m_jsonData.clear();
    m_fileName = "";
    m_fileNum = 0;
    m_account = 0;
}

void SendThread::SendFile(QString fileName)
{
    QByteArray dataPackage;
    QDataStream out(&dataPackage,QIODevice::WriteOnly);
    QFile file(fileName);
    quint32 fileSize = file.size();

    //获取文件名
    QString fName = fileName.split("/").last();
    //发送的文件头包大小
    quint32 size = sizeof(m_Infotype) + sizeof(m_account) + sizeof(fileSize) + fName.size();

    //发送文件头数据
    out << quint16(FileInfoHead);
    out << quint32(0);
    out << size;
    out << m_Infotype << m_account << fileSize << fName;

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

void SendThread::SendJson()
{
    QByteArray dataPackage;
    QDataStream out(&dataPackage,QIODevice::WriteOnly);
    out << quint16(JsonDataHead);
    out << quint32(0);
    out << quint32(m_jsonData.size());
    dataPackage.append(m_jsonData);

    m_tcp->write(dataPackage);
    m_tcp->waitForBytesWritten();
}
