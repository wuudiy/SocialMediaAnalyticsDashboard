#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "../models/User.h"
#include <QString>

/*
 * Service / Repository 层：UserRepository
 * --------------------------------------
 * 作用：
 * 1. 专门负责 users 表的数据操作；
 * 2. Controller 不直接写 SQL，而是调用 Repository；
 * 3. 这样可以让登录逻辑和数据库访问逻辑分开。
 */
class UserRepository
{
public:
    UserRepository();

    // 根据明文用户名查找用户，函数内部会先加密用户名再查询数据库
    User findByUsername(const QString& username);

    // 判断用户名是否已存在，注册用户时会用到
    bool usernameExists(const QString& username);

    // 插入新用户，注册用户时使用
    bool insertUser(const QString& username,
                    const QString& encryptedPassword,
                    const QString& role,
                    const QString& status);
};

#endif // USERREPOSITORY_H
