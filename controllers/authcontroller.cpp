#include "AuthController.h"
#include "../services/DESUtil.h"

AuthController::AuthController()
{
}

/*
 * 登录验证流程
 * ----------------
 * 1. 去除输入前后空格；
 * 2. 检查用户名和密码是否为空；
 * 3. 根据用户名查询数据库用户；
 * 4. 判断用户是否存在；
 * 5. 判断账号是否启用；
 * 6. 解密数据库中的密码；
 * 7. 与用户输入密码进行比较；
 * 8. 登录成功后保存 currentUser。
 */
bool AuthController::loginUser(const QString& username,
                               const QString& password,
                               QString& message)
{
    QString trimmedUsername = username.trimmed();
    QString trimmedPassword = password.trimmed();

    // 输入验证：用户名不能为空
    if (trimmedUsername.isEmpty()) {
        message = "Username cannot be empty.";
        return false;
    }

    // 输入验证：密码不能为空
    if (trimmedPassword.isEmpty()) {
        message = "Password cannot be empty.";
        return false;
    }

    // 查询用户。Repository 内部会自动加密 username 后再查询。
    User user = userRepository.findByUsername(trimmedUsername);

    // userId == -1 表示没有查询到用户
    if (user.userId == -1) {
        message = "User does not exist.";
        return false;
    }

    // 检查账号状态
    if (user.status != "active") {
        message = "This account is disabled.";
        return false;
    }

    // 数据库中的 password 是 DES 密文，这里需要解密后再比较
    QString decryptedPassword = DESUtil::decrypt(user.password);

    if (decryptedPassword != trimmedPassword) {
        message = "Incorrect password.";
        return false;
    }

    // 数据库中的 username 也是密文。
    // 登录成功后，把 User 对象中的 username 改成明文，方便界面显示。
    user.username = DESUtil::decrypt(user.username);

    currentUser = user;
    message = "Login successful.";
    return true;
}

QString AuthController::getCurrentUserRole() const
{
    return currentUser.role;
}

User AuthController::getCurrentUser() const
{
    return currentUser;
}
