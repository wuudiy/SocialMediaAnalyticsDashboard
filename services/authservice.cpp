#include "authservice.h"

#include "../utils/desutil.h"
#include "../utils/sha256util.h"

#include <QDebug>

AuthService::AuthService()
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
 * 7. 构造当前登录用户；
 * 8. 写入登录日志。
 */
AuthLoginResult AuthService::loginUser(const QString& username,
                                       const QString& password)
{
    AuthLoginResult result;

    const QString trimmedUsername = username.trimmed();
    const QString trimmedPassword = password.trimmed();

    if (!validateLoginInput(trimmedUsername, trimmedPassword, result.message)) {
        logService.writeLog(
            -1,
            trimmedUsername,
            QStringLiteral("login"),
            result.message,
            QStringLiteral("failed")
            );

        return result;
    }

    const User userFromDatabase = userRepository.findByUsername(trimmedUsername);

    if (!userFromDatabase.isValid()) {
        result.message = QStringLiteral("User does not exist.");

        logService.writeLog(
            -1,
            trimmedUsername,
            QStringLiteral("login"),
            QStringLiteral("Login failed: user does not exist."),
            QStringLiteral("failed")
            );

        return result;
    }

    if (!userFromDatabase.isActive()) {
        result.message = QStringLiteral("This account is disabled.");

        logService.writeLog(
            userFromDatabase.userId,
            trimmedUsername,
            QStringLiteral("login"),
            QStringLiteral("Login failed: account is disabled."),
            QStringLiteral("failed")
            );

        return result;
    }

    if (!verifyPasswordAndUpgradeIfNeeded(userFromDatabase, trimmedPassword)) {
        result.message = QStringLiteral("Incorrect password.");

        logService.writeLog(
            userFromDatabase.userId,
            trimmedUsername,
            QStringLiteral("login"),
            QStringLiteral("Login failed: incorrect password."),
            QStringLiteral("failed")
            );

        return result;
    }

    result.success = true;
    result.message = QStringLiteral("Login successful.");
    result.user = buildLoginUser(userFromDatabase, trimmedPassword);

    logService.writeLog(
        result.user.userId,
        result.user.username,
        QStringLiteral("login"),
        QStringLiteral("Login successful. Role: %1").arg(result.user.role),
        QStringLiteral("success")
        );

    return result;
}

/*
 * 校验登录输入，避免空用户名或空密码继续进入数据库查询。
 */
bool AuthService::validateLoginInput(const QString& username,
                                     const QString& password,
                                     QString& message) const
{
    if (username.isEmpty()) {
        message = QStringLiteral("Username cannot be empty.");
        return false;
    }

    if (password.isEmpty()) {
        message = QStringLiteral("Password cannot be empty.");
        return false;
    }

    return true;
}

/*
 * 校验密码并升级旧密码。
 *
 * 处理流程：
 * 1. 如果数据库中已经是 SHA-256 哈希，直接计算并比对；
 * 2. 如果数据库中还是旧 DES 密文，先按旧逻辑解密比对；
 * 3. 旧密码比对成功后，立即更新成 SHA-256 哈希；
 * 4. 后续登录就不再走 DES 密码逻辑。
 */
bool AuthService::verifyPasswordAndUpgradeIfNeeded(const User& userFromDatabase,
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

/*
 * 构造登录成功后的当前用户。
 *
 * 注意：
 * 数据库中的 username 是 DES 密文。
 * 界面显示、日志记录、MainWindow 当前用户状态都应该使用明文用户名。
 */
User AuthService::buildLoginUser(const User& userFromDatabase,
                                 const QString& plainPassword) const
{
    User loginUser = userFromDatabase;

    loginUser.username = DESUtil::decrypt(userFromDatabase.username);
    loginUser.password = SHA256Util::hashPassword(plainPassword);

    return loginUser;
}
