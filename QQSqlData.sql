create table qqaccount (
    account int primary key comment '账号',
    pwd varchar(50) not null comment '密码',
    onlinestatus varchar(5) default '离线' comment '在线状态',
    nickname varchar(15) default 'QQ用户' comment '昵称',
    signature varchar(30) default '该用户很懒，暂无个性签名' comment '个性签名',
    sex char(1) default '男' check (sex = '男' or sex = '女') comment '性别',
    age tinyint unsigned default 18 check(age > 0 and age <= 130) comment '年龄',
    birthday date default '2006-11-15' comment '生日',
    location varchar(20) default '中国' comment '地址',
    blood_type varchar(5) default '未知' comment '血型',
    work varchar(10) default '未知' comment '职业',
    sch_comp varchar(20) default '未知' comment '学校/公司'
) comment '账户信息表';

 -- 服务器数据库仅保存好友间关系，不保存用户将好友放在哪个分组下
create table qqfriends (
    account int primary key comment '账号',
    friends varchar(500) comment '好友列表'
) comment 'QQ好友列表';