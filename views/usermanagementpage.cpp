#include "usermanagementpage.h"
#include "../services/appstyle.h"

#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent),
    titleLabel(nullptr),
    tipLabel(nullptr),
    messageLabel(nullptr),
    usernameLineEdit(nullptr),
    passwordLineEdit(nullptr),
    roleComboBox(nullptr),
    createUserButton(nullptr)
{
    buildUi();
}

// 接收当前登录用户，并立即刷新页面权限。
void UserManagementPage::setCurrentUser(const User& user)
{
    currentUser = user;
    updateAccessState();
}

// 创建页面结构：标题说明 + 新增用户表单卡片。
void UserManagementPage::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 22, 24, 24);
    rootLayout->setSpacing(18);

    titleLabel = new QLabel(QStringLiteral("User Management"));
    titleLabel->setObjectName(QStringLiteral("pageTitle"));

    tipLabel = new QLabel(QStringLiteral("Create accounts for people who need access to this dashboard."));
    tipLabel->setObjectName(QStringLiteral("pageSubtitle"));

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(tipLabel);
    rootLayout->addWidget(createFormCard());
    rootLayout->addStretch();

    connect(createUserButton, &QPushButton::clicked,
            this, &UserManagementPage::onCreateUserClicked);

    applyStyleSheet();
    updateAccessState();
}

// 用户管理页复用数据管理页面样式。
// 这里不再写大段 QSS，避免和 PostManagementPage / LogPage 重复。
void UserManagementPage::applyStyleSheet()
{
    setStyleSheet(AppStyle::dataManagementPageStyle());
}

// 创建新增用户表单卡片，表单字段和按钮都在这里初始化。
QFrame* UserManagementPage::createFormCard()
{
    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));
    card->setMaximumWidth(560);

    auto *formLayout = new QGridLayout(card);
    formLayout->setContentsMargins(22, 22, 22, 22);
    formLayout->setHorizontalSpacing(14);
    formLayout->setVerticalSpacing(14);
    formLayout->setColumnStretch(1, 1);

    usernameLineEdit = new QLineEdit();
    usernameLineEdit->setPlaceholderText(QStringLiteral("Enter username"));

    passwordLineEdit = new QLineEdit();
    passwordLineEdit->setPlaceholderText(QStringLiteral("At least 6 characters"));
    passwordLineEdit->setEchoMode(QLineEdit::Password);

    roleComboBox = new QComboBox();
    roleComboBox->addItem(QStringLiteral("User"), QStringLiteral("user"));
    roleComboBox->addItem(QStringLiteral("Admin"), QStringLiteral("admin"));

    createUserButton = new QPushButton(QStringLiteral("Create User"));
    createUserButton->setObjectName(QStringLiteral("primaryButton"));
    createUserButton->setCursor(Qt::PointingHandCursor);

    messageLabel = new QLabel();
    messageLabel->setWordWrap(true);
    messageLabel->setObjectName(QStringLiteral("messageLabel"));

    addFormRow(formLayout, 0, QStringLiteral("Username"), usernameLineEdit);
    addFormRow(formLayout, 1, QStringLiteral("Password"), passwordLineEdit);
    addFormRow(formLayout, 2, QStringLiteral("Role"), roleComboBox);

    formLayout->addWidget(createUserButton, 3, 1);
    formLayout->addWidget(messageLabel, 4, 1);

    return card;
}

// 创建表单字段名，统一 objectName 方便 AppStyle 控制样式。
QLabel* UserManagementPage::createFieldLabel(const QString& text)
{
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("fieldLabel"));

    return label;
}

// 添加一行表单，保持字段名和输入控件布局一致。
void UserManagementPage::addFormRow(QGridLayout *layout,
                                    int row,
                                    const QString& labelText,
                                    QWidget *field)
{
    if (!layout || !field) {
        return;
    }

    layout->addWidget(createFieldLabel(labelText), row, 0);
    layout->addWidget(field, row, 1);
}

// 创建用户按钮事件：收集输入，交给 AuthController 完成注册。
void UserManagementPage::onCreateUserClicked()
{
    if (!canManageUsers()) {
        setMessage(QStringLiteral("You do not have permission to create users."), true);

        QMessageBox::warning(
            this,
            QStringLiteral("Access Denied"),
            QStringLiteral("Only admin users can create users.")
            );

        return;
    }

    QString message;

    const bool success = authController.registerUser(
        currentUser,
        usernameLineEdit->text(),
        passwordLineEdit->text(),
        selectedRole(),
        message
        );

    // 创建失败显示红色提示，创建成功显示普通提示。
    setMessage(message, !success);

    if (!success) {
        QMessageBox::warning(
            this,
            QStringLiteral("Create User Failed"),
            message
            );

        return;
    }

    QMessageBox::information(
        this,
        QStringLiteral("Create User Success"),
        message
        );

    resetForm();
}

// 清空输入框。角色默认回到 User，避免误创建管理员账号。
void UserManagementPage::resetForm()
{
    usernameLineEdit->clear();
    passwordLineEdit->clear();
    roleComboBox->setCurrentIndex(0);
    usernameLineEdit->setFocus();
}

// 统一控制表单可用状态，避免每个控件分散写权限逻辑。
void UserManagementPage::setFormEnabled(bool enabled)
{
    usernameLineEdit->setEnabled(enabled);
    passwordLineEdit->setEnabled(enabled);
    roleComboBox->setEnabled(enabled);
    createUserButton->setEnabled(enabled);
}

// 根据当前用户角色刷新页面提示和表单状态。
void UserManagementPage::updateAccessState()
{
    const bool allowed = canManageUsers();

    setFormEnabled(allowed);

    if (allowed) {
        tipLabel->setText(QStringLiteral("Create accounts for people who need access to this dashboard."));
        setMessage(QString());
        return;
    }

    tipLabel->setText(QStringLiteral("Only admin users can manage accounts."));
    setMessage(QStringLiteral("You do not have permission to create users."), true);
}

// 管理员判断统一收口，后续扩展权限规则时只改这里。
bool UserManagementPage::canManageUsers() const
{
    return currentUser.isValid()
    && currentUser.isActive()
        && currentUser.isAdmin();
}

// 从下拉框读取真实角色值，避免使用界面显示文本做业务判断。
QString UserManagementPage::selectedRole() const
{
    return roleComboBox->currentData().toString();
}

// 设置页面内提示信息，避免多处直接操作 messageLabel。
// error=true 时复用 AppStyle 的错误提示颜色。
void UserManagementPage::setMessage(const QString& message,
                                    bool error)
{
    messageLabel->setText(message);
    messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}
