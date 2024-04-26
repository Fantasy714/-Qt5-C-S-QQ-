#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

#define BufferSize 40960

//包头类型
enum RecvType {
    JsonDataHead = 50,
    FileInfoHead,
    FileDataHead,
    FileEndDataHead
};

//Json数据中信息类型
enum InforType {
    Registration = 1125,
    FindPwd,
    LoginAcc,
    SearchFri,
    AddFri,
    ChangeOnlSta,
    SendMsg,
    AskForData,
    UserChangeData,
    UpdateHeadShot
};

//保存用户资料编号
enum UserMsg {
    ennickname = 0,
    ensignature,
    ensex,
    enage,
    enbirthday,
    enlocation,
    enblood_type,
    enwork,ensch_comp
};

class Global
{
public:
    static void GetWorkPath(QString path); //获取工作目录用户数据文件夹
    static bool CreateWorkPath(); //创建工作目录用户数据文件夹
    static QString UserPath(int account); //返回该用户数据文件夹位置
    static QString UserHeadShot(int account); //返回该用户头像位置
    static QString UserFilePath(int account); //返回该用户文件接收文件夹位置
private:
    static QString WorkPath; //工作目录用户数据文件夹
};

#endif // GLOBAL_H
