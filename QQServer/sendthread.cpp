#include "sendthread.h"
#include <QFile>
#include <QtEndian>

SendThread::SendThread(QTcpSocket * sock,QByteArray jsondata,QString fileName,QString msgtype,QObject *parent) : QObject(parent), QRunnable()
{
    m_tcp = sock;
    m_jsonData = jsondata;
    m_fileName = fileName;
    m_MsgType = msgtype;
}

void SendThread::run()
{
    SendReply(m_jsonData,m_fileName);
}

void SendThread::SendReply(QByteArray jsondata, QString fileName)
{
    QByteArray data(jsondata);
    if(fileName != "")
    {
        qDebug() << "要发送文件";
        if(m_MsgType == "第一次登录")
        {
            qDebug() << "开始读取文件";
            //拆分开文件名
            QStringList fileNames = fileName.split("?");

            //获取两个文件的文件名
            QString fileN1 = fileNames.at(0);
            QString fileN2 = fileNames.at(1);

            //读取头像文件
            QFile file(fileN1);
            file.open(QFile::ReadOnly);
            QByteArray fileD1 = file.readAll();
            file.close();

            //读取好友列表文件
            file.setFileName(fileN2);
            file.open(QFile::ReadOnly);
            QByteArray fileD2 = file.readAll();
            file.close();

            //将两个文件添加入待发送数据中
            data.append(fileD1);
            data.append(fileD2);
        }
        else
        {
            //将文件中数据读出
            QFile file(fileName);
            file.open(QFile::ReadOnly);
            QByteArray fileD = file.readAll();
            file.close();

            //将文件加入待发送数据中
            data.append(fileD);
        }
    }
    //给所有数据数据添加表头
    int size = qToBigEndian(data.size());
    QByteArray alldata((char*)&size,4);
    alldata.append(data);

    m_tcp->write(alldata);
    m_tcp->flush(); //将数据立刻发出
    qDebug() << "已发送信息:" << data.size();
}
