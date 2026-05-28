#ifndef USERCONTROLLER_H
#define USERCONTROLLER_H

#include "../models/user.h"
#include "../services/logservice.h"
#include "../services/userservice.h"

#include <QObject>
#include <QString>

class UserManagementPage;

/*
 * 用户管理控制器。
 *
 * 负责连接 UserManagementPage 和 UserService：
 * - 接收页面发出的用户操作信号；
 * - 调用 UserService 完成业务；
 * - 写入操作日志；
 * - 通知页面刷新表格和显示结果。
 *
 * 不负责：
 * - 创建控件；
 * - 直接写 SQL；
 * - 直接计算密码哈希。
 */
class UserController : public QObject
{
    Q_OBJECT

public:
    explicit UserController(UserManagementPage *view,
                            QObject *parent = nullptr);

    void setCurrentUser(const User& user);

private slots:
    void handleRefreshUsers();

    void handleCreateUser(const QString& username,
                          const QString& password,
                          const QString& role);

    void handleEnableUser(const User& targetUser);

    void handleDisableUser(const User& targetUser);

    void handleResetPassword(const User& targetUser);

private:
    QString currentOperatorName() const;

    void writeUserManagementLog(const QString& action,
                                const User& targetUser,
                                const QString& detail,
                                const QString& result);

    User buildTargetUserForCreate(const QString& username,
                                  const QString& role) const;

private:
    UserManagementPage *view;
    User currentUser;

    UserService userService;
    LogService logService;
};

#endif // USERCONTROLLER_H
