#ifndef USER_H
#define USER_H

#include <QString>


 //Model层：User
 //作用：
 //1.表示系统中的一个用户对象；
 //2.保存从数据库查询出来的用户信息；
 //3.不直接负责数据库操作，也不负责登录逻辑。
 //数据库中username和password都是DES加密后的值。
 //登录成功后，AuthController会把username解密后再保存到currentUser。


class User
{
public:
    User();

    int userId;          //用户唯一编号，对应数据库 users.user_id
    QString username;    //用户名；数据库中是密文，登录成功后对象中可保存明文
    QString password;    //密码；数据库中保存 DES 加密后的密文
    QString role;        //用户角色：admin 或 user
    QString status;      //账号状态：active 或 disabled
};

#endif // USER_H
