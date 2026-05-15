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
 *
 * 不负责：
 * - 登录注册逻辑；
 * - 数据库读写；
 * - 各页面自己的业务处理。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 登录成功后设置当前用户，并刷新界面权限。
    void setCurrentUser(const User& user);

private slots:
    void showDashboardPage();
    void showPostManagementPage();
    void showAnalyticsPage();
    void showUserManagementPage();
    void showSettingsPage();
    void exitApplication();

private:
    // 主界面初始化。
    void buildUi();

    // 当前窗口的统一样式。
    void applyStyleSheet();

    // 左侧导航栏。
    QWidget* createSideBar();

    // 顶部标题栏。
    QWidget* createTopBar();

    // 未完成模块的占位页面。
    QWidget* createPlaceholderPage(const QString& title,
                                   const QString& description);

    // 创建统一风格的导航按钮。
    QPushButton* createNavButton(const QString& text);

    // 统一页面跳转：切页面、改标题、刷新导航高亮。
    void navigateTo(QWidget *page,
                    const QString& title,
                    QPushButton *activeButton);

    // 设置当前选中的导航按钮。
    void setActiveNavButton(QPushButton *activeButton);

    // 根据角色刷新可见功能入口。
    void updateRoleAccess();

    // 当前用户是否为管理员。
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
    QPushButton *settingsButton;
    QPushButton *logoutButton;

    QStackedWidget *pageStack;

    DashboardPage *dashboardPage;
    PostManagementPage *postManagementPage;
    QWidget *analyticsPage;
    UserManagementPage *userManagementPage;
    QWidget *settingsPage;
};

#endif // MAINWINDOW_H
