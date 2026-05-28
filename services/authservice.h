#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include "../models/user.h"
#include "../repositories/userrepository.h"
#include "logservice.h"

#include <QString>

/*
 * 登录结果。
 *
 * success：登录是否成功；
 * message：返回给 LoginView 显示的提示信息；
 * user：登录成功后的当前用户。
 */
struct AuthLoginResult
{
    bool success = false;
    QString message;
    User user;
};

/*
 * 认证业务服务。
 *
 * 本批重构后，AuthService 只负责“认证相关业务”：
 * - 登录输入校验；
 * - 根据用户名查询用户；
 * - 检查账号状态；
 * - 校验 SHA-256 密码；
 * - 兼容旧 DES 密码，并在登录成功后自动升级为 SHA-256；
 * - 写登录日志。
 *
 * 不再负责：
 * - 创建用户；
 * - 用户启用 / 禁用；
 * - 重置密码；
 * - 用户管理日志。
 *
 * 用户管理统一交给：
 * UserController -> UserService -> UserRepository
 */
class AuthService
{
public:
    AuthService();

    AuthLoginResult loginUser(const QString& username,
                              const QString& password);

private:
    bool validateLoginInput(const QString& username,
                            const QString& password,
                            QString& message) const;

    bool verifyPasswordAndUpgradeIfNeeded(const User& userFromDatabase,
                                          const QString& password);

    User buildLoginUser(const User& userFromDatabase,
                        const QString& plainPassword) const;

private:
    UserRepository userRepository;
    LogService logService;
};

#endif // AUTHSERVICE_H
