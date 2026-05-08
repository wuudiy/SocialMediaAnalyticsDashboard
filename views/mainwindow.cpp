#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dashboardpage.h"
#include "usermanagementpage.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
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
    analyticsButton(nullptr),
    userManagementButton(nullptr),
    settingsButton(nullptr),
    logoutButton(nullptr),
    pageStack(nullptr),
    dashboardPage(nullptr),
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
    pageStack->addWidget(analyticsPage);
    pageStack->addWidget(userManagementPage);
    pageStack->addWidget(settingsPage);

    mainLayout->addWidget(pageStack, 1);

    rootLayout->addWidget(mainArea, 1);

    setCentralWidget(rootWidget);

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
    analyticsButton = createNavButton(QStringLiteral("Analytics"));
    userManagementButton = createNavButton(QStringLiteral("User Management"));
    settingsButton = createNavButton(QStringLiteral("Settings"));

    layout->addWidget(dashboardButton);
    layout->addWidget(analyticsButton);
    layout->addWidget(userManagementButton);
    layout->addWidget(settingsButton);
    layout->addStretch();

    connect(dashboardButton, &QPushButton::clicked,
            this, &MainWindow::showDashboardPage);

    connect(analyticsButton, &QPushButton::clicked,
            this, &MainWindow::showAnalyticsPage);

    connect(userManagementButton, &QPushButton::clicked,
            this, &MainWindow::showUserManagementPage);

    connect(settingsButton, &QPushButton::clicked,
            this, &MainWindow::showSettingsPage);

    return sideBar;
}

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

    layout->addWidget(pageTitleLabel);
    layout->addStretch();
    layout->addWidget(userInfoLabel);
    layout->addWidget(logoutButton);

    connect(logoutButton, &QPushButton::clicked,
            this, &MainWindow::exitApplication);

    return topBar;
}

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

QPushButton* MainWindow::createNavButton(const QString& text)
{
    auto *button = new QPushButton(text);
    button->setObjectName(QStringLiteral("navButton"));
    button->setCursor(Qt::PointingHandCursor);
    button->setMinimumHeight(42);
    return button;
}

void MainWindow::showDashboardPage()
{
    pageStack->setCurrentWidget(dashboardPage);
    pageTitleLabel->setText(QStringLiteral("Dashboard"));
    setActiveNavButton(dashboardButton);
}

void MainWindow::showAnalyticsPage()
{
    pageStack->setCurrentWidget(analyticsPage);
    pageTitleLabel->setText(QStringLiteral("Analytics"));
    setActiveNavButton(analyticsButton);
}

void MainWindow::showUserManagementPage()
{
    if (currentUser.role != QStringLiteral("admin")) {
        QMessageBox::warning(
            this,
            QStringLiteral("Access Denied"),
            QStringLiteral("Only admin users can open User Management.")
            );
        return;
    }

    pageStack->setCurrentWidget(userManagementPage);
    pageTitleLabel->setText(QStringLiteral("User Management"));
    setActiveNavButton(userManagementButton);
}

void MainWindow::showSettingsPage()
{
    pageStack->setCurrentWidget(settingsPage);
    pageTitleLabel->setText(QStringLiteral("Settings"));
    setActiveNavButton(settingsButton);
}

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

void MainWindow::setActiveNavButton(QPushButton *activeButton)
{
    const QList<QPushButton*> buttons = {
        dashboardButton,
        analyticsButton,
        userManagementButton,
        settingsButton
    };

    for (QPushButton *button : buttons) {
        if (!button) {
            continue;
        }

        button->setObjectName(button == activeButton
                                  ? QStringLiteral("activeNavButton")
                                  : QStringLiteral("navButton"));

        button->style()->unpolish(button);
        button->style()->polish(button);
    }
}

void MainWindow::updateRoleAccess()
{
    const bool isAdmin = (currentUser.role == QStringLiteral("admin"));

    userManagementButton->setVisible(isAdmin);

    if (!isAdmin && pageStack->currentWidget() == userManagementPage) {
        showDashboardPage();
    }
}
