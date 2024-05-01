#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

#define BufferSize 1024
#define HeadSize 10
#define NoHeadBufSize 1014

/*
文件取消传送可添加一个文件取消包头，收到后删除该端口号已接收的文件数据
或者直接发送文件尾，判断如果接收文件大小不等于客户端发送来的文件总大小也可
*/
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
    UpdateHeadShot,
    SendFileToFri
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
