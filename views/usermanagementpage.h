#ifndef USERMANAGEMENTPAGE_H
#define USERMANAGEMENTPAGE_H

#include "../controllers/authcontroller.h"
#include "../models/user.h"
#include "../services/logservice.h"
#include "../services/userrepository.h"

#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class UserManagementPage;
}
QT_END_NAMESPACE

/*
 * 用户管理页面。
 * 管理员可以创建用户、查看用户列表、启用/禁用用户、重置密码。
 */
class UserManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPage(QWidget *parent = nullptr);
    ~UserManagementPage();

    // MainWindow 设置当前登录用户后调用，用于权限判断。
    void setCurrentUser(const User& user);

private slots:
    void onCreateUserClicked();
    void onRefreshUsersClicked();
    void onEnableUserClicked();
    void onDisableUserClicked();
    void onResetPasswordClicked();

private:
    // 初始化 .ui 中控件的运行时属性。
    void prepareUiObjects();

    // 连接页面按钮事件。
    void connectSignals();

    // 应用统一页面样式。
    void applyStyleSheet();

    // 清空创建用户表单。
    void resetForm();

    // 根据权限启用或禁用创建表单。
    void setFormEnabled(bool enabled);

    // 根据权限启用或禁用用户列表区域。
    void setUserListEnabled(bool enabled);

    // 根据当前用户刷新页面权限状态。
    void updateAccessState();

    // 从数据库重新加载用户列表。
    void refreshUserTable();

    // 当前用户是否可以管理用户。
    bool canManageUsers() const;

    // 返回当前选择的角色值：user / admin。
    QString selectedRole() const;

    // 获取表格中当前选中的用户。
    User selectedUserFromTable() const;

    // 设置页面提示信息。
    void setMessage(const QString& message,
                    bool error = false);

    // 写入用户管理操作日志。
    void writeUserManagementLog(const QString& action,
                                const User& targetUser,
                                const QString& detail,
                                bool success);

private:
    Ui::UserManagementPage *ui;

    User currentUser;

    AuthController authController;
    UserRepository userRepository;
    LogService logService;
};

#endif // USERMANAGEMENTPAGE_H
