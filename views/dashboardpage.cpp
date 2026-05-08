#include "dashboardpage.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
    welcomeLabel(nullptr),
    subtitleLabel(nullptr)
{
    buildUi();
}

void DashboardPage::setCurrentUser(const User& user)
{
    welcomeLabel->setText(
        QStringLiteral("Welcome back, %1").arg(user.username)
        );

    subtitleLabel->setText(
        QStringLiteral("Your dashboard is ready. Data modules will be connected later.")
        );
}

void DashboardPage::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 22, 24, 24);
    rootLayout->setSpacing(18);

    welcomeLabel = new QLabel(QStringLiteral("Welcome back"));
    welcomeLabel->setObjectName(QStringLiteral("pageTitle"));

    subtitleLabel = new QLabel(QStringLiteral("Your dashboard is ready. Data modules will be connected later."));
    subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));

    rootLayout->addWidget(welcomeLabel);
    rootLayout->addWidget(subtitleLabel);

    /*
     * 第一块：主页功能入口
     */
    auto *moduleLayout = new QGridLayout();
    moduleLayout->setSpacing(16);

    moduleLayout->addWidget(
        createModuleCard(
            QStringLiteral("Post Overview"),
            QStringLiteral("Post statistics will be shown here after the post module is connected.")
            ),
        0,
        0
        );

    moduleLayout->addWidget(
        createModuleCard(
            QStringLiteral("Audience Overview"),
            QStringLiteral("Follower and audience data will be shown here after analytics data is available.")
            ),
        0,
        1
        );

    moduleLayout->addWidget(
        createModuleCard(
            QStringLiteral("Engagement Overview"),
            QStringLiteral("Likes, comments, shares and engagement rate will be connected later.")
            ),
        0,
        2
        );

    moduleLayout->addWidget(
        createModuleCard(
            QStringLiteral("Report Overview"),
            QStringLiteral("Report summaries and export status will be placed here later.")
            ),
        0,
        3
        );

    rootLayout->addLayout(moduleLayout);

    /*
     * 第二块：图表区域占位
     * 现在先空着，后面接真实数据或 Qt Charts 时再填。
     */
    auto *panelLayout = new QGridLayout();
    panelLayout->setSpacing(16);

    panelLayout->addWidget(
        createEmptyPanel(
            QStringLiteral("Analytics Trend"),
            QStringLiteral("No data connected yet. This area is reserved for trend charts.")
            ),
        0,
        0
        );

    panelLayout->addWidget(
        createEmptyPanel(
            QStringLiteral("Platform Performance"),
            QStringLiteral("No data connected yet. This area is reserved for platform comparison.")
            ),
        0,
        1
        );

    rootLayout->addLayout(panelLayout);

    /*
     * 第三块：最近内容区域占位
     * 后面接 posts 表之后，这里可以换成 QTableWidget。
     */
    rootLayout->addWidget(
        createEmptyPanel(
            QStringLiteral("Recent Posts"),
            QStringLiteral("No post data yet. Recent posts will be listed here after the database module is added.")
            ),
        1
        );

    setStyleSheet(
        "QLabel#pageTitle {"
        "    font-size: 24px;"
        "    font-weight: 700;"
        "    color: #111827;"
        "}"
        "QLabel#pageSubtitle {"
        "    font-size: 13px;"
        "    color: #6B7280;"
        "}"
        "QFrame#card, QFrame#panel {"
        "    background: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 12px;"
        "}"
        "QLabel#cardTitle {"
        "    color: #111827;"
        "    font-size: 15px;"
        "    font-weight: 700;"
        "}"
        "QLabel#cardDescription {"
        "    color: #6B7280;"
        "    font-size: 12px;"
        "    line-height: 18px;"
        "}"
        "QLabel#panelTitle {"
        "    color: #111827;"
        "    font-size: 16px;"
        "    font-weight: 700;"
        "}"
        "QLabel#emptyText {"
        "    color: #9CA3AF;"
        "    font-size: 13px;"
        "}"
        );
}

QFrame* DashboardPage::createModuleCard(const QString& title,
                                        const QString& description)
{
    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));
    card->setMinimumHeight(120);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(10);

    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName(QStringLiteral("cardTitle"));

    auto *descriptionLabel = new QLabel(description);
    descriptionLabel->setObjectName(QStringLiteral("cardDescription"));
    descriptionLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addWidget(descriptionLabel);
    layout->addStretch();

    return card;
}

QFrame* DashboardPage::createEmptyPanel(const QString& title,
                                        const QString& description)
{
    auto *panel = new QFrame();
    panel->setObjectName(QStringLiteral("panel"));
    panel->setMinimumHeight(220);

    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(12);

    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName(QStringLiteral("panelTitle"));

    auto *descriptionLabel = new QLabel(description);
    descriptionLabel->setObjectName(QStringLiteral("emptyText"));
    descriptionLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->addWidget(descriptionLabel);
    layout->addStretch();

    return panel;
}
