#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../models/user.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class DashboardPage;
class LogPage;
class PostManagementPage;
class UserManagementPage;

class QLabel;
class QPushButton;
class QStackedWidget;
class QWidget;

/*
 * 登录后的主界面。
 *
 * 负责：
 * - 创建主窗口布局；
 * - 管理左侧导航；
 * - 管理页面切换；
 * - 根据当前用户角色控制功能入口。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setCurrentUser(const User& user);

private slots:
    void showDashboardPage();
    void showPostManagementPage();
    void showAnalyticsPage();
    void showUserManagementPage();
    void showLogPage();
    void showSettingsPage();
    void exitApplication();

private:
    void buildUi();
    void applyStyleSheet();

    QWidget* createSideBar();
    QWidget* createTopBar();

    QWidget* createPlaceholderPage(const QString& title,
                                   const QString& description);

    QPushButton* createNavButton(const QString& text);

    void navigateTo(QWidget *page,
                    const QString& title,
                    QPushButton *activeButton);

    void setActiveNavButton(QPushButton *activeButton);
    void updateRoleAccess();
    bool isAdminUser() const;

private:
    Ui::MainWindow *ui;

    User currentUser;

    QLabel *pageTitleLabel;
    QLabel *userInfoLabel;

    QPushButton *dashboardButton;
    QPushButton *postManagementButton;
    QPushButton *analyticsButton;
    QPushButton *userManagementButton;
    QPushButton *operationLogsButton;
    QPushButton *settingsButton;
    QPushButton *logoutButton;

    QStackedWidget *pageStack;

    DashboardPage *dashboardPage;
    PostManagementPage *postManagementPage;
    QWidget *analyticsPage;
    UserManagementPage *userManagementPage;
    LogPage *logPage;
    QWidget *settingsPage;
};

#endif // MAINWINDOW_H
