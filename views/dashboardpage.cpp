#include "dashboardpage.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPair>
#include <QVBoxLayout>
#include <QVector>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
    welcomeLabel(nullptr),
    subtitleLabel(nullptr)
{
    buildUi();
}

// 刷新欢迎信息。这里只更新展示文本，不做用户权限判断。
void DashboardPage::setCurrentUser(const User& user)
{
    welcomeLabel->setText(
        QStringLiteral("Welcome back, %1").arg(user.username)
        );

    subtitleLabel->setText(
        QStringLiteral("Your dashboard is ready. Data modules will be connected later.")
        );
}

// 创建 Dashboard 页面结构：标题、模块卡片、图表占位、最近内容占位。
void DashboardPage::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 22, 24, 24);
    rootLayout->setSpacing(18);

    welcomeLabel = new QLabel(QStringLiteral("Welcome back"));
    welcomeLabel->setObjectName(QStringLiteral("pageTitle"));

    subtitleLabel = new QLabel(
        QStringLiteral("Your dashboard is ready. Data modules will be connected later.")
        );
    subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));

    rootLayout->addWidget(welcomeLabel);
    rootLayout->addWidget(subtitleLabel);

    rootLayout->addLayout(createModuleGrid());
    rootLayout->addLayout(createPanelGrid());

    rootLayout->addWidget(
        createEmptyPanel(
            QStringLiteral("Recent Posts"),
            QStringLiteral("No post data yet. Recent posts will be listed here after the database module is added."),
            220
            ),
        1
        );

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

// 创建顶部模块卡片区域，后续接入真实数据时优先改这里。
QGridLayout* DashboardPage::createModuleGrid()
{
    auto *grid = new QGridLayout();
    grid->setSpacing(16);

    const QVector<QPair<QString, QString>> modules = {
        {
            QStringLiteral("Post Overview"),
            QStringLiteral("Post statistics will be shown here after the post module is connected.")
        },
        {
            QStringLiteral("Audience Overview"),
            QStringLiteral("Follower and audience data will be shown here after analytics data is available.")
        },
        {
            QStringLiteral("Engagement Overview"),
            QStringLiteral("Likes, comments, shares and engagement rate will be connected later.")
        },
        {
            QStringLiteral("Report Overview"),
            QStringLiteral("Report summaries and export status will be placed here later.")
        }
    };

    for (int i = 0; i < modules.size(); ++i) {
        grid->addWidget(
            createModuleCard(modules[i].first, modules[i].second),
            0,
            i
            );
    }

    return grid;
}

// 创建中间图表占位区域。等接入 Qt Charts 或自定义图表后替换这里。
QGridLayout* DashboardPage::createPanelGrid()
{
    auto *grid = new QGridLayout();
    grid->setSpacing(16);

    grid->addWidget(
        createEmptyPanel(
            QStringLiteral("Analytics Trend"),
            QStringLiteral("No data connected yet. This area is reserved for trend charts.")
            ),
        0,
        0
        );

    grid->addWidget(
        createEmptyPanel(
            QStringLiteral("Platform Performance"),
            QStringLiteral("No data connected yet. This area is reserved for platform comparison.")
            ),
        0,
        1
        );

    return grid;
}

// 创建一个顶部功能卡片。卡片只展示模块说明，不处理点击事件。
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

// 创建空状态面板。当前项目还没接入帖子和图表数据，所以先统一显示占位信息。
QFrame* DashboardPage::createEmptyPanel(const QString& title,
                                        const QString& description,
                                        int minimumHeight)
{
    auto *panel = new QFrame();
    panel->setObjectName(QStringLiteral("panel"));
    panel->setMinimumHeight(minimumHeight);

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
