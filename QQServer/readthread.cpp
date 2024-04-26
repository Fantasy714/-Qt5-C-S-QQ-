#include "readthread.h"
#include <QDataStream>
#include <QFile>

ReadThread::ReadThread(QObject *parent) : QObject(parent)
{
    m_type = 0;
    m_recvBytes = 0;
    m_recvFileSize = 0;
    m_account = -1;
}

void ReadThread::ReadDataFromClt(QTcpSocket * sock)
{
    if(sock->bytesAvailable() == 0) //判断有没有数据;
    {
        qDebug() << "无数据或数据已读完";
        return;
    }
    //读取包头或若仍有未读取完毕的数据则跳过直接读取数据
    if(sock->bytesAvailable() >= 10 && m_type == 0)
    {
        QByteArray bytearray = sock->read(10);
        QDataStream in(&bytearray,QIODevice::ReadOnly);

        quint32 Empty;
        in >> m_type  >> Empty >> m_totalBytes;
        qDebug() << "接收到数据的包头类型" << m_type << "总字节数:" << m_totalBytes;
    }
    else if(m_type == 0) //若未开始读取则直接返回
    {
        return;
    }
    //有数据且本次数据未读完
    while(sock->bytesAvailable() && m_recvBytes < m_totalBytes)
    {
        m_byteArray.append(sock->read(m_totalBytes - m_recvBytes));
        m_recvBytes = m_byteArray.size();
    }
    //数据包读取完毕
    if(m_recvBytes == m_totalBytes)
    {
        qDebug() << "数据包读取完毕";
        switch(m_type)
        {
        case JsonDataHead:
            emit RecvFinished(sock->peerPort(),m_byteArray);
            break;
        case FileInfoHead:
            {
                QDataStream in(&m_byteArray,QIODevice::ReadOnly);
                in >> m_infotype >> m_account >> m_fileSize >> m_fileName;
                qDebug() << "接收类型: " << m_infotype << "接收方账号: " << m_account
                         << "接收文件大小: " << m_fileSize << "接收文件名称: " << m_fileName;
                m_recvFileSize = 0;
                m_buffer.open(QIODevice::ReadWrite | QIODevice::Truncate);
            }
            break;
        case FileDataHead:
            {
                m_recvFileSize += m_buffer.write(m_byteArray);
                qDebug() << "已接收的文件数据大小: " << m_recvFileSize
                         << "缓冲区大小: " << m_buffer.size();
            }
            break;
        case FileEndDataHead:
            {
                switch(m_infotype)
                {
                case SendMsg:
                    {
                        QString filePath = Global::UserFilePath(m_account) + m_fileName;
                        WriteToFile(filePath);
                        m_buffer.close();
                    }
                    break;
                case UpdateHeadShot:
                    {
                        QString filePath = Global::UserHeadShot(m_account);
                        WriteToFile(filePath);
                        m_buffer.close();
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
    if(sock->bytesAvailable())
    {
        ReadDataFromClt(sock);
    }
}

void ReadThread::WriteToFile(QString fileName)
{
    m_buffer.open(QIODevice::ReadOnly);

    //将数据存入文件中
    QFile file(fileName);
    qDebug() << "写入文件: " << fileName << "中...";
    file.open(QFile::WriteOnly);
    if(m_buffer.size() > 0)
    {
        int size1 = file.write(m_buffer.readAll());
        qDebug() << "m_buffer 写入" << size1;
    }
    int size2 = file.write(m_byteArray);
    qDebug() << "m_byteArray 写入" << size2;
    file.close();

    m_buffer.close();
}
