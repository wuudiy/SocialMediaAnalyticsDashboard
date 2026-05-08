#include "AuthController.h"
#include "../services/DESUtil.h"

AuthController::AuthController()
{
}
/*
 * 登录流程：
 * 1. 校验输入；
 * 2. 根据用户名查询用户；
 * 3. 检查用户是否存在；
 * 4. 检查账号是否启用；
 * 5. 解密密码并比对；
 * 6. 保存当前用户。
 */

// 登录用户：校验输入、查询用户、检查状态、比对密码。
bool AuthController::loginUser(const QString& username,
                               const QString& password,
                               QString& message)
{
    const QString trimmedUsername = username.trimmed();
    const QString trimmedPassword = password.trimmed();

    if (!validateLoginInput(trimmedUsername, trimmedPassword, message)) {
        return false;
    }

    const User userFromDatabase = userRepository.findByUsername(trimmedUsername);

    if (!userFromDatabase.isValid()) {
        message = "User does not exist.";
        return false;
    }

    if (!userFromDatabase.isActive()) {
        message = "This account is disabled.";
        return false;
    }

    const QString realPassword = DESUtil::decrypt(userFromDatabase.password);

    if (realPassword != trimmedPassword) {
        message = "Incorrect password.";
        return false;
    }

    currentUser = userFromDatabase;

    // 数据库里 username 是密文，登录成功后转成明文供界面显示。
    currentUser.username = DESUtil::decrypt(userFromDatabase.username);

    message = "Login successful.";
    return true;
}

// 注册用户：只允许管理员操作，并在写入前完成必要校验。
bool AuthController::registerUser(const User& operatorUser,
                                  const QString& username,
                                  const QString& password,
                                  const QString& role,
                                  QString& message)
{
    const QString trimmedUsername = username.trimmed();
    const QString trimmedPassword = password.trimmed();
    const QString normalizedRole = role.trimmed().toLower();

    if (!canCreateUser(operatorUser)) {
        message = "Only admin can register new users.";
        return false;
    }

    if (!validateRegisterInput(trimmedUsername,
                               trimmedPassword,
                               normalizedRole,
                               message)) {
        return false;
    }

    if (userRepository.usernameExists(trimmedUsername)) {
        message = "The username already exists.";
        return false;
    }

    const QString encryptedPassword = DESUtil::encrypt(trimmedPassword);

    const bool inserted = userRepository.insertUser(
        trimmedUsername,
        encryptedPassword,
        normalizedRole,
        QStringLiteral("active")
        );

    if (!inserted) {
        message = "Failed to register user. Please check the database.";
        return false;
    }

    message = "User registered successfully.";
    return true;
}

// 获取当前用户角色，主要给界面判断权限使用。
QString AuthController::getCurrentUserRole() const
{
    return currentUser.role;
}

// 获取当前登录用户，登录失败时返回默认无效用户。
User AuthController::getCurrentUser() const
{
    return currentUser;
}

// 校验登录输入，避免空值继续进入数据库查询。
bool AuthController::validateLoginInput(const QString& username,
                                        const QString& password,
                                        QString& message) const
{
    if (username.isEmpty()) {
        message = "Username cannot be empty.";
        return false;
    }

    if (password.isEmpty()) {
        message = "Password cannot be empty.";
        return false;
    }

    return true;
}

// 校验注册输入，规则集中放这里，避免散落在界面代码里。
bool AuthController::validateRegisterInput(const QString& username,
                                           const QString& password,
                                           const QString& role,
                                           QString& message) const
{
    if (username.isEmpty()) {
        message = "Username cannot be empty.";
        return false;
    }

    if (username.length() < 3) {
        message = "Username must be at least 3 characters.";
        return false;
    }

    if (username.contains(' ')) {
        message = "Username cannot contain spaces.";
        return false;
    }

    if (password.isEmpty()) {
        message = "Password cannot be empty.";
        return false;
    }

    if (password.length() < 6) {
        message = "Password must be at least 6 characters.";
        return false;
    }

    if (role != QStringLiteral("admin") && role != QStringLiteral("user")) {
        message = "Invalid role selected.";
        return false;
    }

    return true;
}

// 创建用户权限统一收口，后续扩展角色权限时只改这里。
bool AuthController::canCreateUser(const User& operatorUser) const
{
    return operatorUser.isValid() && operatorUser.isActive() && operatorUser.isAdmin();
}

