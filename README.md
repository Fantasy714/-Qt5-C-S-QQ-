# 使用Qt5开发的一个C/S架构多线程仿QQ聊天软件

### 开发环境

Windows + Qt 5.14.2 MinGW 64-bit

### 主要内容

使用Qt完成了一个C/S架构的多线程仿QQ聊天软件，客户端使用Json格式向服务器端发送数据包，在每个数据包前添加包头以识别该数据包的操作类型和有效数据大小，服务器端负责读写套接字的线程接收数据后将数据包传给工作线程处理，处理完毕后再将处理结果发送回客户端或转发信息给其他客户端并将用户更改资料和添加好友的操作内容保存在MySql数据库中，客户端之间通过服务器转发来相互聊天或转发图片文件等操作

### 已实现功能:

- 掉线自动重连
- 注册及找回密码功能
- 仿QQ的用户登录界面，保存账号，记住密码
- 更换头像，更改个人昵称，签名和其他个人资料
- 查看好友个人资料，移动好友到其他分组，添加好友，删除好友
- 添加好友分组，更改分组名，删除分组
- 发送消息，发送普通图片和Gif图片，发送大文件
  
### 项目介绍

##### 1.登录界面
![1](https://github.com/Fantasy714/MyQQ/assets/128826119/efb27f78-8990-4dff-998b-882c5fdaa40b)
##### 2.注册账号及找回密码界面
![2](https://github.com/Fantasy714/MyQQ/assets/128826119/5c2c99e2-48ec-4ec5-ae9f-a07fc5e7416b)
![3](https://github.com/Fantasy714/MyQQ/assets/128826119/61001d0f-23d3-4db6-b688-ebf8cc1a9693)
##### 3.掉线，重复登录及密码错误
![5](https://github.com/Fantasy714/MyQQ/assets/128826119/af98c2c7-0942-4fc9-88b3-3496beb41e40)
![4](https://github.com/Fantasy714/MyQQ/assets/128826119/8710cc67-9d17-4737-9cca-a716b360a429)
![6](https://github.com/Fantasy714/MyQQ/assets/128826119/d502401e-5ad2-4cb1-933d-22f63ffd9418)
##### 4.保存账号
![7](https://github.com/Fantasy714/MyQQ/assets/128826119/1b286fdd-56e1-451e-9592-1998d165dd5f)
##### 5.好友列表
![9](https://github.com/Fantasy714/MyQQ/assets/128826119/ddd8683a-520e-46d2-b182-4c481c740e40)
![10](https://github.com/Fantasy714/MyQQ/assets/128826119/18c8d5c7-c36d-4532-85e5-e5eddeab32fd)
![17](https://github.com/Fantasy714/MyQQ/assets/128826119/9f7fde0d-77f7-4ea2-ac6c-ff4817cfe985)
##### 6.个人资料
![11](https://github.com/Fantasy714/MyQQ/assets/128826119/8649b7a4-d958-491c-b13b-7aa513e50e57)
![12](https://github.com/Fantasy714/MyQQ/assets/128826119/50a7f26e-b47f-40fc-80e7-288ef01bea2e)
![13](https://github.com/Fantasy714/MyQQ/assets/128826119/fa990f4f-8b48-4752-ac30-5f072da60381)
##### 7.添加好友
![14](https://github.com/Fantasy714/MyQQ/assets/128826119/671cbe95-bd15-4a03-933a-cb9c1e38f0bc)
![15](https://github.com/Fantasy714/MyQQ/assets/128826119/c9957a4f-3360-419b-be8e-b9b4f0232942)
![16](https://github.com/Fantasy714/MyQQ/assets/128826119/f4c13364-de76-4066-a07c-ce769bbcf9c6)
##### 8.聊天及收发文件
![18](https://github.com/Fantasy714/MyQQ/assets/128826119/1575d7bd-725c-4acf-b91e-0b601075545b)
![19](https://github.com/Fantasy714/MyQQ/assets/128826119/78e8f2dc-58a4-433f-aa4e-dc0d046b3df4)
![20](https://github.com/Fantasy714/MyQQ/assets/128826119/046d9185-5279-4283-b5d6-a16ca42db3cf)
![21](https://github.com/Fantasy714/MyQQ/assets/128826119/f1e941b0-1cb7-4d4d-8333-71540afa371e)
![22](https://github.com/Fantasy714/MyQQ/assets/128826119/000f177f-83c2-405c-81b9-65e5e7d31b8c)
![23](https://github.com/Fantasy714/MyQQ/assets/128826119/abbda8e3-e74b-48b0-853a-c293bdd17a00)
