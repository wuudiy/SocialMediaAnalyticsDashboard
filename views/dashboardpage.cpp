#include "dashboardpage.h"

#include <QAbstractItemView>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
    welcomeLabel(nullptr),
    subtitleLabel(nullptr),
    totalPostsValueLabel(nullptr),
    totalInteractionsValueLabel(nullptr),
    totalViewsValueLabel(nullptr),
    engagementRateValueLabel(nullptr),
    topPostTitleLabel(nullptr),
    topPostDetailLabel(nullptr),
    recentPostsTable(nullptr)
{
    buildUi();
    refreshDashboard();
}

// 刷新欢迎信息。这里只更新展示文本，不做用户权限判断。
void DashboardPage::setCurrentUser(const User& user)
{
    welcomeLabel->setText(
        QStringLiteral("Welcome back, %1").arg(user.username)
        );

    subtitleLabel->setText(
        QStringLiteral("Here is the latest overview of your local social media data.")
        );

    refreshDashboard();
}

// 创建 Dashboard 页面结构：欢迎信息、统计卡片、热门帖子、最近帖子表格。
void DashboardPage::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 22, 24, 24);
    rootLayout->setSpacing(18);

    welcomeLabel = new QLabel(QStringLiteral("Welcome back"));
    welcomeLabel->setObjectName(QStringLiteral("pageTitle"));

    subtitleLabel = new QLabel(
        QStringLiteral("Here is the latest overview of your local social media data.")
        );
    subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));

    rootLayout->addWidget(welcomeLabel);
    rootLayout->addWidget(subtitleLabel);

    rootLayout->addLayout(createSummaryGrid());
    rootLayout->addWidget(createTopPostCard());
    rootLayout->addWidget(createRecentPostsCard(), 1);

    applyStyleSheet();
}

// 集中管理 Dashboard 样式，避免布局代码里混入大段 QSS。
void DashboardPage::applyStyleSheet()
{
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
        "QFrame#card {"
        "    background: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 12px;"
        "}"
        "QLabel#cardTitle {"
        "    color: #6B7280;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "}"
        "QLabel#cardValue {"
        "    color: #111827;"
        "    font-size: 26px;"
        "    font-weight: 700;"
        "}"
        "QLabel#cardDescription {"
        "    color: #9CA3AF;"
        "    font-size: 12px;"
        "}"
        "QLabel#panelTitle {"
        "    color: #111827;"
        "    font-size: 16px;"
        "    font-weight: 700;"
        "}"
        "QLabel#topPostTitle {"
        "    color: #111827;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"
        "QLabel#topPostDetail {"
        "    color: #6B7280;"
        "    font-size: 13px;"
        "}"
        "QTableWidget {"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 8px;"
        "    background: #FFFFFF;"
        "    gridline-color: #E5E7EB;"
        "}"
        "QHeaderView::section {"
        "    background: #F9FAFB;"
        "    color: #374151;"
        "    border: none;"
        "    border-bottom: 1px solid #E5E7EB;"
        "    padding: 8px;"
        "    font-weight: 600;"
        "}"
        );
}

QGridLayout* DashboardPage::createSummaryGrid()
{
    auto *grid = new QGridLayout();
    grid->setSpacing(16);

    grid->addWidget(
        createSummaryCard(
            QStringLiteral("Total Posts"),
            &totalPostsValueLabel,
            QStringLiteral("All imported or manually added posts")
            ),
        0,
        0
        );

    grid->addWidget(
        createSummaryCard(
            QStringLiteral("Interactions"),
            &totalInteractionsValueLabel,
            QStringLiteral("Likes + comments + shares")
            ),
        0,
        1
        );

    grid->addWidget(
        createSummaryCard(
            QStringLiteral("Views"),
            &totalViewsValueLabel,
            QStringLiteral("Total exposure or view count")
            ),
        0,
        2
        );

    grid->addWidget(
        createSummaryCard(
            QStringLiteral("Engagement Rate"),
            &engagementRateValueLabel,
            QStringLiteral("Interactions divided by views")
            ),
        0,
        3
        );

    return grid;
}

QFrame* DashboardPage::createSummaryCard(const QString& title,
                                         QLabel **valueLabel,
                                         const QString& description)
{
    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));
    card->setMinimumHeight(116);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(18, 16, 18, 16);
    layout->setSpacing(8);

    auto *titleLabel = new QLabel(title);
    titleLabel->setObjectName(QStringLiteral("cardTitle"));

    auto *numberLabel = new QLabel(QStringLiteral("0"));
    numberLabel->setObjectName(QStringLiteral("cardValue"));

    auto *descriptionLabel = new QLabel(description);
    descriptionLabel->setObjectName(QStringLiteral("cardDescription"));
    descriptionLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addWidget(numberLabel);
    layout->addWidget(descriptionLabel);
    layout->addStretch();

    if (valueLabel) {
        *valueLabel = numberLabel;
    }

    return card;
}

QFrame* DashboardPage::createTopPostCard()
{
    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));
    card->setMinimumHeight(110);

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(10);

    auto *titleLabel = new QLabel(QStringLiteral("Top Post"));
    titleLabel->setObjectName(QStringLiteral("panelTitle"));

    topPostTitleLabel = new QLabel(QStringLiteral("No post data yet."));
    topPostTitleLabel->setObjectName(QStringLiteral("topPostTitle"));
    topPostTitleLabel->setWordWrap(true);

    topPostDetailLabel = new QLabel(QStringLiteral("Add post data in Post Data page first."));
    topPostDetailLabel->setObjectName(QStringLiteral("topPostDetail"));
    topPostDetailLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addWidget(topPostTitleLabel);
    layout->addWidget(topPostDetailLabel);

    return card;
}

QFrame* DashboardPage::createRecentPostsCard()
{
    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(12);

    auto *titleLabel = new QLabel(QStringLiteral("Recent Posts"));
    titleLabel->setObjectName(QStringLiteral("panelTitle"));

    recentPostsTable = new QTableWidget();
    setupRecentPostsTable();

    layout->addWidget(titleLabel);
    layout->addWidget(recentPostsTable, 1);

    return card;
}

void DashboardPage::setupRecentPostsTable()
{
    recentPostsTable->setColumnCount(7);

    recentPostsTable->setHorizontalHeaderLabels({
        QStringLiteral("Platform"),
        QStringLiteral("Account"),
        QStringLiteral("Content"),
        QStringLiteral("Date"),
        QStringLiteral("Interactions"),
        QStringLiteral("Views"),
        QStringLiteral("Rate")
    });

    recentPostsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentPostsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    recentPostsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recentPostsTable->verticalHeader()->setVisible(false);
    recentPostsTable->horizontalHeader()->setStretchLastSection(true);
    recentPostsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
}

void DashboardPage::refreshDashboard()
{
    const DashboardSummary summary = analyticsService.loadDashboardSummary();

    totalPostsValueLabel->setText(QString::number(summary.totalPosts));
    totalInteractionsValueLabel->setText(QString::number(summary.totalInteractions));
    totalViewsValueLabel->setText(QString::number(summary.totalViews));
    engagementRateValueLabel->setText(formatPercent(summary.averageEngagementRate));

    if (summary.totalPosts <= 0) {
        topPostTitleLabel->setText(QStringLiteral("No post data yet."));
        topPostDetailLabel->setText(QStringLiteral("Go to Post Data page and add some records first."));
    } else {
        topPostTitleLabel->setText(summary.topPostContent);
        topPostDetailLabel->setText(
            QStringLiteral("Platform: %1 | Interactions: %2")
                .arg(summary.topPostPlatform)
                .arg(summary.topPostInteractions)
            );
    }

    fillRecentPostsTable(analyticsService.loadRecentPosts(8));
}

void DashboardPage::fillRecentPostsTable(const QList<Post>& posts)
{
    recentPostsTable->setRowCount(posts.size());

    for (int row = 0; row < posts.size(); ++row) {
        const Post post = posts.at(row);

        const QStringList values = {
            post.platform,
            post.accountName,
            post.content,
            post.publishDate.toString(QStringLiteral("yyyy-MM-dd")),
            QString::number(post.interactionCount()),
            QString::number(post.views),
            formatPercent(post.engagementRate())
        };

        for (int column = 0; column < values.size(); ++column) {
            auto *item = new QTableWidgetItem(values.at(column));

            if (column >= 4) {
                item->setTextAlignment(Qt::AlignCenter);
            }

            recentPostsTable->setItem(row, column, item);
        }
    }
}

QString DashboardPage::formatPercent(double value) const
{
    return QStringLiteral("%1%").arg(value * 100.0, 0, 'f', 2);
}
