#ifndef USERMANAGEMENTPAGE_H
#define USERMANAGEMENTPAGE_H

#include <QWidget>

#include "../controllers/authcontroller.h"
#include "../models/user.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;

/*
 * UserManagementPage
 * ------------------
 * 管理员页面。
 * 这一版先做新增用户，后面可以继续加用户列表、禁用账号、重置密码。
 */
class UserManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserManagementPage(QWidget *parent = nullptr);

    void setCurrentUser(const User& user);

private slots:
    void onCreateUserClicked();

private:
    void buildUi();
    void clearForm();
    void updateAccessState();

private:
    User currentUser;
    AuthController authController;

    QLabel *titleLabel;
    QLabel *tipLabel;
    QLabel *messageLabel;

    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QComboBox *roleComboBox;
    QPushButton *createUserButton;
};

#endif // USERMANAGEMENTPAGE_H
