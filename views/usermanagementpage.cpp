#include "usermanagementpage.h"
#include "ui_usermanagementpage.h"

#include "../controllers/usercontroller.h"
#include "../styles/appstyle.h"

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
    ui(new Ui::UserManagementPage),
    userController(nullptr)
{
    ui->setupUi(this);

    prepareUiObjects();
    connectSignals();
    applyStyleSheet();

    /*
     * 当前批次为了减少 MainWindow 改动，Controller 先由页面内部创建。
     * 后续如果继续做依赖注入，可以把 Controller 改成由 MainWindowController 创建。
     */
    userController = new UserController(this, this);

    showAccessState(false);
}

UserManagementPage::~UserManagementPage()
{
    delete ui;
}

// 接收当前登录用户，并交给 Controller 判断权限和刷新用户列表。
void UserManagementPage::setCurrentUser(const User& user)
{
    if (userController) {
        userController->setCurrentUser(user);
    }
}

/*
 * 初始化 .ui 中已有控件的运行时属性。
 *
 * 固定结构放在 .ui 文件中，
 * 这里只设置样式名、输入提示和表格行为。
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

    /*
     * 下拉框显示文本和真实业务值分开，
     * 避免用界面文本参与逻辑判断。
     */
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

void UserManagementPage::showAccessState(bool allowed)
{
    setFormEnabled(allowed);
    setUserListEnabled(allowed);

    if (allowed) {
        ui->pageSubtitleLabel->setText(
            QStringLiteral("Create accounts, view users, disable accounts, enable accounts, and reset passwords.")
            );
        return;
    }

    ui->pageSubtitleLabel->setText(
        QStringLiteral("Only admin users can manage accounts.")
        );

    ui->userTable->clearContents();
    ui->userTable->setRowCount(0);
}

void UserManagementPage::showUsers(const QList<User>& users)
{
    fillUserTable(users);
}

void UserManagementPage::fillUserTable(const QList<User>& users)
{
    ui->userTable->setSortingEnabled(false);
    ui->userTable->clearContents();
    ui->userTable->setRowCount(users.size());

    for (int row = 0; row < users.size(); ++row) {
        const User& user = users.at(row);

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(user.userId));
        idItem->setData(Qt::UserRole, user.userId);
        idItem->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *usernameItem = new QTableWidgetItem(user.username);
        usernameItem->setData(Qt::UserRole, user.userId);

        QTableWidgetItem *roleItem = new QTableWidgetItem(user.role);
        roleItem->setData(Qt::UserRole, user.userId);
        roleItem->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *statusItem = new QTableWidgetItem(user.status);
        statusItem->setData(Qt::UserRole, user.userId);
        statusItem->setTextAlignment(Qt::AlignCenter);

        ui->userTable->setItem(row, 0, idItem);
        ui->userTable->setItem(row, 1, usernameItem);
        ui->userTable->setItem(row, 2, roleItem);
        ui->userTable->setItem(row, 3, statusItem);
    }

    ui->userTable->setSortingEnabled(true);
    ui->userTable->resizeRowsToContents();
}

void UserManagementPage::onCreateUserClicked()
{
    /*
     * View 只收集输入，然后发给 Controller。
     *
     * 权限判断、输入校验、密码哈希、数据库写入、日志记录
     * 都不再写在 View 中。
     */
    emit createUserRequested(
        ui->usernameLineEdit->text(),
        ui->passwordLineEdit->text(),
        selectedRole()
        );
}

void UserManagementPage::onRefreshUsersClicked()
{
    emit refreshUsersRequested();
}

void UserManagementPage::onEnableUserClicked()
{
    const User targetUser = selectedUserFromTable();

    if (!targetUser.isValid()) {
        showMessage(QStringLiteral("Please select a user first."), true);
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

    emit enableUserRequested(targetUser);
}

void UserManagementPage::onDisableUserClicked()
{
    const User targetUser = selectedUserFromTable();

    if (!targetUser.isValid()) {
        showMessage(QStringLiteral("Please select a user first."), true);
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

    emit disableUserRequested(targetUser);
}

void UserManagementPage::onResetPasswordClicked()
{
    const User targetUser = selectedUserFromTable();

    if (!targetUser.isValid()) {
        showMessage(QStringLiteral("Please select a user first."), true);
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

    emit resetPasswordRequested(targetUser);
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
void UserManagementPage::showMessage(const QString& message,
                                     bool error)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}

void UserManagementPage::showWarningMessage(const QString& title,
                                            const QString& message)
{
    QMessageBox::warning(this, title, message);
}

void UserManagementPage::handleCreateUserSuccess(const QString& message)
{
    showMessage(message);

    QMessageBox::information(
        this,
        QStringLiteral("Create User Success"),
        message
        );

    resetForm();
}

void UserManagementPage::handleEnableUserSuccess(const QString& message)
{
    showMessage(message);
}

void UserManagementPage::handleDisableUserSuccess(const QString& message)
{
    showMessage(message);
}

void UserManagementPage::handleResetPasswordSuccess(const QString& message,
                                                    const QString& defaultPassword)
{
    showMessage(message);

    QMessageBox::information(
        this,
        QStringLiteral("Reset Password Success"),
        QStringLiteral("Password has been reset to %1.")
            .arg(defaultPassword)
        );
}
