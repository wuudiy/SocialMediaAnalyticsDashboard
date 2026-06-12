#include "loginview.h"
#include "ui_LoginForm.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

LoginView::LoginView(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::LoginForm)
{
    ui->setupUi(this);

    setupWindow();
    rebuildLayout();
    applyStyleSheet();
    connectSignals();

    clearMessage();
    focusUsername();
}

LoginView::~LoginView()
{
    delete ui;
}
void LoginView::prepareForNextLogin()
{
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    clearMessage();

    show();
    raise();
    activateWindow();
    focusUsername();
}

// 初始化登录页基础属性，具体布局交给 rebuildLayout。
void LoginView::setupWindow()
{
    setObjectName(QStringLiteral("loginRoot"));
    setWindowTitle(QStringLiteral("Login - Social Media Analytics Dashboard"));

    resize(900, 620);
    setMinimumSize(760, 520);

    ui->label->setText(QStringLiteral("Username"));
    ui->label_2->setText(QStringLiteral("Password"));

    ui->label->setObjectName(QStringLiteral("fieldLabel"));
    ui->label_2->setObjectName(QStringLiteral("fieldLabel"));

    ui->usernameLineEdit->setPlaceholderText(QStringLiteral("Enter username"));
    ui->passwordLineEdit->setPlaceholderText(QStringLiteral("Enter password"));
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    ui->loginButton->setText(QStringLiteral("Login"));
    ui->loginButton->setObjectName(QStringLiteral("primaryButton"));
    ui->loginButton->setCursor(Qt::PointingHandCursor);

    ui->messageLabel->setObjectName(QStringLiteral("messageLabel"));
    ui->messageLabel->setWordWrap(true);
}

// 重新组织 .ui 里的控件，避免继续依赖 Designer 里的绝对坐标。
void LoginView::rebuildLayout()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(32, 32, 32, 32);
    rootLayout->setSpacing(0);

    auto *card = new QFrame(this);
    card->setObjectName(QStringLiteral("loginCard"));
    card->setFixedWidth(430);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(34, 34, 34, 30);
    cardLayout->setSpacing(18);

    auto *titleLabel = new QLabel(QStringLiteral("Social Media Analytics"));
    titleLabel->setObjectName(QStringLiteral("loginTitle"));
    titleLabel->setAlignment(Qt::AlignCenter);

    auto *subtitleLabel = new QLabel(QStringLiteral("Sign in to continue to the dashboard."));
    subtitleLabel->setObjectName(QStringLiteral("loginSubtitle"));
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);

    auto *formLayout = new QGridLayout();
    formLayout->setHorizontalSpacing(14);
    formLayout->setVerticalSpacing(14);
    formLayout->setColumnStretch(1, 1);

    formLayout->addWidget(ui->label, 0, 0);
    formLayout->addWidget(ui->usernameLineEdit, 0, 1);
    formLayout->addWidget(ui->label_2, 1, 0);
    formLayout->addWidget(ui->passwordLineEdit, 1, 1);

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(subtitleLabel);
    cardLayout->addSpacing(8);
    cardLayout->addLayout(formLayout);
    cardLayout->addWidget(ui->loginButton);
    cardLayout->addWidget(ui->messageLabel);

    rootLayout->addStretch();
    rootLayout->addWidget(card, 0, Qt::AlignHCenter);
    rootLayout->addStretch();
}

// 集中管理登录页样式，后续调整视觉效果优先改这里。
void LoginView::applyStyleSheet()
{
    setStyleSheet(
        "QWidget#loginRoot {"
        "    background: #F3F4F6;"
        "}"
        "QFrame#loginCard {"
        "    background: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 14px;"
        "}"
        "QLabel#loginTitle {"
        "    color: #111827;"
        "    font-size: 24px;"
        "    font-weight: 700;"
        "}"
        "QLabel#loginSubtitle {"
        "    color: #6B7280;"
        "    font-size: 13px;"
        "}"
        "QLabel#fieldLabel {"
        "    color: #374151;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QLineEdit {"
        "    min-height: 36px;"
        "    border: 1px solid #D1D5DB;"
        "    border-radius: 8px;"
        "    padding-left: 10px;"
        "    background: #FFFFFF;"
        "    color: #111827;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid #2563EB;"
        "}"
        "QPushButton#primaryButton {"
        "    min-height: 38px;"
        "    background: #2563EB;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 8px;"
        "    font-weight: 600;"
        "}"
        "QPushButton#primaryButton:hover {"
        "    background: #1D4ED8;"
        "}"
        "QPushButton#primaryButton:pressed {"
        "    background: #1E40AF;"
        "}"
        "QLabel#messageLabel {"
        "    min-height: 22px;"
        "    color: #DC2626;"
        "    font-size: 13px;"
        "}"
        );
}

// 绑定登录相关事件：按钮点击、密码框回车、输入后清除错误提示。
void LoginView::connectSignals()
{
    connect(ui->loginButton, &QPushButton::clicked,
            this, &LoginView::onLoginButtonClicked);

    connect(ui->passwordLineEdit, &QLineEdit::returnPressed,
            this, &LoginView::onLoginButtonClicked);

    connect(ui->usernameLineEdit, &QLineEdit::returnPressed,
            this, &LoginView::focusPassword);

    connect(ui->usernameLineEdit, &QLineEdit::textChanged,
            this, [this]() {
                clearMessage();
            });

    connect(ui->passwordLineEdit, &QLineEdit::textChanged,
            this, [this]() {
                clearMessage();
            });
}

// 登录按钮事件：读取输入，交给 AuthController，按结果更新界面。
void LoginView::onLoginButtonClicked()
{
    clearMessage();

    const LoginCredentials credentials = readCredentials();

    QString message;

    const bool success = authController.loginUser(
        credentials.username,
        credentials.password,
        message
        );

    if (!success) {
        handleLoginFailure(message);
        return;
    }

    handleLoginSuccess(authController.getCurrentUser());
}

// 读取登录表单内容。具体校验规则放在 AuthController。
LoginView::LoginCredentials LoginView::readCredentials() const
{
    LoginCredentials credentials;

    credentials.username = ui->usernameLineEdit->text();
    credentials.password = ui->passwordLineEdit->text();

    return credentials;
}

// 清空页面内错误提示。
void LoginView::clearMessage()
{
    setMessage(QString());
}

// 设置页面内提示信息。
void LoginView::setMessage(const QString& message)
{
    ui->messageLabel->setText(message);
}

// 聚焦用户名输入框。
void LoginView::focusUsername()
{
    ui->usernameLineEdit->setFocus();
}

// 聚焦密码输入框，并选中原内容，方便用户直接重输。
void LoginView::focusPassword()
{
    ui->passwordLineEdit->setFocus();
    ui->passwordLineEdit->selectAll();
}

// 登录失败后显示提示，并把焦点放回更可能需要修改的输入框。
void LoginView::handleLoginFailure(const QString& message)
{
    setMessage(message);

    if (ui->usernameLineEdit->text().trimmed().isEmpty()) {
        focusUsername();
    } else {
        focusPassword();
    }

    QMessageBox::warning(
        this,
        QStringLiteral("Login Failed"),
        message
        );
}

// 登录成功后通知外部打开主窗口，LoginView 不直接依赖 MainWindow。
void LoginView::handleLoginSuccess(const User& user)
{
    emit loginSuccess(user);
    close();
}