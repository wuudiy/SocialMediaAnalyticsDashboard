#include "LoginView.h"
#include "ui_LoginForm.h"

#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>

/*
 * 构造函数
 * ----------------
 * 1. 初始化 LoginForm.ui；
 * 2. 设置密码框隐藏输入；
 * 3. 清空提示文本；
 * 4. 绑定登录按钮点击事件。
 */
LoginView::LoginView(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);

    // 密码输入框隐藏字符
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    // messageLabel 用于显示错误提示
    if (ui->messageLabel) {
        ui->messageLabel->clear();
    }

    // 点击 loginButton 时，执行 onLoginButtonClicked()
    connect(ui->loginButton, &QPushButton::clicked,
            this, &LoginView::onLoginButtonClicked);
}

LoginView::~LoginView()
{
    delete ui;
}

/*
 * 登录按钮点击事件
 * ----------------
 * View 层只负责：
 * 1. 获取界面输入；
 * 2. 调用 Controller；
 * 3. 根据结果显示提示或发送登录成功信号。
 */
void LoginView::onLoginButtonClicked()
{
    QString username = ui->usernameLineEdit->text();
    QString password = ui->passwordLineEdit->text();

    QString message;

    // 调用 Controller 进行登录验证
    bool success = authController.loginUser(username, password, message);

    if (!success) {
        // 登录失败时保留提示，方便用户知道哪里错了
        if (ui->messageLabel) {
            ui->messageLabel->setText(message);
        }

        QMessageBox::warning(this, "Login Failed", message);
        return;
    }

    // 登录成功后直接进入主界面
    User currentUser = authController.getCurrentUser();

    emit loginSuccess(currentUser);
    this->close();
}

