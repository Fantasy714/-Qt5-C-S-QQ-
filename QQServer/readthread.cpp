#include "readthread.h"
#include <QDataStream>
#include <QThread>

ReadThread::ReadThread(QObject *parent) : QObject(parent)
{

}

void ReadThread::ReadDataFromClt(QTcpSocket * sock)
{
    //读取包头或若仍有未读取完毕的数据则跳过直接读取数据
    if(sock->bytesAvailable())
    {
        quint16 port = sock->peerPort();
        QByteArray RecvData = sock->readAll();
        int dataSize = RecvData.size();
        qDebug() << "当前已存储数据大小: " << m_buffer.size()
                 << "此次读取数据大小: " << dataSize;

        //刚好接收到数据包大小数据，且之前无未组成数据包的数据
        if(dataSize == BufferSize && m_buffer.size() == 0)
        {
            emit RecvFinished(RecvData,port);
            qDebug() << "直接收到数据包，处理...";
            return;
        }
        //刚好收到数据包大小数据但已保存有数据则拼接处理后再拼接
        else if(dataSize == BufferSize && m_buffer.size() > 0)
        {
            int MergeSize = BufferSize - m_buffer.size();
            QByteArray MergeData = RecvData.left(MergeSize);
            RecvData.remove(0,MergeSize);

            m_buffer.append(MergeData);
            if(m_buffer.size() == BufferSize)
            {
                emit RecvFinished(m_buffer,port);
                m_buffer.clear();
                qDebug() << "已拼接数据，处理中...";
            }

            m_buffer.append(RecvData);
            qDebug() << "拼接后余下数据为: " << m_buffer.size();
            return;
        }

        //两次数据拼接后刚好够数据包大小
        if((m_buffer.size() + dataSize) == BufferSize)
        {
            m_buffer.append(RecvData);
            emit RecvFinished(m_buffer,port);
            m_buffer.clear();
            qDebug() << "拼接后刚好够数据包大小";
            return;
        }

        //两次数据相加不够一个数据包大小,直接拼接
        if((m_buffer.size() + dataSize) < BufferSize)
        {
            m_buffer.append(RecvData);
            qDebug() << "拼接";
            return;
        }

        //若两次数据相加超过单个数据包大小则分包处理
        if((m_buffer.size() + dataSize) > BufferSize)
        {
            //若无已保存数据且本次读取数据大于单个数据包大小则直接分包处理
            if((dataSize > BufferSize) && (m_buffer.size() == 0))
            {
                while(dataSize/BufferSize)
                {
                    QByteArray package = RecvData.left(BufferSize);
                    RecvData.remove(0,BufferSize);

                    emit RecvFinished(package,port);
                    dataSize = RecvData.size();

                    if((dataSize/BufferSize == 0) && (dataSize != 0))
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
                RecvData.remove(0,MergeSize);

                m_buffer.append(MergeData);
                if(m_buffer.size() == BufferSize)
                {
                    emit RecvFinished(m_buffer,port);
                    m_buffer.clear();
                    qDebug() << "已拼接数据，处理中...";
                }

                dataSize = RecvData.size();

                while(dataSize/BufferSize)
                {
                    QByteArray package = RecvData.left(BufferSize);
                    RecvData.remove(0,BufferSize);

                    emit RecvFinished(package,port);
                    dataSize = RecvData.size();

                    if((dataSize/BufferSize == 0) && (dataSize != 0))
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
