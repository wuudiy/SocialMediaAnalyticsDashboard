#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "../controllers/mainwindowcontroller.h"
#include "../styles/appstyle.h"

#include "analyticspage.h"
#include "dashboardpage.h"
#include "exportpage.h"
#include "logpage.h"
#include "postmanagementpage.h"
#include "settingspage.h"
#include "usermanagementpage.h"

#include <QFrame>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QStyle>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    mainWindowController(nullptr),
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

    /*
     * Controller 在页面和信号都准备好后创建。
     * 后续按钮点击、登录用户设置、退出请求都交给 Controller 处理。
     */
    mainWindowController = new MainWindowController(this, this);
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

    /*
     * 登录前先隐藏管理员入口。
     * 登录成功后由 MainWindowController 根据当前用户角色决定是否显示。
     */
    setAdminEntrancesVisible(false);
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
 *
 * 注意：
 * 这些槽函数现在只发出导航请求，
 * 是否允许打开页面由 MainWindowController 判断。
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
 * LoginView 登录成功后调用。
 *
 * 当前函数只负责把用户变化通知给 Controller，
 * 不再直接判断权限或生成用户信息文本。
 */
void MainWindow::setCurrentUser(const User& user)
{
    emit currentUserChanged(user);
}

/*
 * Controller 调用：应用当前登录用户状态。
 *
 * 这里仍然负责把用户信息同步给子页面，
 * 因为子页面对象由 MainWindow 创建和持有。
 */
void MainWindow::applyCurrentUserState(const User& user,
                                       const QString& windowTitle,
                                       const QString& userInfoText,
                                       bool canUseAdminPages)
{
    currentUser = user;

    setWindowTitle(windowTitle);
    ui->userInfoLabel->setText(userInfoText);

    /*
     * 当前用户需要同步给所有涉及数据权限的页面。
     *
     * - Dashboard：下一批会做数据隔离，先保留；
     * - PostManagement：已经按当前用户过滤帖子；
     * - Analytics：本批新增按当前用户过滤统计；
     * - Export：本批新增按当前用户过滤导出；
     * - UserManagement：管理员业务需要当前用户。
     */
    dashboardPage->setCurrentUser(currentUser);
    postManagementPage->setCurrentUser(currentUser);
    analyticsPage->setCurrentUser(currentUser);
    exportPage->setCurrentUser(currentUser);
    userManagementPage->setCurrentUser(currentUser);

    setAdminEntrancesVisible(canUseAdminPages);
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
 * 导航按钮槽函数。
 *
 * 每个函数只发出页面请求，不做权限判断。
 */
void MainWindow::showDashboardPage()
{
    emit navigationRequested(MainWindowPage::Dashboard);
}

void MainWindow::showPostManagementPage()
{
    emit navigationRequested(MainWindowPage::PostManagement);
}

void MainWindow::showAnalyticsPage()
{
    emit navigationRequested(MainWindowPage::Analytics);
}

void MainWindow::showExportPage()
{
    emit navigationRequested(MainWindowPage::Export);
}

void MainWindow::showUserManagementPage()
{
    emit navigationRequested(MainWindowPage::UserManagement);
}

void MainWindow::showLogPage()
{
    emit navigationRequested(MainWindowPage::OperationLogs);
}

void MainWindow::showSettingsPage()
{
    emit navigationRequested(MainWindowPage::Settings);
}

void MainWindow::exitApplication()
{
    emit logoutRequested();
}

/*
 * Controller 调用：显示某个页面。
 *
 * 页面刷新仍放在 View 中执行，
 * 因为具体哪个页面需要调用哪个 refresh 函数，
 * 属于 MainWindow 和子页面对象之间的界面协作。
 */
void MainWindow::navigateToPage(MainWindowPage page,
                                const QString& title)
{
    switch (page) {
    case MainWindowPage::Dashboard:
        dashboardPage->refreshDashboard();
        navigateTo(dashboardPage, title, ui->dashboardButton);
        return;

    case MainWindowPage::PostManagement:
        postManagementPage->refreshPosts();
        navigateTo(postManagementPage, title, ui->postManagementButton);
        return;

    case MainWindowPage::Analytics:
        analyticsPage->refreshData();
        navigateTo(analyticsPage, title, ui->analyticsButton);
        return;

    case MainWindowPage::Export:
        navigateTo(exportPage, title, ui->exportButton);
        return;

    case MainWindowPage::UserManagement:
        navigateTo(userManagementPage, title, ui->userManagementButton);
        return;

    case MainWindowPage::OperationLogs:
        logPage->refreshLogs();
        navigateTo(logPage, title, ui->operationLogsButton);
        return;

    case MainWindowPage::Settings:
        settingsPage->refreshSettings();
        navigateTo(settingsPage, title, ui->settingsButton);
        return;
    }
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
 * 显示或隐藏管理员入口。
 *
 * 具体权限由 Controller 判断，
 * View 只负责把按钮隐藏或显示。
 */
void MainWindow::setAdminEntrancesVisible(bool visible)
{
    ui->userManagementButton->setVisible(visible);
    ui->operationLogsButton->setVisible(visible);
}

void MainWindow::showAccessDeniedMessage(const QString& title,
                                         const QString& message)
{
    QMessageBox::warning(this, title, message);
}

void MainWindow::showStatusMessage(const QString& message,
                                   int timeoutMs)
{
    statusBar()->showMessage(message, timeoutMs);
}

/*
 * 退出程序前二次确认，避免误点。
 *
 * 弹窗属于界面交互，保留在 View 层。
 */
bool MainWindow::confirmExitApplication()
{
    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        QStringLiteral("Logout"),
        QStringLiteral("Do you want to exit the application?")
        );

    return answer == QMessageBox::Yes;
}
