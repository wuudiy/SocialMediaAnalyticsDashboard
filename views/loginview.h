#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QWidget>
#include "../controllers/AuthController.h"
#include "../models/User.h"

namespace Ui {
class LoginForm;
}

/*
 * View 层：LoginView
 * 作用：
 * 1. 加载 LoginForm.ui；
 * 2. 获取用户输入；
 * 3. 调用 AuthController 处理登录逻辑；
 * 4. 登录成功后发送 loginSuccess 信号。
 */
class LoginView : public QWidget
{
    Q_OBJECT

public:
    explicit LoginView(QWidget *parent = nullptr);
    ~LoginView();

signals:
    // 登录成功后，把当前用户传给 main.cpp，由 main.cpp 打开 MainWindow
    void loginSuccess(const User& user);

private slots:
    // 登录按钮点击事件
    void onLoginButtonClicked();

private:
    Ui::LoginForm *ui;              // Qt Designer 自动生成的 UI 对象
    AuthController authController;  // 登录控制器
};

#endif // LOGINVIEW_H
