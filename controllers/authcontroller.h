#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include "../models/user.h"
#include "../repositories/userrepository.h"
#include "../services/logservice.h"

#include <QString>

/*
 * 登录注册业务控制器。
 *
 * 负责：
 * - 登录流程；
 * - 注册流程；
 * - 输入校验；
 * - 当前登录用户状态维护。
 *
 * 不负责：
 * - 直接写 SQL；
 * - 创建界面控件；
 * - 管理数据库连接。
 */
class AuthController
{
public:
    AuthController();

    // 用户登录，message 返回给界面显示。
    bool loginUser(const QString& username,
                   const QString& password,
                   QString& message);

    // 管理员创建新用户，operatorUser 是当前操作人。
    bool registerUser(const User& operatorUser,
                      const QString& username,
                      const QString& password,
                      const QString& role,
                      QString& message);

    // 返回当前登录用户角色。
    QString getCurrentUserRole() const;

    // 返回当前登录用户完整信息。
    User getCurrentUser() const;

private:
    // 登录输入校验。
    bool validateLoginInput(const QString& username,
                            const QString& password,
                            QString& message) const;

    // 注册输入校验。
    bool validateRegisterInput(const QString& username,
                               const QString& password,
                               const QString& role,
                               QString& message) const;

    // 创建用户权限判断。
    bool canCreateUser(const User& operatorUser) const;

    // 校验密码。新密码走 SHA-256，旧 DES 密码登录成功后自动升级。
    bool verifyPasswordAndUpgradeIfNeeded(const User& userFromDatabase,
                                          const QString& password);

private:
    UserRepository userRepository;

    LogService logService;

    User currentUser;
};

#endif // AUTHCONTROLLER_H
