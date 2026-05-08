#include "usermanagementpage.h"

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

void UserManagementPage::setCurrentUser(const User& user)
{
    currentUser = user;
    updateAccessState();
}

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

    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));
    card->setMaximumWidth(560);

    auto *formLayout = new QGridLayout(card);
    formLayout->setContentsMargins(22, 22, 22, 22);
    formLayout->setHorizontalSpacing(14);
    formLayout->setVerticalSpacing(14);

    auto *usernameLabel = new QLabel(QStringLiteral("Username"));
    auto *passwordLabel = new QLabel(QStringLiteral("Password"));
    auto *roleLabel = new QLabel(QStringLiteral("Role"));

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

    messageLabel = new QLabel();
    messageLabel->setWordWrap(true);
    messageLabel->setObjectName(QStringLiteral("messageLabel"));

    formLayout->addWidget(usernameLabel, 0, 0);
    formLayout->addWidget(usernameLineEdit, 0, 1);

    formLayout->addWidget(passwordLabel, 1, 0);
    formLayout->addWidget(passwordLineEdit, 1, 1);

    formLayout->addWidget(roleLabel, 2, 0);
    formLayout->addWidget(roleComboBox, 2, 1);

    formLayout->addWidget(createUserButton, 3, 1);
    formLayout->addWidget(messageLabel, 4, 1);

    rootLayout->addWidget(card);
    rootLayout->addStretch();

    connect(createUserButton, &QPushButton::clicked,
            this, &UserManagementPage::onCreateUserClicked);

    setStyleSheet(
        "QLabel#pageTitle {"
        "    font-size: 24px;"
        "    font-weight: 700;"
        "    color: #111827;"
        "}"
        "QLabel#pageSubtitle {"
        "    font-size: 13px;"
        "    color: #6B7280;"
        "}"
        "QFrame#card {"
        "    background: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 12px;"
        "}"
        "QLineEdit, QComboBox {"
        "    min-height: 34px;"
        "    border: 1px solid #D1D5DB;"
        "    border-radius: 8px;"
        "    padding-left: 10px;"
        "    background: #FFFFFF;"
        "}"
        "QLineEdit:focus, QComboBox:focus {"
        "    border: 1px solid #2563EB;"
        "}"
        "QPushButton#primaryButton {"
        "    min-height: 36px;"
        "    background: #2563EB;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 0 18px;"
        "    font-weight: 600;"
        "}"
        "QPushButton#primaryButton:hover {"
        "    background: #1D4ED8;"
        "}"
        "QPushButton#primaryButton:disabled {"
        "    background: #9CA3AF;"
        "}"
        "QLabel#messageLabel {"
        "    color: #374151;"
        "}"
        );
}

void UserManagementPage::onCreateUserClicked()
{
    QString username = usernameLineEdit->text();
    QString password = passwordLineEdit->text();
    QString role = roleComboBox->currentData().toString();

    QString message;

    bool success = authController.registerUser(
        currentUser,
        username,
        password,
        role,
        message
        );

    messageLabel->setText(message);

    if (!success) {
        QMessageBox::warning(this, QStringLiteral("Create User Failed"), message);
        return;
    }

    QMessageBox::information(this, QStringLiteral("Create User Success"), message);
    clearForm();
}

void UserManagementPage::clearForm()
{
    usernameLineEdit->clear();
    passwordLineEdit->clear();
    roleComboBox->setCurrentIndex(0);
    usernameLineEdit->setFocus();
}

void UserManagementPage::updateAccessState()
{
    const bool isAdmin = (currentUser.role == QStringLiteral("admin"));

    usernameLineEdit->setEnabled(isAdmin);
    passwordLineEdit->setEnabled(isAdmin);
    roleComboBox->setEnabled(isAdmin);
    createUserButton->setEnabled(isAdmin);

    if (isAdmin) {
        tipLabel->setText(QStringLiteral("Create accounts for people who need access to this dashboard."));
        messageLabel->clear();
    } else {
        tipLabel->setText(QStringLiteral("Only admin users can manage accounts."));
        messageLabel->setText(QStringLiteral("You do not have permission to create users."));
    }
}
