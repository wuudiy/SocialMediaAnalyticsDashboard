#include "authcontroller.h"

AuthController::AuthController()
{
}

/*
 * 登录入口。
 *
 * 具体登录校验、密码校验、旧密码升级、日志写入都在 AuthService 中完成。
 * Controller 只负责保存登录成功后的当前用户。
 */
bool AuthController::loginUser(const QString& username,
                               const QString& password,
                               QString& message)
{
    const AuthLoginResult result = authService.loginUser(username, password);

    message = result.message;

    if (!result.success) {
        currentUser = User();
        return false;
    }

    currentUser = result.user;

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
