#ifndef USERMANAGEMENTPAGE_H
#define USERMANAGEMENTPAGE_H

#include "../controllers/authcontroller.h"
#include "../models/user.h"

#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class UserManagementPage;
}
QT_END_NAMESPACE

/*
 * 用户管理页面。
 *
 * 当前版本采用 “usermanagementpage.ui + usermanagementpage.cpp” 分工：
 *
 * 1. forms/usermanagementpage.ui
 *    负责固定界面结构，例如标题、说明文字、输入框、角色下拉框、按钮、提示 Label。
 *
 * 2. views/usermanagementpage.cpp
 *    负责业务逻辑，例如：
 *    - 根据当前登录用户判断权限；
 *    - 调用 AuthController 创建用户；
 *    - 清空表单；
 *    - 显示成功或失败提示。
 *
 * 这样可以减少 cpp 中大量手写控件和布局代码，
 * 后续继续调整 UI 时，可以优先在 Qt Designer 中拖控件完成。
 */
class UserManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPage(QWidget *parent = nullptr);
    ~UserManagementPage();

    // MainWindow 设置当前登录用户后调用，用来刷新页面权限。
    void setCurrentUser(const User& user);

private slots:
    void onCreateUserClicked();

private:
    // 初始化 .ui 中已经创建好的控件，例如 objectName、placeholder、角色下拉框数据。
    void prepareUiObjects();

    // 连接 .ui 控件的信号槽。
    void connectSignals();

    // 应用统一样式。具体 QSS 内容不写在本类里，统一交给 AppStyle。
    void applyStyleSheet();

    // 清空表单并恢复默认输入状态。
    void resetForm();

    // 根据权限启用或禁用表单。
    void setFormEnabled(bool enabled);

    // 根据当前用户角色刷新页面提示和表单状态。
    void updateAccessState();

    // 当前用户是否可以管理账号。
    bool canManageUsers() const;

    // 当前选择的角色值，返回 user 或 admin。
    QString selectedRole() const;

    // 页面内提示信息统一从这里设置，error=true 时显示红色错误提示。
    void setMessage(const QString& message,
                    bool error = false);

private:
    Ui::UserManagementPage *ui;

    User currentUser;
    AuthController authController;
};

#endif // USERMANAGEMENTPAGE_H
