#include "usermanagementpage.h"
#include "ui_usermanagementpage.h"

#include "../services/appstyle.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

UserManagementPage::UserManagementPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::UserManagementPage)
{
    /*
     * setupUi() 会读取 forms/usermanagementpage.ui，
     * 并创建标题、说明文字、表单卡片、输入框、下拉框、按钮和提示 Label。
     *
     * 注意：
     * 这里不再手写 new QLabel / new QLineEdit / new QGridLayout。
     * 固定 UI 结构已经交给 .ui 文件。
     */
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
 *
 * .ui 文件只负责控件结构；
 * 这里负责：
 * - 设置 QSS 使用的 objectName；
 * - 设置输入框提示；
 * - 初始化角色下拉框；
 * - 设置密码输入模式；
 * - 设置按钮鼠标样式。
 */
void UserManagementPage::prepareUiObjects()
{
    setObjectName(QStringLiteral("userManagementPage"));

    // 这些 objectName 对应 AppStyle::dataManagementPageStyle() 中的 QSS 选择器。
    ui->pageTitleLabel->setObjectName(QStringLiteral("pageTitle"));
    ui->pageSubtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));
    ui->formCard->setObjectName(QStringLiteral("card"));
    ui->usernameLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->passwordLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->roleLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->messageLabel->setObjectName(QStringLiteral("messageLabel"));

    ui->pageSubtitleLabel->setWordWrap(true);
    ui->messageLabel->setWordWrap(true);

    // 第二列自动占满剩余宽度，让输入框宽度跟随卡片变化。
    ui->formLayout->setColumnStretch(1, 1);

    ui->usernameLineEdit->setPlaceholderText(QStringLiteral("Enter username"));

    ui->passwordLineEdit->setPlaceholderText(QStringLiteral("At least 6 characters"));
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    /*
     * 下拉框显示文本和真实业务值分开：
     * - 用户看到 User / Admin；
     * - 业务逻辑读取 user / admin。
     */
    ui->roleComboBox->clear();
    ui->roleComboBox->addItem(QStringLiteral("User"), QStringLiteral("user"));
    ui->roleComboBox->addItem(QStringLiteral("Admin"), QStringLiteral("admin"));

    ui->createUserButton->setObjectName(QStringLiteral("primaryButton"));
    ui->createUserButton->setCursor(Qt::PointingHandCursor);
}

// 集中连接信号槽，方便后续维护页面行为。
void UserManagementPage::connectSignals()
{
    connect(ui->createUserButton, &QPushButton::clicked,
            this, &UserManagementPage::onCreateUserClicked);
}

// 用户管理页复用数据管理页面样式。
// 这里不再写大段 QSS，避免和 PostManagementPage / LogPage 重复。
void UserManagementPage::applyStyleSheet()
{
    setStyleSheet(AppStyle::dataManagementPageStyle());
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
        ui->usernameLineEdit->text(),
        ui->passwordLineEdit->text(),
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
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->roleComboBox->setCurrentIndex(0);
    ui->usernameLineEdit->setFocus();
}

// 统一控制表单可用状态，避免每个控件分散写权限逻辑。
void UserManagementPage::setFormEnabled(bool enabled)
{
    ui->usernameLineEdit->setEnabled(enabled);
    ui->passwordLineEdit->setEnabled(enabled);
    ui->roleComboBox->setEnabled(enabled);
    ui->createUserButton->setEnabled(enabled);
}

// 根据当前用户角色刷新页面提示和表单状态。
void UserManagementPage::updateAccessState()
{
    const bool allowed = canManageUsers();

    setFormEnabled(allowed);

    if (allowed) {
        ui->pageSubtitleLabel->setText(
            QStringLiteral("Create accounts for people who need access to this dashboard.")
            );

        setMessage(QString());
        return;
    }

    ui->pageSubtitleLabel->setText(
        QStringLiteral("Only admin users can manage accounts.")
        );

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
    return ui->roleComboBox->currentData().toString();
}

// 设置页面内提示信息，避免多处直接操作 messageLabel。
// error=true 时复用 AppStyle 的错误提示颜色。
void UserManagementPage::setMessage(const QString& message,
                                    bool error)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}
