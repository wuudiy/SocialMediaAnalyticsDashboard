#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../models/User.h"
#include "../controllers/authcontroller.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class DashboardPage;
class UserManagementPage;

class QLabel;
class QPushButton;
class QStackedWidget;
class QWidget;
/*
 * View 层：MainWindow
 * -------------------
 * 作用：
 * 1. 登录成功后的系统主窗口；
 * 2. 接收当前登录用户；
 * 3. 根据用户角色决定显示管理员功能还是普通用户功能。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 登录成功后，由 main.cpp 调用，把当前用户传入主窗口
    void setCurrentUser(const User& user);

private slots:
    void showDashboardPage();
    void showAnalyticsPage();
    void showUserManagementPage();
    void showSettingsPage();
    void exitApplication();

private:
    void buildUi();
    QWidget* createSideBar();
    QWidget* createTopBar();
    QWidget* createPlaceholderPage(const QString& title,
                                   const QString& description);

    QPushButton* createNavButton(const QString& text);
    void setActiveNavButton(QPushButton *activeButton);
    void updateRoleAccess();

private:
    Ui::MainWindow *ui;

    User currentUser;

    QLabel *pageTitleLabel;
    QLabel *userInfoLabel;

    QPushButton *dashboardButton;
    QPushButton *analyticsButton;
    QPushButton *userManagementButton;
    QPushButton *settingsButton;
    QPushButton *logoutButton;

    QStackedWidget *pageStack;

    DashboardPage *dashboardPage;
    QWidget *analyticsPage;
    UserManagementPage *userManagementPage;
    QWidget *settingsPage;
};

#endif
 // MAINWINDOW_H
