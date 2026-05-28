#ifndef USERMANAGEMENTPAGE_H
#define USERMANAGEMENTPAGE_H

#include "../models/user.h"

#include <QList>
#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class UserManagementPage;
}
QT_END_NAMESPACE

class UserController;

/*
 * 用户管理页面。
 *
 * MVC 重构后，本类只负责 View 层：
 * - 初始化 UI 控件；
 * - 收集表单输入；
 * - 读取当前表格选中用户；
 * - 显示用户列表；
 * - 显示提示和弹窗；
 * - 发出用户操作信号。
 *
 * 不再负责：
 * - 直接调用 AuthController；
 * - 直接调用 UserRepository；
 * - 直接计算 SHA-256 密码哈希；
 * - 直接写操作日志。
 */
class UserManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPage(QWidget *parent = nullptr);
    ~UserManagementPage();

    // MainWindow 设置当前登录用户后调用。
    void setCurrentUser(const User& user);

public slots:
    // Controller 调用：根据权限刷新页面可用状态。
    void showAccessState(bool allowed);

    // Controller 调用：显示用户列表。
    void showUsers(const QList<User>& users);

    // Controller 调用：显示页面底部提示。
    void showMessage(const QString& message,
                     bool error = false);

    // Controller 调用：统一显示警告弹窗。
    void showWarningMessage(const QString& title,
                            const QString& message);

    // Controller 调用：创建用户成功后的页面处理。
    void handleCreateUserSuccess(const QString& message);

    // Controller 调用：启用用户成功后的页面处理。
    void handleEnableUserSuccess(const QString& message);

    // Controller 调用：禁用用户成功后的页面处理。
    void handleDisableUserSuccess(const QString& message);

    // Controller 调用：重置密码成功后的页面处理。
    void handleResetPasswordSuccess(const QString& message,
                                    const QString& defaultPassword);

signals:
    void refreshUsersRequested();

    void createUserRequested(const QString& username,
                             const QString& password,
                             const QString& role);

    void enableUserRequested(const User& targetUser);

    void disableUserRequested(const User& targetUser);

    void resetPasswordRequested(const User& targetUser);

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

    // 把 Controller 返回的数据填入表格。
    void fillUserTable(const QList<User>& users);

    // 返回当前选择的角色值：user / admin。
    QString selectedRole() const;

    // 获取表格中当前选中的用户。
    User selectedUserFromTable() const;

private:
    Ui::UserManagementPage *ui;

    UserController *userController;
};

#endif // USERMANAGEMENTPAGE_H
