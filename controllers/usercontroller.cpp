#include "usercontroller.h"

#include "../views/usermanagementpage.h"

UserController::UserController(UserManagementPage *view,
                               QObject *parent)
    : QObject(parent),
    view(view)
{
    /*
     * Controller 接管用户管理页面的业务请求。
     *
     * View 只负责发出：
     * - createUserRequested
     * - refreshUsersRequested
     * - enableUserRequested
     * - disableUserRequested
     * - resetPasswordRequested
     *
     * 具体业务和日志都在 Controller / Service 中处理。
     */
    connect(view, &UserManagementPage::refreshUsersRequested,
            this, &UserController::handleRefreshUsers);

    connect(view, &UserManagementPage::createUserRequested,
            this, &UserController::handleCreateUser);

    connect(view, &UserManagementPage::enableUserRequested,
            this, &UserController::handleEnableUser);

    connect(view, &UserManagementPage::disableUserRequested,
            this, &UserController::handleDisableUser);

    connect(view, &UserManagementPage::resetPasswordRequested,
            this, &UserController::handleResetPassword);
}

void UserController::setCurrentUser(const User& user)
{
    currentUser = user;

    const bool allowed = userService.canManageUsers(currentUser);
    view->showAccessState(allowed);

    if (!allowed) {
        view->showUsers(QList<User>());
        view->showMessage(
            QStringLiteral("You do not have permission to manage users."),
            true
            );
        return;
    }

    /*
     * 登录用户切换后立即刷新用户列表，
     * 保持 MainWindow 原有 setCurrentUser() 行为不变。
     */
    view->showUsers(userService.findAllUsers());
    view->showMessage(QString());
}

void UserController::handleRefreshUsers()
{
    if (!userService.canManageUsers(currentUser)) {
        view->showAccessState(false);
        view->showUsers(QList<User>());
        view->showMessage(
            QStringLiteral("You do not have permission to view users."),
            true
            );
        return;
    }

    view->showAccessState(true);
    view->showUsers(userService.findAllUsers());
    view->showMessage(QStringLiteral("User list refreshed."));
}

void UserController::handleCreateUser(const QString& username,
                                      const QString& password,
                                      const QString& role)
{
    const UserOperationResult result = userService.createUser(
        currentUser,
        username,
        password,
        role
        );

    const User targetUser = buildTargetUserForCreate(username, role);

    writeUserManagementLog(
        QStringLiteral("create_user"),
        targetUser,
        result.success
            ? QStringLiteral("Create user successful")
            : QStringLiteral("Create user failed: %1").arg(result.message),
        result.success ? QStringLiteral("success") : QStringLiteral("failed")
        );

    if (!result.success) {
        view->showMessage(result.message, true);
        view->showWarningMessage(
            QStringLiteral("Create User Failed"),
            result.message
            );
        return;
    }

    view->handleCreateUserSuccess(result.message);
    view->showUsers(userService.findAllUsers());
}

void UserController::handleEnableUser(const User& targetUser)
{
    const UserOperationResult result = userService.enableUser(
        currentUser,
        targetUser
        );

    writeUserManagementLog(
        QStringLiteral("enable_user"),
        targetUser,
        result.success
            ? QStringLiteral("Enable user")
            : QStringLiteral("Enable user failed: %1").arg(result.message),
        result.success ? QStringLiteral("success") : QStringLiteral("failed")
        );

    if (!result.success) {
        view->showMessage(result.message, true);
        view->showWarningMessage(
            QStringLiteral("Enable User Failed"),
            result.message
            );
        return;
    }

    view->handleEnableUserSuccess(result.message);
    view->showUsers(userService.findAllUsers());
}

void UserController::handleDisableUser(const User& targetUser)
{
    const UserOperationResult result = userService.disableUser(
        currentUser,
        targetUser
        );

    writeUserManagementLog(
        QStringLiteral("disable_user"),
        targetUser,
        result.success
            ? QStringLiteral("Disable user")
            : QStringLiteral("Disable user failed: %1").arg(result.message),
        result.success ? QStringLiteral("success") : QStringLiteral("failed")
        );

    if (!result.success) {
        view->showMessage(result.message, true);
        view->showWarningMessage(
            QStringLiteral("Disable User Failed"),
            result.message
            );
        return;
    }

    view->handleDisableUserSuccess(result.message);
    view->showUsers(userService.findAllUsers());
}

void UserController::handleResetPassword(const User& targetUser)
{
    const UserOperationResult result = userService.resetPasswordToDefault(
        currentUser,
        targetUser
        );

    writeUserManagementLog(
        QStringLiteral("reset_password"),
        targetUser,
        result.success
            ? QStringLiteral("Reset password to default value")
            : QStringLiteral("Reset password failed: %1").arg(result.message),
        result.success ? QStringLiteral("success") : QStringLiteral("failed")
        );

    if (!result.success) {
        view->showMessage(result.message, true);
        view->showWarningMessage(
            QStringLiteral("Reset Password Failed"),
            result.message
            );
        return;
    }

    view->handleResetPasswordSuccess(result.message, UserService::defaultPassword());
    view->showUsers(userService.findAllUsers());
}

QString UserController::currentOperatorName() const
{
    if (currentUser.isValid() && !currentUser.username.trimmed().isEmpty()) {
        return currentUser.username.trimmed();
    }

    return QStringLiteral("unknown");
}

/*
 * 用户管理操作写入日志。
 *
 * 原来这段逻辑在 UserManagementPage::writeUserManagementLog() 中，
 * 现在移动到 Controller，避免 View 直接依赖 LogService。
 */
void UserController::writeUserManagementLog(const QString& action,
                                            const User& targetUser,
                                            const QString& detail,
                                            const QString& result)
{
    const QString logDetail = QStringLiteral("%1. Target user_id: %2, username: %3, role: %4, status: %5")
    .arg(detail)
        .arg(targetUser.userId)
        .arg(targetUser.username)
        .arg(targetUser.role)
        .arg(targetUser.status);

    logService.writeLog(
        currentUser.isValid() ? currentUser.userId : -1,
        currentOperatorName(),
        action,
        logDetail,
        result
        );
}

User UserController::buildTargetUserForCreate(const QString& username,
                                              const QString& role) const
{
    User targetUser;

    /*
     * 新建用户时 insertUser() 当前没有返回自增 ID，
     * 所以这里 userId 保持默认 -1。
     *
     * 日志中仍然记录 username / role / status，
     * 足够追踪创建操作。
     */
    targetUser.username = username.trimmed();
    targetUser.role = role.trimmed().toLower();
    targetUser.status = QStringLiteral("active");

    return targetUser;
}
