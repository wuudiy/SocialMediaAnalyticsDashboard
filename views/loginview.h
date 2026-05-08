#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include "../controllers/authcontroller.h"
#include "../models/user.h"

#include <QString>
#include <QWidget>

namespace Ui {
class LoginForm;
}

/*
 * 登录窗口。
 *
 * 负责：
 * - 展示登录表单；
 * - 读取用户名和密码；
 * - 调用 AuthController 完成登录；
 * - 登录成功后发出 loginSuccess 信号。
 *
 * 不负责：
 * - 直接查询数据库；
 * - 保存全局登录状态；
 * - 创建 MainWindow。
 */
class LoginView : public QWidget
{
    Q_OBJECT

public:
    explicit LoginView(QWidget *parent = nullptr);
    ~LoginView();

signals:
    // 登录成功后把当前用户交给 main.cpp。
    void loginSuccess(const User& user);

private slots:
    void onLoginButtonClicked();

private:
    struct LoginCredentials
    {
        QString username;
        QString password;
    };

private:
    // 初始化窗口标题、输入框提示、密码框模式等基础属性。
    void setupWindow();

    // 使用代码重新整理 LoginForm.ui 里的控件布局。
    void rebuildLayout();

    // 登录窗口统一样式。
    void applyStyleSheet();

    // 绑定按钮、回车键和输入变化事件。
    void connectSignals();

    // 从界面读取登录输入。
    LoginCredentials readCredentials() const;

    // 清空页面内提示。
    void clearMessage();

    // 设置页面内提示。
    void setMessage(const QString& message);

    // 聚焦用户名输入框。
    void focusUsername();

    // 聚焦密码输入框。
    void focusPassword();

    // 登录失败后的界面反馈。
    void handleLoginFailure(const QString& message);

    // 登录成功后发出信号并关闭登录窗口。
    void handleLoginSuccess(const User& user);

private:
    Ui::LoginForm *ui;
    AuthController authController;
};

#endif // LOGINVIEW_H
