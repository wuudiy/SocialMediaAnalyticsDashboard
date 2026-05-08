#ifndef USER_H
#define USER_H

#include <QString>

/*
 * 用户数据模型。
 *
 * 这个类只保存用户信息和一些基础判断，
 * 不负责登录、注册、数据库查询、密码加密等业务。
 */
class User
{
public:
    User();

    // 判断是否是有效用户。userId 为 -1 时表示未查询到用户。
    bool isValid() const;

    // 判断是否是管理员角色。
    bool isAdmin() const;

    // 判断账号是否处于启用状态。
    bool isActive() const;

public:
    int userId;          // 用户 ID，对应 users.user_id
    QString username;    // 用户名，数据库中保存密文
    QString password;    // 密码，数据库中保存密文
    QString role;        // 用户角色：admin 或 user
    QString status;      // 账号状态：active 或 disabled
};

#endif // USER_H
