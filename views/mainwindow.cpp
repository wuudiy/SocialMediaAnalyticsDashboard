#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../services/appstyle.h"

#include "analyticspage.h"
#include "dashboardpage.h"
#include "exportpage.h"
#include "logpage.h"
#include "postmanagementpage.h"
#include "settingspage.h"
#include "usermanagementpage.h"

#include <QApplication>
#include <QFrame>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    dashboardPage(nullptr),
    postManagementPage(nullptr),
    analyticsPage(nullptr),
    exportPage(nullptr),
    userManagementPage(nullptr),
    logPage(nullptr),
    settingsPage(nullptr)
{
    /*
     * setupUi() 会读取 forms/mainwindow.ui，
     * 并创建左侧导航栏、顶部栏、pageStack 等固定界面结构。
     */
    ui->setupUi(this);

    prepareUiObjects();
    setupPages();
    connectSignals();
    applyStyleSheet();

    showDashboardPage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * 初始化 ui 中已有控件的运行时属性。
 *
 * 注意：
 * Qt Designer 里的对象名必须保持唯一，例如 dashboardButton、postManagementButton。
 * 但是 AppStyle 里导航按钮样式使用的是 navButton / activeNavButton。
 *
 * 所以这里在 setupUi() 之后，统一把这些按钮的 objectName 改成 navButton。
 * 这样既能保留 ui->dashboardButton 这种访问方式，又能复用统一 QSS。
 */
void MainWindow::prepareUiObjects()
{
    const QList<QPushButton*> navButtons = {
        ui->dashboardButton,
        ui->postManagementButton,
        ui->analyticsButton,
        ui->exportButton,
        ui->userManagementButton,
        ui->operationLogsButton,
        ui->settingsButton
    };

    for (QPushButton *button : navButtons) {
        if (!button) {
            continue;
        }

        button->setObjectName(QStringLiteral("navButton"));
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(42);
    }

    ui->logoutButton->setObjectName(QStringLiteral("logoutButton"));
    ui->logoutButton->setCursor(Qt::PointingHandCursor);

    ui->pageTitleLabel->setText(QStringLiteral("Dashboard"));
    ui->userInfoLabel->setText(QStringLiteral("Not logged in"));
}

/*
 * 创建真实页面对象，并加入 ui 文件中的 pageStack。
 *
 * ui 文件只负责提供 QStackedWidget 容器，
 * 具体页面仍然由 C++ 创建，方便继续使用原来的 DashboardPage、
 * PostManagementPage、UserManagementPage 和 LogPage 逻辑。
 */
void MainWindow::setupPages()
{
    dashboardPage = new DashboardPage(ui->pageStack);
    postManagementPage = new PostManagementPage(ui->pageStack);

    analyticsPage = new AnalyticsPage(ui->pageStack);
    exportPage = new ExportPage(ui->pageStack);

    userManagementPage = new UserManagementPage(ui->pageStack);
    logPage = new LogPage(ui->pageStack);

    settingsPage = new SettingsPage(ui->pageStack);

    ui->pageStack->addWidget(dashboardPage);
    ui->pageStack->addWidget(postManagementPage);
    ui->pageStack->addWidget(analyticsPage);
    ui->pageStack->addWidget(exportPage);
    ui->pageStack->addWidget(userManagementPage);
    ui->pageStack->addWidget(logPage);
    ui->pageStack->addWidget(settingsPage);
}

/*
 * 集中连接主窗口按钮的信号槽。
 *
 * 这样 ui 文件只负责“有哪些按钮”，
 * cpp 文件负责“按钮点击后做什么”。
 */
void MainWindow::connectSignals()
{
    connect(ui->dashboardButton, &QPushButton::clicked,
            this, &MainWindow::showDashboardPage);

    connect(ui->postManagementButton, &QPushButton::clicked,
            this, &MainWindow::showPostManagementPage);

    connect(ui->analyticsButton, &QPushButton::clicked,
            this, &MainWindow::showAnalyticsPage);

    connect(ui->exportButton, &QPushButton::clicked,
            this, &MainWindow::showExportPage);

    connect(ui->userManagementButton, &QPushButton::clicked,
            this, &MainWindow::showUserManagementPage);

    connect(ui->operationLogsButton, &QPushButton::clicked,
            this, &MainWindow::showLogPage);

    connect(ui->settingsButton, &QPushButton::clicked,
            this, &MainWindow::showSettingsPage);

    connect(ui->logoutButton, &QPushButton::clicked,
            this, &MainWindow::exitApplication);
}

/*
 * 主窗口样式统一交给 AppStyle。
 *
 * MainWindow 只保留页面逻辑，不再保存大段 QSS 字符串。
 */
void MainWindow::applyStyleSheet()
{
    setStyleSheet(AppStyle::mainWindowStyle());
}

/*
 * 设置当前登录用户，并把用户信息同步给主界面和子页面。
 */
void MainWindow::setCurrentUser(const User& user)
{
    currentUser = user;

    setWindowTitle(
        QStringLiteral("Social Media Analytics Dashboard - %1")
            .arg(currentUser.username)
        );

    ui->userInfoLabel->setText(
        QStringLiteral("%1  |  %2")
            .arg(currentUser.username, currentUser.role.toUpper())
        );

    dashboardPage->setCurrentUser(currentUser);
    postManagementPage->setCurrentUser(currentUser);
    exportPage->setCurrentUser(currentUser);
    userManagementPage->setCurrentUser(currentUser);

    updateRoleAccess();
    showDashboardPage();

    statusBar()->showMessage(
        QStringLiteral("Logged in as %1").arg(currentUser.role),
        3000
        );
}

/*
 * 创建占位页。
 *
 * 当前 Analytics 和 Settings 还没有做成真实页面，
 * 所以先保留占位卡片，后续可以继续替换为独立 Page 类。
 */
QWidget* MainWindow::createPlaceholderPage(const QString& title,
                                           const QString& description)
{
    auto *page = new QWidget();

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 22, 24, 24);

    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("placeholderCard"));
    card->setMinimumHeight(220);

    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setSpacing(12);

    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName(QStringLiteral("placeholderTitle"));

    auto *textLabel = new QLabel(description);
    textLabel->setObjectName(QStringLiteral("placeholderText"));
    textLabel->setWordWrap(true);

    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(textLabel);
    cardLayout->addStretch();

    layout->addWidget(card);
    layout->addStretch();

    return page;
}

/*
 * 切换到 Dashboard 页面。
 */
void MainWindow::showDashboardPage()
{
    dashboardPage->refreshDashboard();

    navigateTo(
        dashboardPage,
        QStringLiteral("Dashboard"),
        ui->dashboardButton
        );
}

/*
 * 切换到帖子数据管理页面。
 */
void MainWindow::showPostManagementPage()
{
    postManagementPage->refreshPosts();

    navigateTo(
        postManagementPage,
        QStringLiteral("Post Data"),
        ui->postManagementButton
        );
}

/*
 * 切换到 Analytics 页面。
 */
void MainWindow::showAnalyticsPage()
{
    analyticsPage->refreshData();

    navigateTo(
        analyticsPage,
        QStringLiteral("Analytics"),
        ui->analyticsButton
        );
}

/*
 * 切换到报表导出页面。
 *
 * 导出功能对所有用户开放，
 * 允许用户将统计数据导出为 CSV 或 TXT 格式。
 */
void MainWindow::showExportPage()
{
    navigateTo(
        exportPage,
        QStringLiteral("Export Reports"),
        ui->exportButton
        );
}

/*
 * 切换到用户管理页面。
 *
 * 用户管理属于管理员功能，
 * 所以进入页面前先做权限检查。
 */
void MainWindow::showUserManagementPage()
{
    if (!isAdminUser()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Access Denied"),
            QStringLiteral("Only admin users can open User Management.")
            );

        return;
    }

    navigateTo(
        userManagementPage,
        QStringLiteral("User Management"),
        ui->userManagementButton
        );
}

/*
 * 切换到系统日志页面。
 *
 * 日志页面同样只允许管理员查看。
 */
void MainWindow::showLogPage()
{
    if (!isAdminUser()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Access Denied"),
            QStringLiteral("Only admin users can open Operation Logs.")
            );

        return;
    }

    logPage->refreshLogs();

    navigateTo(
        logPage,
        QStringLiteral("Operation Logs"),
        ui->operationLogsButton
        );
}

/*
 * 切换到 Settings 页面。
 */
void MainWindow::showSettingsPage()
{
    navigateTo(
        settingsPage,
        QStringLiteral("Settings"),
        ui->settingsButton
        );
}

/*
 * 统一处理页面跳转。
 *
 * 这里集中完成三件事：
 * 1. 切换 pageStack 当前页面；
 * 2. 修改顶部标题；
 * 3. 修改左侧导航按钮高亮状态。
 */
void MainWindow::navigateTo(QWidget *page,
                            const QString& title,
                            QPushButton *activeButton)
{
    if (!page || !ui->pageStack) {
        return;
    }

    ui->pageStack->setCurrentWidget(page);
    ui->pageTitleLabel->setText(title);
    setActiveNavButton(activeButton);
}

/*
 * 退出程序前二次确认，避免用户误点。
 */
void MainWindow::exitApplication()
{
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        QStringLiteral("Logout"),
        QStringLiteral("Do you want to exit the application?")
        );

    if (answer == QMessageBox::Yes) {
        qApp->quit();
    }
}

/*
 * 刷新左侧导航高亮状态。
 *
 * 说明：
 * 这里通过切换 objectName 来触发不同 QSS：
 * - navButton：普通状态
 * - activeNavButton：当前选中状态
 *
 * 修改 objectName 后需要 unpolish / polish，
 * 否则 Qt 可能不会立刻重新应用样式。
 */
void MainWindow::setActiveNavButton(QPushButton *activeButton)
{
    const QList<QPushButton*> buttons = {
        ui->dashboardButton,
        ui->postManagementButton,
        ui->analyticsButton,
        ui->userManagementButton,
        ui->operationLogsButton,
        ui->settingsButton
    };

    for (QPushButton *button : buttons) {
        if (!button) {
            continue;
        }

        const bool isActive = (button == activeButton);

        button->setObjectName(
            isActive
                ? QStringLiteral("activeNavButton")
                : QStringLiteral("navButton")
            );

        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }
}

/*
 * 根据当前用户角色刷新主界面入口。
 *
 * 普通用户不能看到：
 * - User Management
 * - Operation Logs
 */
void MainWindow::updateRoleAccess()
{
    const bool canUseAdminPages = isAdminUser();

    ui->userManagementButton->setVisible(canUseAdminPages);
    ui->operationLogsButton->setVisible(canUseAdminPages);

    /*
     * 防止普通用户因为代码跳转或角色变化停留在管理员页面。
     */
    if (!canUseAdminPages
        && (ui->pageStack->currentWidget() == userManagementPage
            || ui->pageStack->currentWidget() == logPage)) {
        showDashboardPage();
    }
}

/*
 * 管理员判断统一收口，避免到处直接比较 role 字符串。
 */
bool MainWindow::isAdminUser() const
{
    return currentUser.isValid() && currentUser.isAdmin();
}
