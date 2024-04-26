#ifndef READTHREAD_H
#define READTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QBuffer>
#include "global.h"

class ReadThread : public QObject
{
    Q_OBJECT
public:
    explicit ReadThread(QObject *parent = nullptr);

    void ReadDataFromClt(QTcpSocket*); //读取数据
    void WriteToFile(QString fileName); //写入文件

signals:
    void RecvFinished(quint16 port,QByteArray byteArray);

private:
    /* 数据操作 */
    QByteArray m_byteArray; //保存数据
    quint16 m_type; //包头类型
    quint32 m_totalBytes; //数据包实际大小
    quint32 m_recvBytes; //已接收数据的大小

    /* 文件操作 */
    QBuffer m_buffer; //数据缓存
    quint32 m_infotype; //具体数据类型
    quint32 m_fileSize; //文件大小
    int m_recvFileSize; //已接收的文件数据大小
    QString m_fileName; //文件名称
    int m_account; //存放文件方账号
};

#endif // READTHREAD_H
