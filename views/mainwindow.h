#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../models/user.h"

#include <QMainWindow>

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

class QPushButton;
class QWidget;

/*
 * 登录后的主界面。
 * 负责页面创建、页面切换、权限控制和导航栏状态刷新。
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

    QWidget* createPlaceholderPage(const QString& title,
                                   const QString& description);

    // 给页面套滚动区域，避免默认窗口下内容被压缩错版。
    QWidget* createScrollablePage(QWidget *page);

    // pageStack 中保存的是滚动容器，这里负责根据真实页面找到容器。
    QWidget* pageContainer(QWidget *page) const;

    void navigateTo(QWidget *page,
                    const QString& title,
                    QPushButton *activeButton);

    void setActiveNavButton(QPushButton *activeButton);
    void updateRoleAccess();
    bool isAdminUser() const;

private:
    Ui::MainWindow *ui;

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
