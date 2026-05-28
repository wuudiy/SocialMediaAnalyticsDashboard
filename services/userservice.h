#ifndef USERSERVICE_H
#define USERSERVICE_H

#include "../models/user.h"
#include "../repositories/userrepository.h"

#include <QList>
#include <QString>

/*
 * 用户操作结果。
 *
 * success：业务是否成功；
 * message：返回给 Controller / View 显示的提示信息。
 */
struct UserOperationResult
{
    bool success = false;
    QString message;
};

/*
 * 用户业务服务。
 *
 * 负责：
 * - 创建用户；
 * - 启用用户；
 * - 禁用用户；
 * - 重置默认密码；
 * - 用户权限判断；
 * - 用户输入校验；
 * - 密码哈希生成。
 *
 * 不负责：
 * - 弹窗；
 * - 表格渲染；
 * - 写操作日志；
 * - 按钮启用禁用。
 */
class UserService
{
public:
    UserService();

    QList<User> findAllUsers();

    bool canManageUsers(const User& operatorUser) const;

    UserOperationResult createUser(const User& operatorUser,
                                   const QString& username,
                                   const QString& password,
                                   const QString& role);

    UserOperationResult enableUser(const User& operatorUser,
                                   const User& targetUser);

    UserOperationResult disableUser(const User& operatorUser,
                                    const User& targetUser);

    UserOperationResult resetPasswordToDefault(const User& operatorUser,
                                               const User& targetUser);

    static QString defaultPassword();

private:
    bool validateCreateUserInput(const QString& username,
                                 const QString& password,
                                 const QString& role,
                                 QString& message) const;

    QString normalizedRole(const QString& role) const;

private:
    UserRepository userRepository;
};

#endif // USERSERVICE_H
