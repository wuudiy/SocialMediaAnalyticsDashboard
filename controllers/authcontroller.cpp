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

// 注册新用户
bool AuthController::registerUser(const User& operatorUser,
                                  const QString& username,
                                  const QString& password,
                                  const QString& role,
                                  QString& message)
{
    QString trimmedUsername = username.trimmed();
    QString trimmedPassword = password.trimmed();
    QString trimmedRole = role.trimmed().toLower();

    // 界面可以隐藏按钮，但权限判断还是要放在业务层再挡一次
    if (operatorUser.userId == -1 || operatorUser.role != "admin") {
        message = "Only admin can register new users.";
        return false;
    }

    if (trimmedUsername.isEmpty()) {
        message = "Username cannot be empty.";
        return false;
    }

    if (trimmedUsername.length() < 3) {
        message = "Username must be at least 3 characters.";
        return false;
    }

    if (trimmedUsername.contains(' ')) {
        message = "Username cannot contain spaces.";
        return false;
    }

    if (trimmedPassword.isEmpty()) {
        message = "Password cannot be empty.";
        return false;
    }

    if (trimmedPassword.length() < 6) {
        message = "Password must be at least 6 characters.";
        return false;
    }

    if (trimmedRole != "admin" && trimmedRole != "user") {
        message = "Invalid role selected.";
        return false;
    }

    if (userRepository.usernameExists(trimmedUsername)) {
        message = "The username already exists.";
        return false;
    }

    QString encryptedPassword = DESUtil::encrypt(trimmedPassword);

    bool inserted = userRepository.insertUser(
        trimmedUsername,
        encryptedPassword,
        trimmedRole,
        "active"
        );

    if (!inserted) {
        message = "Failed to register user. Please check the database.";
        return false;
    }

    message = "User registered successfully.";
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
