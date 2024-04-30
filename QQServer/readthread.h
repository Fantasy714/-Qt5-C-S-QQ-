#ifndef READTHREAD_H
#define READTHREAD_H

#include <QObject>
#include <QTcpSocket>
#include <QBuffer>
#include "global.h"
#include <QFile>

class ReadThread : public QObject
{
    Q_OBJECT
public:
    explicit ReadThread(QObject *parent = nullptr);

    void ReadDataFromClt(QTcpSocket*); //读取数据

signals:
    void RecvFinished(QByteArray byteArray,quint16 port); //信息接收完成

private:
    /* 数据操作 */
    QByteArray m_buffer; //保存数据
};

#endif // READTHREAD_H
