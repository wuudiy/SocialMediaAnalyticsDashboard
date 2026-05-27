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
#include <QScrollArea>
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
    settingsPage(nullptr),
    dashboardContainer(nullptr),
    postManagementContainer(nullptr),
    analyticsContainer(nullptr),
    exportContainer(nullptr),
    userManagementContainer(nullptr),
    logContainer(nullptr),
    settingsContainer(nullptr)
{
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
 * 创建页面对象，并统一放入滚动容器。
 */
void MainWindow::setupPages()
{
    dashboardPage = new DashboardPage();
    postManagementPage = new PostManagementPage();
    analyticsPage = new AnalyticsPage();
    exportPage = new ExportPage();
    userManagementPage = new UserManagementPage();
    logPage = new LogPage();
    settingsPage = new SettingsPage();

    dashboardContainer = createScrollablePage(dashboardPage);
    postManagementContainer = createScrollablePage(postManagementPage);
    analyticsContainer = createScrollablePage(analyticsPage);
    exportContainer = createScrollablePage(exportPage);
    userManagementContainer = createScrollablePage(userManagementPage);
    logContainer = createScrollablePage(logPage);
    settingsContainer = createScrollablePage(settingsPage);

    ui->pageStack->addWidget(dashboardContainer);
    ui->pageStack->addWidget(postManagementContainer);
    ui->pageStack->addWidget(analyticsContainer);
    ui->pageStack->addWidget(exportContainer);
    ui->pageStack->addWidget(userManagementContainer);
    ui->pageStack->addWidget(logContainer);
    ui->pageStack->addWidget(settingsContainer);
}

/*
 * 集中连接主窗口按钮的信号槽。
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
 */
void MainWindow::applyStyleSheet()
{
    setStyleSheet(AppStyle::mainWindowStyle());
}

/*
 * 设置当前登录用户，并同步给需要用户信息的子页面。
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
 * 当前保留此函数，后续新增页面时仍可复用。
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
 * 给页面套一层 QScrollArea。
 * 内容超过默认窗口高度时，使用滚动条显示，避免控件互相挤压。
 */
QWidget* MainWindow::createScrollablePage(QWidget *page)
{
    auto *scrollArea = new QScrollArea(ui->pageStack);

    scrollArea->setWidget(page);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    return scrollArea;
}

/*
 * pageStack 里保存的是滚动容器，不是原始页面。
 */
QWidget* MainWindow::pageContainer(QWidget *page) const
{
    if (page == dashboardPage) {
        return dashboardContainer;
    }

    if (page == postManagementPage) {
        return postManagementContainer;
    }

    if (page == analyticsPage) {
        return analyticsContainer;
    }

    if (page == exportPage) {
        return exportContainer;
    }

    if (page == userManagementPage) {
        return userManagementContainer;
    }

    if (page == logPage) {
        return logContainer;
    }

    if (page == settingsPage) {
        return settingsContainer;
    }

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
 */
void MainWindow::navigateTo(QWidget *page,
                            const QString& title,
                            QPushButton *activeButton)
{
    if (!page || !ui->pageStack) {
        return;
    }

    QWidget *container = pageContainer(page);

    if (!container) {
        return;
    }

    ui->pageStack->setCurrentWidget(container);
    ui->pageTitleLabel->setText(title);
    setActiveNavButton(activeButton);
}

/*
 * 退出程序前二次确认，避免误点。
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
 */
void MainWindow::setActiveNavButton(QPushButton *activeButton)
{
    const QList<QPushButton*> buttons = {
        ui->dashboardButton,
        ui->postManagementButton,
        ui->analyticsButton,
        ui->exportButton,
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
 * 根据当前用户角色刷新管理员入口。
 */
void MainWindow::updateRoleAccess()
{
    const bool canUseAdminPages = isAdminUser();

    ui->userManagementButton->setVisible(canUseAdminPages);
    ui->operationLogsButton->setVisible(canUseAdminPages);

    if (!canUseAdminPages
        && (ui->pageStack->currentWidget() == userManagementContainer
            || ui->pageStack->currentWidget() == logContainer)) {
        showDashboardPage();
    }
}

/*
 * 管理员判断统一收口。
 */
bool MainWindow::isAdminUser() const
{
    return currentUser.isValid() && currentUser.isAdmin();
}
