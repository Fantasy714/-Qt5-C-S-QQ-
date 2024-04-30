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

