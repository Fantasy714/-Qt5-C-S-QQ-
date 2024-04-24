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

void ReadThread::GetTcpSocket(QTcpSocket *sock)
{
    m_tcp = sock;
}

void ReadThread::ReadDataFromClt()
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
        qDebug() << "接收到数据的包头类型" << m_type << "总字节数:" << m_totalBytes;
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
            emit RecvFinished(m_byteArray);
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
        ReadDataFromClt();
    }
}

void ReadThread::WriteToFile(QString fileName)
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
