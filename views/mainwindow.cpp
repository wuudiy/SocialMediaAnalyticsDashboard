#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dashboardpage.h"
#include "postmanagementpage.h"
#include "usermanagementpage.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QStyle>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    pageTitleLabel(nullptr),
    userInfoLabel(nullptr),
    dashboardButton(nullptr),
    postManagementButton(nullptr),
    analyticsButton(nullptr),
    userManagementButton(nullptr),
    settingsButton(nullptr),
    logoutButton(nullptr),
    pageStack(nullptr),
    dashboardPage(nullptr),
    postManagementPage(nullptr),
    analyticsPage(nullptr),
    userManagementPage(nullptr),
    settingsPage(nullptr)
{
    ui->setupUi(this);

    buildUi();

    resize(1180, 760);
    setMinimumSize(980, 640);

    showDashboardPage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 设置当前登录用户，并把用户信息同步给主界面和子页面。
void MainWindow::setCurrentUser(const User& user)
{
    currentUser = user;

    setWindowTitle(
        QStringLiteral("Social Media Analytics Dashboard - %1")
            .arg(currentUser.username)
        );

    userInfoLabel->setText(
        QStringLiteral("%1  |  %2")
            .arg(currentUser.username, currentUser.role.toUpper())
        );

    dashboardPage->setCurrentUser(currentUser);
    userManagementPage->setCurrentUser(currentUser);

    updateRoleAccess();
    showDashboardPage();

    statusBar()->showMessage(
        QStringLiteral("Logged in as %1").arg(currentUser.role),
        3000
        );
}

// 搭建主界面：左侧导航 + 顶部栏 + 中间页面容器。
void MainWindow::buildUi()
{
    auto *rootWidget = new QWidget(this);

    auto *rootLayout = new QHBoxLayout(rootWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    rootLayout->addWidget(createSideBar());

    auto *mainArea = new QWidget(rootWidget);
    mainArea->setObjectName(QStringLiteral("mainArea"));

    auto *mainLayout = new QVBoxLayout(mainArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(createTopBar());

    pageStack = new QStackedWidget(mainArea);

    dashboardPage = new DashboardPage(pageStack);
    postManagementPage = new PostManagementPage(pageStack);

    analyticsPage = createPlaceholderPage(
        QStringLiteral("Analytics"),
        QStringLiteral("Trend analysis, platform comparison and advanced metrics will be placed here.")
        );

    userManagementPage = new UserManagementPage(pageStack);

    settingsPage = createPlaceholderPage(
        QStringLiteral("Settings"),
        QStringLiteral("System preferences, database settings and profile options will be placed here.")
        );

    pageStack->addWidget(dashboardPage);
    pageStack->addWidget(postManagementPage);
    pageStack->addWidget(analyticsPage);
    pageStack->addWidget(userManagementPage);
    pageStack->addWidget(settingsPage);

    mainLayout->addWidget(pageStack, 1);
    rootLayout->addWidget(mainArea, 1);

    setCentralWidget(rootWidget);

    applyStyleSheet();
}

// 集中管理主窗口样式，避免布局代码里混入大段 QSS。
void MainWindow::applyStyleSheet()
{
    setStyleSheet(
        "QWidget#sideBar {"
        "    background: #111827;"
        "}"
        "QWidget#mainArea {"
        "    background: #F3F4F6;"
        "}"
        "QWidget#topBar {"
        "    background: #FFFFFF;"
        "    border-bottom: 1px solid #E5E7EB;"
        "}"

        "QLabel#appName {"
        "    color: #FFFFFF;"
        "    font-size: 18px;"
        "    font-weight: 700;"
        "}"
        "QLabel#appSubTitle {"
        "    color: #9CA3AF;"
        "    font-size: 12px;"
        "}"
        "QLabel#pageTitleLabel {"
        "    color: #111827;"
        "    font-size: 18px;"
        "    font-weight: 700;"
        "}"
        "QLabel#userInfoLabel {"
        "    color: #374151;"
        "    font-size: 13px;"
        "}"

        "QPushButton#navButton {"
        "    color: #D1D5DB;"
        "    background: transparent;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 14px;"
        "    text-align: left;"
        "    font-size: 14px;"
        "}"
        "QPushButton#navButton:hover {"
        "    background: #1F2937;"
        "    color: #FFFFFF;"
        "}"
        "QPushButton#activeNavButton {"
        "    color: #FFFFFF;"
        "    background: #2563EB;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 14px;"
        "    text-align: left;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QPushButton#logoutButton {"
        "    color: #374151;"
        "    background: #F3F4F6;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 8px;"
        "    padding: 8px 14px;"
        "}"
        "QPushButton#logoutButton:hover {"
        "    background: #E5E7EB;"
        "}"

        "QFrame#placeholderCard {"
        "    background: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 12px;"
        "}"
        "QLabel#placeholderTitle {"
        "    color: #111827;"
        "    font-size: 24px;"
        "    font-weight: 700;"
        "}"
        "QLabel#placeholderText {"
        "    color: #6B7280;"
        "    font-size: 14px;"
        "}"
        );
}

// 构建左侧导航栏：应用标题、菜单按钮、页面切换信号。
QWidget* MainWindow::createSideBar()
{
    auto *sideBar = new QWidget();
    sideBar->setObjectName(QStringLiteral("sideBar"));
    sideBar->setFixedWidth(230);

    auto *layout = new QVBoxLayout(sideBar);
    layout->setContentsMargins(18, 24, 18, 18);
    layout->setSpacing(10);

    auto *appName = new QLabel(QStringLiteral("Social Media"));
    appName->setObjectName(QStringLiteral("appName"));

    auto *appSubTitle = new QLabel(QStringLiteral("Analytics Dashboard"));
    appSubTitle->setObjectName(QStringLiteral("appSubTitle"));

    layout->addWidget(appName);
    layout->addWidget(appSubTitle);
    layout->addSpacing(24);

    dashboardButton = createNavButton(QStringLiteral("Dashboard"));
    postManagementButton = createNavButton(QStringLiteral("Post Data"));
    analyticsButton = createNavButton(QStringLiteral("Analytics"));
    userManagementButton = createNavButton(QStringLiteral("User Management"));
    settingsButton = createNavButton(QStringLiteral("Settings"));

    layout->addWidget(dashboardButton);
    layout->addWidget(postManagementButton);
    layout->addWidget(analyticsButton);
    layout->addWidget(userManagementButton);
    layout->addWidget(settingsButton);
    layout->addStretch();

    connect(dashboardButton, &QPushButton::clicked,
            this, &MainWindow::showDashboardPage);

    connect(postManagementButton, &QPushButton::clicked,
            this, &MainWindow::showPostManagementPage);

    connect(analyticsButton, &QPushButton::clicked,
            this, &MainWindow::showAnalyticsPage);

    connect(userManagementButton, &QPushButton::clicked,
            this, &MainWindow::showUserManagementPage);

    connect(settingsButton, &QPushButton::clicked,
            this, &MainWindow::showSettingsPage);

    return sideBar;
}

// 构建顶部栏：当前页面标题、当前登录用户、退出按钮。
QWidget* MainWindow::createTopBar()
{
    auto *topBar = new QWidget();
    topBar->setObjectName(QStringLiteral("topBar"));
    topBar->setFixedHeight(68);

    auto *layout = new QHBoxLayout(topBar);
    layout->setContentsMargins(24, 0, 24, 0);
    layout->setSpacing(16);

    pageTitleLabel = new QLabel(QStringLiteral("Dashboard"));
    pageTitleLabel->setObjectName(QStringLiteral("pageTitleLabel"));

    userInfoLabel = new QLabel(QStringLiteral("Not logged in"));
    userInfoLabel->setObjectName(QStringLiteral("userInfoLabel"));

    logoutButton = new QPushButton(QStringLiteral("Logout"));
    logoutButton->setObjectName(QStringLiteral("logoutButton"));
    logoutButton->setCursor(Qt::PointingHandCursor);

    layout->addWidget(pageTitleLabel);
    layout->addStretch();
    layout->addWidget(userInfoLabel);
    layout->addWidget(logoutButton);

    connect(logoutButton, &QPushButton::clicked,
            this, &MainWindow::exitApplication);

    return topBar;
}

// 创建占位页：用于暂未实现的 Analytics 和 Settings 模块。
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

// 创建统一样式的左侧导航按钮。
QPushButton* MainWindow::createNavButton(const QString& text)
{
    auto *button = new QPushButton(text);
    button->setObjectName(QStringLiteral("navButton"));
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(42);

    return button;
}

// 切换到 Dashboard 页面。
void MainWindow::showDashboardPage()
{
    dashboardPage->refreshDashboard();

    navigateTo(
        dashboardPage,
        QStringLiteral("Dashboard"),
        dashboardButton
        );
}

// 切换到帖子数据管理页面。
void MainWindow::showPostManagementPage()
{
    postManagementPage->refreshPosts();

    navigateTo(
        postManagementPage,
        QStringLiteral("Post Data"),
        postManagementButton
        );
}

// 切换到 Analytics 页面。
void MainWindow::showAnalyticsPage()
{
    navigateTo(
        analyticsPage,
        QStringLiteral("Analytics"),
        analyticsButton
        );
}

// 切换到用户管理页面，进入前先做管理员权限检查。
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
        userManagementButton
        );
}

// 切换到 Settings 页面。
void MainWindow::showSettingsPage()
{
    navigateTo(
        settingsPage,
        QStringLiteral("Settings"),
        settingsButton
        );
}

// 统一处理页面跳转，避免每个 showXXXPage 重复三段相同代码。
void MainWindow::navigateTo(QWidget *page,
                            const QString& title,
                            QPushButton *activeButton)
{
    if (!page || !pageStack) {
        return;
    }

    pageStack->setCurrentWidget(page);
    pageTitleLabel->setText(title);
    setActiveNavButton(activeButton);
}

// 退出程序前二次确认，避免用户误点。
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

// 刷新左侧导航高亮状态。
void MainWindow::setActiveNavButton(QPushButton *activeButton)
{
    const QList<QPushButton*> buttons = {
        dashboardButton,
        postManagementButton,
        analyticsButton,
        userManagementButton,
        settingsButton
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

        // objectName 改变后手动刷新 QSS，否则高亮样式可能不会立即生效。
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }
}

// 根据当前用户角色刷新主界面可用入口。
void MainWindow::updateRoleAccess()
{
    const bool canManageUsers = isAdminUser();

    userManagementButton->setVisible(canManageUsers);

    // 防止普通用户因为后续代码跳转而停留在用户管理页。
    if (!canManageUsers && pageStack->currentWidget() == userManagementPage) {
        showDashboardPage();
    }
}

// 管理员判断统一收口，避免到处直接比较 role 字符串。
bool MainWindow::isAdminUser() const
{
    return currentUser.isValid() && currentUser.isAdmin();
}
