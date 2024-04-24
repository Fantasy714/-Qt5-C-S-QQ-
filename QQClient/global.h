#ifndef GLOBAL_H
#define GLOBAL_H

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
    Dnickname = 0,
    Dsignature,
    Dsex,
    Dage,
    Dbirthday,
    Dlocation,
    Dblood_type,
    Dwork,
    Dsch_comp
};

#endif // GLOBAL_H
