#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include "../models/User.h"
#include "../services/UserRepository.h"

#include <QString>

/*
 * Controller 层：AuthController
 * -----------------------------
 * 作用：
 * 1. 处理登录请求；
 * 2. 检查用户输入；
 * 3. 调用 UserRepository 查询用户；
 * 4. 调用 DESUtil 解密密码；
 * 5. 判断登录是否成功；
 * 6. 保存当前登录用户。
 *
 * 注意：
 * View 不直接访问数据库；
 * View 只调用 AuthController。
 */
class AuthController
{
public:
    AuthController();

    // 登录验证，message 用于返回错误信息或成功信息
    bool loginUser(const QString& username,
                   const QString& password,
                   QString& message);

    // 获取当前用户角色
    QString getCurrentUserRole() const;

    // 获取当前登录用户
    User getCurrentUser() const;

private:
    UserRepository userRepository;  // 用于访问 users 表
    User currentUser;               // 保存当前登录成功的用户
};

#endif // AUTHCONTROLLER_H
