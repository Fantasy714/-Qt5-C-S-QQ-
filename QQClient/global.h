#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QPixmap>

#define BufferSize 1024
#define HeadSize 10 //包头大小
#define NoHeadBufSize 1014 //去掉包头数据包剩余大小

//包头类型
enum RecvType
{
    JsonDataHead = 50,
    FileInfoHead,
    FileDataHead,
    FileEndDataHead
};

//Json数据中信息类型
enum InforType
{
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
enum UserMsg
{
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

class Global
{
public:
    /* 用户数据文件夹目录 */
    static void GetWorkPath(QString path); //获取工作目录用户数据文件夹
    static QString AppWorkPath(); //返回工作目录
    static QString AppAllUserPath(); //返回好友头像目录
    static bool CreateWorkPath(); //创建工作目录用户数据文件夹

    //设置用户信息
    static void InitLoginUserInfo(int account, QString pwd); //设置用户数据
    static void InitUserNameAndSig(QString name, QString sig); //设置用户名和个签

    /* 登录用户数据文件夹 */
    static QString UserPath(); //返回登录用户文件夹位置
    static QString UserLoginFile(); //返回用户登录初始化文件
    static QString UserFileRecvPath(); //返回登录用户文件接收文件夹位置
    static QString UserHeadShot(); //登录用户头像位置

    /* 登录用户信息 */
    static int UserAccount(); //返回登录用户账号
    static QString UserPwd(); //返回登录用户密码
    static QString UserNickName(); //返回用户昵称
    static QString UserSignature(); //返回用户个签

    static QPixmap CreateHeadShot(QString picPath); //返回无锯齿圆形头像
    static QString IsFileExist(QString filepath); //查看文件是否存在，不存在直接返回，存在则添加后缀

    static QString scrollbarStyle;

    static bool isFirstLogin; //是否是第一次登录
    static bool isRemember; //是否记住密码
    static bool isConnecting; //是否已连接到服务器
private:
    static QString WorkPath; //工作目录下用户数据文件夹
    static QString AllUserPath; //好友头像文件夹

    static QString LoginUserPath; //登录账号用户文件夹
    static QString LoginUserFileRecvPath; //登录账号用户文件存放文件夹

    static int m_UserAccount; //登录账号
    static QString m_UserPwd; //登录密码
    static QString m_UserHeadShot; //登录账号头像
    static QString m_nickName; //昵称
    static QString m_signature; //个性签名
};

#endif // GLOBAL_H
