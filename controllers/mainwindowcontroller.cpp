#include "mainwindowcontroller.h"

#include "../views/mainwindow.h"

#include <QCoreApplication>

MainWindowController::MainWindowController(MainWindow *view,
                                           QObject *parent)
    : QObject(parent),
    view(view)
{
    /*
     * Controller 接管 MainWindow 的全局请求。
     *
     * MainWindow 只负责：
     * - 用户点了哪个导航按钮；
     * - 当前登录用户变了；
     * - 用户点击了退出按钮。
     *
     * 是否允许跳转、跳到哪个页面、显示什么标题，
     * 都由 Controller 决定。
     */
    connect(view, &MainWindow::currentUserChanged,
            this, &MainWindowController::handleCurrentUserChanged);

    connect(view, &MainWindow::navigationRequested,
            this, &MainWindowController::handleNavigationRequested);

    connect(view, &MainWindow::logoutRequested,
            this, &MainWindowController::handleLogoutRequested);
}

void MainWindowController::handleCurrentUserChanged(const User& user)
{
    currentUser = user;

    const bool canUseAdminPages = isAdminUser();

    /*
     * 当前用户状态由 Controller 保存；
     * 具体如何显示、如何传递给子页面仍由 View 完成。
     */
    view->applyCurrentUserState(
        currentUser,
        windowTitleForUser(currentUser),
        userInfoTextForUser(currentUser),
        canUseAdminPages
        );

    view->showStatusMessage(
        QStringLiteral("Logged in as %1").arg(currentUser.role),
        3000
        );

    /*
     * 登录成功后默认进入 Dashboard。
     */
    handleNavigationRequested(MainWindowPage::Dashboard);
}

void MainWindowController::handleNavigationRequested(MainWindowPage page)
{
    if (!canOpenPage(page)) {
        view->showAccessDeniedMessage(
            QStringLiteral("Access Denied"),
            accessDeniedMessage(page)
            );

        return;
    }

    view->navigateToPage(page, pageTitle(page));
}

void MainWindowController::handleLogoutRequested()
{
    /*
     * 退出确认弹窗属于 View 交互；
     * 是否真正退出由 Controller 决定。
     */
    if (!view->confirmExitApplication()) {
        return;
    }

    QCoreApplication::quit();
}

bool MainWindowController::isAdminUser() const
{
    return currentUser.isValid()
    && currentUser.isAdmin();
}

bool MainWindowController::canOpenPage(MainWindowPage page) const
{
    if (page == MainWindowPage::UserManagement
        || page == MainWindowPage::OperationLogs) {
        return isAdminUser();
    }

    return true;
}

QString MainWindowController::pageTitle(MainWindowPage page) const
{
    switch (page) {
    case MainWindowPage::Dashboard:
        return QStringLiteral("Dashboard");

    case MainWindowPage::PostManagement:
        return QStringLiteral("Post Data");

    case MainWindowPage::Analytics:
        return QStringLiteral("Analytics");

    case MainWindowPage::Export:
        return QStringLiteral("Export Reports");

    case MainWindowPage::UserManagement:
        return QStringLiteral("User Management");

    case MainWindowPage::OperationLogs:
        return QStringLiteral("Operation Logs");

    case MainWindowPage::Settings:
        return QStringLiteral("Settings");
    }

    return QStringLiteral("Dashboard");
}

QString MainWindowController::accessDeniedMessage(MainWindowPage page) const
{
    if (page == MainWindowPage::UserManagement) {
        return QStringLiteral("Only admin users can open User Management.");
    }

    if (page == MainWindowPage::OperationLogs) {
        return QStringLiteral("Only admin users can open Operation Logs.");
    }

    return QStringLiteral("You do not have permission to open this page.");
}

QString MainWindowController::windowTitleForUser(const User& user) const
{
    if (!user.isValid()) {
        return QStringLiteral("Social Media Analytics Dashboard");
    }

    return QStringLiteral("Social Media Analytics Dashboard - %1")
        .arg(user.username);
}

QString MainWindowController::userInfoTextForUser(const User& user) const
{
    if (!user.isValid()) {
        return QStringLiteral("Not logged in");
    }

    return QStringLiteral("%1  |  %2")
        .arg(user.username,
             user.role.toUpper());
}
