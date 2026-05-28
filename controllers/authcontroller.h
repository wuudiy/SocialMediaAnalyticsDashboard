#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include "../models/user.h"
#include "../services/authservice.h"

#include <QString>

/*
 * 登录认证控制器。
 *
 * MVC 重构后，本类只负责 Controller 层：
 * - 给 LoginView 提供 loginUser()；
 * - 保存当前登录用户状态；
 * - 对外返回当前用户和角色。
 *
 * 不再负责：
 * - 登录输入校验；
 * - 密码 SHA-256 校验；
 * - 旧 DES 密码升级；
 * - 用户注册；
 * - 写登录日志；
 * - 直接调用 UserRepository。
 *
 * 用户创建已经统一迁移到：
 * UserController -> UserService
 */
class AuthController
{
public:
    AuthController();

    // 用户登录，message 返回给界面显示。
    bool loginUser(const QString& username,
                   const QString& password,
                   QString& message);

    // 返回当前用户角色。
    QString getCurrentUserRole() const;

    // 返回当前登录用户完整信息。
    User getCurrentUser() const;

private:
    AuthService authService;

    User currentUser;
};

#endif // AUTHCONTROLLER_H
