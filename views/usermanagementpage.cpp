#include "usermanagementpage.h"
#include "ui_usermanagementpage.h"

#include "../services/appstyle.h"
#include "../services/sha256util.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::UserManagementPage)
{
    ui->setupUi(this);

    prepareUiObjects();
    connectSignals();
    applyStyleSheet();
    updateAccessState();
}

UserManagementPage::~UserManagementPage()
{
    delete ui;
}

// 接收当前登录用户，并立即刷新页面权限。
void UserManagementPage::setCurrentUser(const User& user)
{
    currentUser = user;
    updateAccessState();
}

/*
 * 初始化 .ui 中已有控件的运行时属性。
 * 固定结构放在 .ui 文件中，这里只设置样式名、输入提示和表格行为。
 */
void UserManagementPage::prepareUiObjects()
{
    setObjectName(QStringLiteral("userManagementPage"));

    ui->pageTitleLabel->setObjectName(QStringLiteral("pageTitle"));
    ui->pageSubtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));

    ui->formCard->setObjectName(QStringLiteral("card"));
    ui->userListCard->setObjectName(QStringLiteral("card"));

    ui->usernameLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->passwordLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->roleLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->userListTitleLabel->setObjectName(QStringLiteral("fieldLabel"));

    ui->messageLabel->setObjectName(QStringLiteral("messageLabel"));
    ui->userListHintLabel->setObjectName(QStringLiteral("messageLabel"));

    ui->pageSubtitleLabel->setWordWrap(true);
    ui->messageLabel->setWordWrap(true);
    ui->userListHintLabel->setWordWrap(true);

    ui->formLayout->setColumnStretch(1, 1);

    ui->usernameLineEdit->setPlaceholderText(QStringLiteral("Enter username"));

    ui->passwordLineEdit->setPlaceholderText(QStringLiteral("At least 6 characters"));
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    // 下拉框显示文本和真实业务值分开，避免用界面文本参与逻辑判断。
    ui->roleComboBox->clear();
    ui->roleComboBox->addItem(QStringLiteral("User"), QStringLiteral("user"));
    ui->roleComboBox->addItem(QStringLiteral("Admin"), QStringLiteral("admin"));

    ui->createUserButton->setObjectName(QStringLiteral("primaryButton"));
    ui->refreshUsersButton->setObjectName(QStringLiteral("secondaryButton"));
    ui->enableUserButton->setObjectName(QStringLiteral("successButton"));
    ui->disableUserButton->setObjectName(QStringLiteral("dangerButton"));
    ui->resetPasswordButton->setObjectName(QStringLiteral("secondaryButton"));

    ui->createUserButton->setCursor(Qt::PointingHandCursor);
    ui->refreshUsersButton->setCursor(Qt::PointingHandCursor);
    ui->enableUserButton->setCursor(Qt::PointingHandCursor);
    ui->disableUserButton->setCursor(Qt::PointingHandCursor);
    ui->resetPasswordButton->setCursor(Qt::PointingHandCursor);

    ui->userTable->setColumnCount(4);
    ui->userTable->setHorizontalHeaderLabels({
        QStringLiteral("ID"),
        QStringLiteral("Username"),
        QStringLiteral("Role"),
        QStringLiteral("Status")
    });

    ui->userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 隐藏左侧行号列，让表格在默认窗口下更紧凑。
    ui->userTable->verticalHeader()->setVisible(false);
    ui->userTable->verticalHeader()->setDefaultSectionSize(36);

    ui->userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->userTable->verticalHeader()->setVisible(false);
    ui->userTable->verticalHeader()->setDefaultSectionSize(34);
    ui->userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->userTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->userTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->userTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->userTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->userTable->setAlternatingRowColors(true);
    ui->userTable->setSortingEnabled(true);
}

// 集中连接信号槽，便于维护页面行为。
void UserManagementPage::connectSignals()
{
    connect(ui->createUserButton, &QPushButton::clicked,
            this, &UserManagementPage::onCreateUserClicked);

    connect(ui->refreshUsersButton, &QPushButton::clicked,
            this, &UserManagementPage::onRefreshUsersClicked);

    connect(ui->enableUserButton, &QPushButton::clicked,
            this, &UserManagementPage::onEnableUserClicked);

    connect(ui->disableUserButton, &QPushButton::clicked,
            this, &UserManagementPage::onDisableUserClicked);

    connect(ui->resetPasswordButton, &QPushButton::clicked,
            this, &UserManagementPage::onResetPasswordClicked);
}

// 复用已有数据管理页面样式，避免重复写 QSS。
void UserManagementPage::applyStyleSheet()
{
    setStyleSheet(AppStyle::dataManagementPageStyle());
}

// 创建用户仍然复用 AuthController 中已有注册逻辑。
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
        ui->usernameLineEdit->text(),
        ui->passwordLineEdit->text(),
        selectedRole(),
        message
        );

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
    refreshUserTable();
}

void UserManagementPage::onRefreshUsersClicked()
{
    if (!canManageUsers()) {
        setMessage(QStringLiteral("You do not have permission to view users."), true);
        return;
    }

    refreshUserTable();
    setMessage(QStringLiteral("User list refreshed."));
}

void UserManagementPage::onEnableUserClicked()
{
    if (!canManageUsers()) {
        setMessage(QStringLiteral("You do not have permission to enable users."), true);
        return;
    }

    const User targetUser = selectedUserFromTable();

    if (!targetUser.isValid()) {
        setMessage(QStringLiteral("Please select a user first."), true);
        return;
    }

    if (targetUser.isActive()) {
        setMessage(QStringLiteral("The selected user is already active."), true);
        return;
    }

    const int answer = QMessageBox::question(
        this,
        QStringLiteral("Enable User"),
        QStringLiteral("Enable user \"%1\"?").arg(targetUser.username),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    const bool success = userRepository.updateStatusByUserId(
        targetUser.userId,
        QStringLiteral("active")
        );

    writeUserManagementLog(
        QStringLiteral("enable_user"),
        targetUser,
        QStringLiteral("Enable user"),
        success
        );

    if (!success) {
        setMessage(QStringLiteral("Failed to enable user. Please check the database."), true);

        QMessageBox::warning(
            this,
            QStringLiteral("Enable User Failed"),
            QStringLiteral("Failed to enable user.")
            );

        return;
    }

    setMessage(QStringLiteral("User enabled successfully."));
    refreshUserTable();
}

void UserManagementPage::onDisableUserClicked()
{
    if (!canManageUsers()) {
        setMessage(QStringLiteral("You do not have permission to disable users."), true);
        return;
    }

    const User targetUser = selectedUserFromTable();

    if (!targetUser.isValid()) {
        setMessage(QStringLiteral("Please select a user first."), true);
        return;
    }

    if (!targetUser.isActive()) {
        setMessage(QStringLiteral("The selected user is already disabled."), true);
        return;
    }

    // 禁止禁用当前登录用户，避免管理员把自己踢出系统。
    if (targetUser.userId == currentUser.userId) {
        setMessage(QStringLiteral("You cannot disable the currently logged-in user."), true);

        QMessageBox::warning(
            this,
            QStringLiteral("Operation Not Allowed"),
            QStringLiteral("You cannot disable the currently logged-in user.")
            );

        return;
    }

    // 系统至少保留一个 active admin。
    if (targetUser.isAdmin() && userRepository.countActiveAdmins() <= 1) {
        setMessage(QStringLiteral("You cannot disable the last active admin."), true);

        QMessageBox::warning(
            this,
            QStringLiteral("Operation Not Allowed"),
            QStringLiteral("You cannot disable the last active admin.")
            );

        return;
    }

    const int answer = QMessageBox::question(
        this,
        QStringLiteral("Disable User"),
        QStringLiteral("Disable user \"%1\"?").arg(targetUser.username),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    const bool success = userRepository.updateStatusByUserId(
        targetUser.userId,
        QStringLiteral("disabled")
        );

    writeUserManagementLog(
        QStringLiteral("disable_user"),
        targetUser,
        QStringLiteral("Disable user"),
        success
        );

    if (!success) {
        setMessage(QStringLiteral("Failed to disable user. Please check the database."), true);

        QMessageBox::warning(
            this,
            QStringLiteral("Disable User Failed"),
            QStringLiteral("Failed to disable user.")
            );

        return;
    }

    setMessage(QStringLiteral("User disabled successfully."));
    refreshUserTable();
}

void UserManagementPage::onResetPasswordClicked()
{
    if (!canManageUsers()) {
        setMessage(QStringLiteral("You do not have permission to reset passwords."), true);
        return;
    }

    const User targetUser = selectedUserFromTable();

    if (!targetUser.isValid()) {
        setMessage(QStringLiteral("Please select a user first."), true);
        return;
    }

    const int answer = QMessageBox::question(
        this,
        QStringLiteral("Reset Password"),
        QStringLiteral("Reset password for user \"%1\" to 123456?").arg(targetUser.username),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    // 重置密码仍然写入 SHA-256 哈希，不保存明文。
    const QString defaultPasswordHash = SHA256Util::hashPassword(QStringLiteral("123456"));

    const bool success = userRepository.updatePasswordByUserId(
        targetUser.userId,
        defaultPasswordHash
        );

    writeUserManagementLog(
        QStringLiteral("reset_password"),
        targetUser,
        QStringLiteral("Reset password to default value"),
        success
        );

    if (!success) {
        setMessage(QStringLiteral("Failed to reset password. Please check the database."), true);

        QMessageBox::warning(
            this,
            QStringLiteral("Reset Password Failed"),
            QStringLiteral("Failed to reset password.")
            );

        return;
    }

    setMessage(QStringLiteral("Password reset successfully. Default password: 123456"));

    QMessageBox::information(
        this,
        QStringLiteral("Reset Password Success"),
        QStringLiteral("Password has been reset to 123456.")
        );

    refreshUserTable();
}

// 清空表单。角色默认恢复为 User，避免误创建管理员账号。
void UserManagementPage::resetForm()
{
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->roleComboBox->setCurrentIndex(0);
    ui->usernameLineEdit->setFocus();
}

// 统一控制表单可用状态。
void UserManagementPage::setFormEnabled(bool enabled)
{
    ui->usernameLineEdit->setEnabled(enabled);
    ui->passwordLineEdit->setEnabled(enabled);
    ui->roleComboBox->setEnabled(enabled);
    ui->createUserButton->setEnabled(enabled);
}

// 统一控制用户列表和操作按钮可用状态。
void UserManagementPage::setUserListEnabled(bool enabled)
{
    ui->userListCard->setEnabled(enabled);
    ui->userTable->setEnabled(enabled);
    ui->refreshUsersButton->setEnabled(enabled);
    ui->enableUserButton->setEnabled(enabled);
    ui->disableUserButton->setEnabled(enabled);
    ui->resetPasswordButton->setEnabled(enabled);
}

// 根据当前用户角色刷新页面权限。
void UserManagementPage::updateAccessState()
{
    const bool allowed = canManageUsers();

    setFormEnabled(allowed);
    setUserListEnabled(allowed);

    if (allowed) {
        ui->pageSubtitleLabel->setText(
            QStringLiteral("Create accounts, view users, disable accounts, enable accounts, and reset passwords.")
            );

        setMessage(QString());
        refreshUserTable();
        return;
    }

    ui->pageSubtitleLabel->setText(
        QStringLiteral("Only admin users can manage accounts.")
        );

    setMessage(QStringLiteral("You do not have permission to manage users."), true);

    ui->userTable->clearContents();
    ui->userTable->setRowCount(0);
}

// 刷新用户列表。userId 存入 Qt::UserRole，后续操作直接按 ID 更新。
void UserManagementPage::refreshUserTable()
{
    if (!canManageUsers()) {
        ui->userTable->clearContents();
        ui->userTable->setRowCount(0);
        return;
    }

    const QList<User> users = userRepository.findAllUsers();

    ui->userTable->setSortingEnabled(false);
    ui->userTable->clearContents();
    ui->userTable->setRowCount(users.size());

    for (int row = 0; row < users.size(); ++row) {
        const User& user = users.at(row);

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(user.userId));
        idItem->setData(Qt::UserRole, user.userId);

        QTableWidgetItem *usernameItem = new QTableWidgetItem(user.username);
        usernameItem->setData(Qt::UserRole, user.userId);

        QTableWidgetItem *roleItem = new QTableWidgetItem(user.role);
        roleItem->setData(Qt::UserRole, user.userId);

        QTableWidgetItem *statusItem = new QTableWidgetItem(user.status);
        statusItem->setData(Qt::UserRole, user.userId);

        ui->userTable->setItem(row, 0, idItem);
        ui->userTable->setItem(row, 1, usernameItem);
        ui->userTable->setItem(row, 2, roleItem);
        ui->userTable->setItem(row, 3, statusItem);
    }

    ui->userTable->setSortingEnabled(true);
    ui->userTable->resizeRowsToContents();
}

// 当前用户必须是 active admin 才能管理用户。
bool UserManagementPage::canManageUsers() const
{
    return currentUser.isValid()
    && currentUser.isActive()
        && currentUser.isAdmin();
}

// 从下拉框读取真实角色值。
QString UserManagementPage::selectedRole() const
{
    return ui->roleComboBox->currentData().toString();
}

// 从当前选中行构造 User，只读取操作需要的字段。
User UserManagementPage::selectedUserFromTable() const
{
    User user;

    const int row = ui->userTable->currentRow();

    if (row < 0) {
        return user;
    }

    QTableWidgetItem *idItem = ui->userTable->item(row, 0);
    QTableWidgetItem *usernameItem = ui->userTable->item(row, 1);
    QTableWidgetItem *roleItem = ui->userTable->item(row, 2);
    QTableWidgetItem *statusItem = ui->userTable->item(row, 3);

    if (!idItem || !usernameItem || !roleItem || !statusItem) {
        return user;
    }

    user.userId = idItem->data(Qt::UserRole).toInt();
    user.username = usernameItem->text();
    user.role = roleItem->text();
    user.status = statusItem->text();

    return user;
}

// 页面提示统一从这里设置。
void UserManagementPage::setMessage(const QString& message,
                                    bool error)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}

// 用户管理操作写入日志，方便管理员在 Operation Logs 页面审计。
void UserManagementPage::writeUserManagementLog(const QString& action,
                                                const User& targetUser,
                                                const QString& detail,
                                                bool success)
{
    const QString operatorUsername = currentUser.username.trimmed().isEmpty()
    ? QStringLiteral("unknown")
    : currentUser.username.trimmed();

    const QString logDetail = QStringLiteral("%1. Target user_id: %2, username: %3, role: %4, status: %5")
                                  .arg(detail)
                                  .arg(targetUser.userId)
                                  .arg(targetUser.username)
                                  .arg(targetUser.role)
                                  .arg(targetUser.status);

    logService.writeLog(
        currentUser.userId,
        operatorUsername,
        action,
        logDetail,
        success ? QStringLiteral("success") : QStringLiteral("failed")
        );
}
