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
 *
 * 现在采用“ui + cpp + AppStyle”分工：
 *
 * - forms/mainwindow.ui：
 *   负责主窗口固定布局，例如左侧导航、顶部栏、QStackedWidget。
 *
 * - views/mainwindow.cpp：
 *   负责页面创建、页面切换、权限控制、信号槽连接。
 *
 * - services/AppStyle：
 *   负责统一 QSS 样式。
 *
 * 这样可以减少 MainWindow.cpp 中大量 new 控件和 addWidget 代码。
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
    void showExportPage();
    void showSettingsPage();
    void exitApplication();

private:
    void prepareUiObjects();
    void setupPages();
    void connectSignals();
    void applyStyleSheet();

    QWidget* createPlaceholderPage(const QString& title,
                                   const QString& description);

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
};

#endif // MAINWINDOW_H
