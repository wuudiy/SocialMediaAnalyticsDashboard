#include "authcontroller.h"
#include "../services/desutil.h"
#include "../services/sha256util.h"

#include <QDebug>

AuthController::AuthController()
{
}

/*
 * 登录流程：
 * 1. 校验输入；
 * 2. 根据用户名查询用户；
 * 3. 检查用户是否存在；
 * 4. 检查账号是否启用；
 * 5. 使用 SHA-256 校验密码；
 * 6. 如果是旧 DES 密码，登录成功后自动升级成 SHA-256；
 * 7. 保存当前用户。
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

    if (!verifyPasswordAndUpgradeIfNeeded(userFromDatabase, trimmedPassword)) {
        message = "Incorrect password.";
        return false;
    }

    currentUser = userFromDatabase;

    // 数据库里 username 是密文，登录成功后转成明文供界面显示。
    currentUser.username = DESUtil::decrypt(userFromDatabase.username);

    // 登录成功后，当前用户对象中也统一保存 SHA-256 哈希。
    currentUser.password = SHA256Util::hashPassword(trimmedPassword);

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

    const QString passwordHash = SHA256Util::hashPassword(trimmedPassword);

    const bool inserted = userRepository.insertUser(
        trimmedUsername,
        passwordHash,
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

/*
 * 校验密码并升级旧数据
 * ----------------
 * 处理流程：
 * 1. 如果数据库中已经是 SHA-256 哈希，直接计算并比对；
 * 2. 如果数据库中还是旧 DES 密文，先按旧逻辑解密比对；
 * 3. 旧密码比对成功后，立即更新成 SHA-256 哈希；
 * 4. 后续登录就不再走 DES 密码逻辑。
 */
bool AuthController::verifyPasswordAndUpgradeIfNeeded(const User& userFromDatabase,
                                                      const QString& password)
{
    const QString storedPassword = userFromDatabase.password.trimmed();

    if (SHA256Util::isSha256Hash(storedPassword)) {
        return SHA256Util::verifyPassword(password, storedPassword);
    }

    const QString legacyPassword = DESUtil::decrypt(storedPassword);

    if (legacyPassword != password) {
        return false;
    }

    const QString passwordHash = SHA256Util::hashPassword(password);
    const bool upgraded = userRepository.updatePasswordByUserId(
        userFromDatabase.userId,
        passwordHash
        );

    if (!upgraded) {
        qDebug() << "Password verified, but upgrade to SHA-256 failed.";
    }

    return true;
}
