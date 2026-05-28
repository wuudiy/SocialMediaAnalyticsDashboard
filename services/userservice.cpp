#include "userservice.h"

#include "../utils/sha256util.h"

UserService::UserService()
{
}

QList<User> UserService::findAllUsers()
{
    return userRepository.findAllUsers();
}

/*
 * 当前项目规则：
 * 只有 active admin 才能管理用户。
 */
bool UserService::canManageUsers(const User& operatorUser) const
{
    return operatorUser.isValid()
    && operatorUser.isActive()
        && operatorUser.isAdmin();
}

UserOperationResult UserService::createUser(const User& operatorUser,
                                            const QString& username,
                                            const QString& password,
                                            const QString& role)
{
    const QString trimmedUsername = username.trimmed();
    const QString trimmedPassword = password.trimmed();
    const QString cleanRole = normalizedRole(role);

    if (!canManageUsers(operatorUser)) {
        return {
            false,
            QStringLiteral("Only admin users can create users.")
        };
    }

    QString message;

    if (!validateCreateUserInput(trimmedUsername,
                                 trimmedPassword,
                                 cleanRole,
                                 message)) {
        return { false, message };
    }

    if (userRepository.usernameExists(trimmedUsername)) {
        return {
            false,
            QStringLiteral("The username already exists.")
        };
    }

    /*
     * 这里从 View 中迁移过来：
     * 新用户密码统一保存 SHA-256 哈希，不保存明文。
     */
    const QString passwordHash = SHA256Util::hashPassword(trimmedPassword);

    const bool inserted = userRepository.insertUser(
        trimmedUsername,
        passwordHash,
        cleanRole,
        QStringLiteral("active")
        );

    if (!inserted) {
        return {
            false,
            QStringLiteral("Failed to create user. Please check the database.")
        };
    }

    return {
        true,
        QStringLiteral("User registered successfully.")
    };
}

UserOperationResult UserService::enableUser(const User& operatorUser,
                                            const User& targetUser)
{
    if (!canManageUsers(operatorUser)) {
        return {
            false,
            QStringLiteral("You do not have permission to enable users.")
        };
    }

    if (!targetUser.isValid()) {
        return {
            false,
            QStringLiteral("Please select a user first.")
        };
    }

    if (targetUser.isActive()) {
        return {
            false,
            QStringLiteral("The selected user is already active.")
        };
    }

    const bool updated = userRepository.updateStatusByUserId(
        targetUser.userId,
        QStringLiteral("active")
        );

    if (!updated) {
        return {
            false,
            QStringLiteral("Failed to enable user. Please check the database.")
        };
    }

    return {
        true,
        QStringLiteral("User enabled successfully.")
    };
}

UserOperationResult UserService::disableUser(const User& operatorUser,
                                             const User& targetUser)
{
    if (!canManageUsers(operatorUser)) {
        return {
            false,
            QStringLiteral("You do not have permission to disable users.")
        };
    }

    if (!targetUser.isValid()) {
        return {
            false,
            QStringLiteral("Please select a user first.")
        };
    }

    if (!targetUser.isActive()) {
        return {
            false,
            QStringLiteral("The selected user is already disabled.")
        };
    }

    /*
     * 禁止禁用当前登录用户。
     *
     * 这条业务规则原来写在 UserManagementPage 里，
     * 现在迁移到 Service，避免 View 参与业务判断。
     */
    if (targetUser.userId == operatorUser.userId) {
        return {
            false,
            QStringLiteral("You cannot disable the currently logged-in user.")
        };
    }

    /*
     * 系统至少保留一个 active admin。
     *
     * 如果只剩一个可用管理员，不能把它禁用。
     */
    if (targetUser.isAdmin() && userRepository.countActiveAdmins() <= 1) {
        return {
            false,
            QStringLiteral("You cannot disable the last active admin.")
        };
    }

    const bool updated = userRepository.updateStatusByUserId(
        targetUser.userId,
        QStringLiteral("disabled")
        );

    if (!updated) {
        return {
            false,
            QStringLiteral("Failed to disable user. Please check the database.")
        };
    }

    return {
        true,
        QStringLiteral("User disabled successfully.")
    };
}

UserOperationResult UserService::resetPasswordToDefault(const User& operatorUser,
                                                        const User& targetUser)
{
    if (!canManageUsers(operatorUser)) {
        return {
            false,
            QStringLiteral("You do not have permission to reset passwords.")
        };
    }

    if (!targetUser.isValid()) {
        return {
            false,
            QStringLiteral("Please select a user first.")
        };
    }

    /*
     * 默认密码规则集中放在 Service。
     *
     * View 不应该知道：
     * - 默认密码是多少；
     * - 密码用什么算法哈希；
     * - 密码写入哪个 Repository。
     */
    const QString defaultPasswordHash = SHA256Util::hashPassword(defaultPassword());

    const bool updated = userRepository.updatePasswordByUserId(
        targetUser.userId,
        defaultPasswordHash
        );

    if (!updated) {
        return {
            false,
            QStringLiteral("Failed to reset password. Please check the database.")
        };
    }

    return {
        true,
        QStringLiteral("Password reset successfully. Default password: %1")
            .arg(defaultPassword())
    };
}

QString UserService::defaultPassword()
{
    return QStringLiteral("123456");
}

/*
 * 创建用户输入校验。
 *
 * 规则从 AuthController / UserManagementPage 中收敛到这里，
 * 后续如果要调整用户名长度、密码复杂度，只需要改这一处。
 */
bool UserService::validateCreateUserInput(const QString& username,
                                          const QString& password,
                                          const QString& role,
                                          QString& message) const
{
    if (username.isEmpty()) {
        message = QStringLiteral("Username cannot be empty.");
        return false;
    }

    if (username.length() < 3) {
        message = QStringLiteral("Username must be at least 3 characters.");
        return false;
    }

    if (username.contains(QChar(' '))) {
        message = QStringLiteral("Username cannot contain spaces.");
        return false;
    }

    if (password.isEmpty()) {
        message = QStringLiteral("Password cannot be empty.");
        return false;
    }

    if (password.length() < 6) {
        message = QStringLiteral("Password must be at least 6 characters.");
        return false;
    }

    if (role != QStringLiteral("admin")
        && role != QStringLiteral("user")) {
        message = QStringLiteral("Invalid role selected.");
        return false;
    }

    return true;
}

QString UserService::normalizedRole(const QString& role) const
{
    return role.trimmed().toLower();
}
