#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../models/mainwindowmodels.h"
#include "../models/user.h"

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class AnalyticsPage;
class DashboardPage;
class ExportPage;
class LogPage;
class PostManagementPage;
class SettingsPage;
class UserManagementPage;

class MainWindowController;
class QPushButton;
class QWidget;

/*
 * 登录后的主界面。
 *
 * MVC 重构后，本类只负责 View 层：
 * - 创建子页面；
 * - 创建滚动容器；
 * - 显示指定页面；
 * - 设置导航按钮高亮；
 * - 显示用户信息；
 * - 显示权限弹窗；
 * - 发出导航、退出和用户切换信号。
 *
 * 不再负责：
 * - 判断管理员权限；
 * - 决定某页面是否能打开；
 * - 保存导航业务状态；
 * - 生成用户信息展示文本。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    /*
     * LoginView 登录成功后仍调用这个函数。
     * 函数内部只发信号，具体状态处理交给 MainWindowController。
     */
    void setCurrentUser(const User& user);

    /*
     * Controller 调用：应用当前登录用户状态。
     */
    void applyCurrentUserState(const User& user,
                               const QString& windowTitle,
                               const QString& userInfoText,
                               bool canUseAdminPages);

    /*
     * Controller 调用：显示指定页面。
     */
    void navigateToPage(MainWindowPage page,
                        const QString& title);

    /*
     * Controller 调用：显示权限不足提示。
     */
    void showAccessDeniedMessage(const QString& title,
                                 const QString& message);

    /*
     * Controller 调用：显示状态栏提示。
     */
    void showStatusMessage(const QString& message,
                           int timeoutMs = 3000);

     /*
     * Controller 调用：注销前确认。
     */
    bool confirmLogout();

    /*
    * Controller 调用：通知 main.cpp 关闭主窗口并回到登录页。
    */
    void requestReturnToLogin();

signals:
    void currentUserChanged(const User& user);

    void navigationRequested(MainWindowPage page);

    void logoutRequested();

    void returnToLoginRequested();

private slots:
    void showDashboardPage();
    void showPostManagementPage();
    void showAnalyticsPage();
    void showExportPage();
    void showUserManagementPage();
    void showLogPage();
    void showSettingsPage();
    void exitApplication();

private:
    void prepareUiObjects();
    void setupPages();
    void connectSignals();
    void applyStyleSheet();

    // 给页面套滚动区域，避免默认窗口下内容被压缩错版。
    QWidget* createScrollablePage(QWidget *page);

    // pageStack 中保存的是滚动容器，这里负责根据真实页面找到容器。
    QWidget* pageContainer(QWidget *page) const;

    void navigateTo(QWidget *page,
                    const QString& title,
                    QPushButton *activeButton);

    void setActiveNavButton(QPushButton *activeButton);

    void setAdminEntrancesVisible(bool visible);

private:
    Ui::MainWindow *ui;

    MainWindowController *mainWindowController;

    User currentUser;

    DashboardPage *dashboardPage;
    PostManagementPage *postManagementPage;
    AnalyticsPage *analyticsPage;
    ExportPage *exportPage;
    UserManagementPage *userManagementPage;
    LogPage *logPage;
    SettingsPage *settingsPage;

    QWidget *dashboardContainer;
    QWidget *postManagementContainer;
    QWidget *analyticsContainer;
    QWidget *exportContainer;
    QWidget *userManagementContainer;
    QWidget *logContainer;
    QWidget *settingsContainer;
};

#endif // MAINWINDOW_H
