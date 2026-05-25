#ifndef USERMANAGEMENTPAGE_H
#define USERMANAGEMENTPAGE_H

#include "../controllers/authcontroller.h"
#include "../models/user.h"

#include <QWidget>

class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QFrame;
class QGridLayout;

/*
 * 用户管理页面。
 *
 * 负责：
 * - 管理员创建新用户；
 * - 根据当前用户角色启用或禁用表单；
 * - 显示创建结果提示。
 *
 * 不负责：
 * - 直接写数据库；
 * - 判断登录状态；
 * - 管理主窗口导航；
 * - 管理具体样式细节。
 *
 * 样式说明：
 * 页面样式统一交给 AppStyle 管理，本类只负责页面结构和业务交互。
 */
class UserManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPage(QWidget *parent = nullptr);

    // MainWindow 设置当前登录用户后调用，用来刷新页面权限。
    void setCurrentUser(const User& user);

private slots:
    void onCreateUserClicked();

private:
    // 创建页面整体布局。
    void buildUi();

    // 应用页面样式。具体 QSS 内容不写在这里，统一交给 AppStyle。
    void applyStyleSheet();

    // 创建新增用户表单卡片。
    QFrame* createFormCard();

    // 创建表单左侧字段名。
    QLabel* createFieldLabel(const QString& text);

    // 添加一行表单项。
    void addFormRow(QGridLayout *layout,
                    int row,
                    const QString& labelText,
                    QWidget *field);

    // 清空表单并恢复默认输入状态。
    void resetForm();

    // 根据权限启用或禁用表单。
    void setFormEnabled(bool enabled);

    // 刷新当前用户能看到和操作的内容。
    void updateAccessState();

    // 当前用户是否可以管理账号。
    bool canManageUsers() const;

    // 当前选择的角色值，返回 user 或 admin。
    QString selectedRole() const;

    // 页面内提示信息统一从这里设置，error=true 时显示红色错误提示。
    void setMessage(const QString& message,
                    bool error = false);

private:
    User currentUser;
    AuthController authController;

    QLabel *titleLabel;
    QLabel *tipLabel;
    QLabel *messageLabel;

    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QComboBox *roleComboBox;
    QPushButton *createUserButton;
};

#endif // USERMANAGEMENTPAGE_H
