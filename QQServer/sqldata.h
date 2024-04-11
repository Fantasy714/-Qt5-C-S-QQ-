#ifndef SQLDATA_H
#define SQLDATA_H

#include <QSqlDatabase>
#include <QObject>

class SqlData
{
    Q_OBJECT
public:
    SqlData(QObject * parent);
    void ConnectSql();
private:
    QSqlDatabase db;
};

#endif // SQLDATA_H
