#include "dashboardpage.h"
#include "ui_dashboardpage.h"

#include "../services/appstyle.h"

#include <QAbstractItemView>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QList>
#include <QTableWidget>
#include <QTableWidgetItem>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::DashboardPage)
{
    /*
     * setupUi() 会读取 forms/dashboardpage.ui，
     * 自动创建欢迎标题、统计卡片、Top Post 卡片和 Recent Posts 表格。
     */
    ui->setupUi(this);

    prepareUiObjects();
    setupRecentPostsTable();
    applyStyleSheet();

    refreshDashboard();
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

/*
 * MainWindow 登录成功后会调用这个函数。
 *
 * 这里只更新欢迎文案，不做权限判断。
 * 权限判断仍然放在 MainWindow 中统一管理。
 */
void DashboardPage::setCurrentUser(const User& user)
{
    if (user.isValid() && !user.username.trimmed().isEmpty()) {
        ui->welcomeLabel->setText(
            QStringLiteral("Welcome back, %1").arg(user.username.trimmed())
            );
    } else {
        ui->welcomeLabel->setText(QStringLiteral("Welcome back"));
    }

    ui->subtitleLabel->setText(
        QStringLiteral("Here is the latest overview of your local social media data.")
        );

    refreshDashboard();
}

/*
 * 初始化 .ui 中控件的运行时属性。
 * 所以这里统一设置 objectName，让界面可以复用 AppStyle::dashboardPageStyle()。
 */
void DashboardPage::prepareUiObjects()
{
    setObjectName(QStringLiteral("dashboardPage"));

    ui->welcomeLabel->setObjectName(QStringLiteral("pageTitle"));
    ui->subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));
    ui->subtitleLabel->setWordWrap(true);

    const QList<QFrame*> cards = {
        ui->totalPostsCard,
        ui->interactionsCard,
        ui->viewsCard,
        ui->engagementCard,
        ui->topPostCard,
        ui->recentPostsCard
    };

    for (QFrame *card : cards) {
        if (card) {
            card->setObjectName(QStringLiteral("card"));
        }
    }

    const QList<QLabel*> cardTitleLabels = {
        ui->totalPostsTitleLabel,
        ui->interactionsTitleLabel,
        ui->viewsTitleLabel,
        ui->engagementTitleLabel
    };

    for (QLabel *label : cardTitleLabels) {
        if (label) {
            label->setObjectName(QStringLiteral("cardTitle"));
        }
    }

    const QList<QLabel*> cardValueLabels = {
        ui->totalPostsValueLabel,
        ui->interactionsValueLabel,
        ui->viewsValueLabel,
        ui->engagementValueLabel
    };

    for (QLabel *label : cardValueLabels) {
        if (label) {
            label->setObjectName(QStringLiteral("cardValue"));
        }
    }

    const QList<QLabel*> cardDescriptionLabels = {
        ui->totalPostsDescriptionLabel,
        ui->interactionsDescriptionLabel,
        ui->viewsDescriptionLabel,
        ui->engagementDescriptionLabel
    };

    for (QLabel *label : cardDescriptionLabels) {
        if (label) {
            label->setObjectName(QStringLiteral("cardDescription"));
            label->setWordWrap(true);
        }
    }

    ui->topPostPanelTitleLabel->setObjectName(QStringLiteral("panelTitle"));
    ui->recentPostsPanelTitleLabel->setObjectName(QStringLiteral("panelTitle"));

    ui->topPostTitleLabel->setObjectName(QStringLiteral("topPostTitle"));
    ui->topPostTitleLabel->setWordWrap(true);

    ui->topPostDetailLabel->setObjectName(QStringLiteral("topPostDetail"));
    ui->topPostDetailLabel->setWordWrap(true);

    /*
     * 让 4 个统计卡片平均分配宽度。
     * 如果窗口变宽，四张卡片会一起伸展。
     */
    ui->summaryGrid->setColumnStretch(0, 1);
    ui->summaryGrid->setColumnStretch(1, 1);
    ui->summaryGrid->setColumnStretch(2, 1);
    ui->summaryGrid->setColumnStretch(3, 1);
}

/*
 * Dashboard 样式统一交给 AppStyle。
 */
void DashboardPage::applyStyleSheet()
{
    setStyleSheet(AppStyle::dashboardPageStyle());
}

/*
 * 初始化最近帖子表格。
 *
 * 表格只负责展示数据：
 * - 禁止直接编辑；
 * - 单行选择；
 * - Content 列自动拉伸；
 * - 表头由这里统一设置。
 */
void DashboardPage::setupRecentPostsTable()
{
    ui->recentPostsTable->setColumnCount(7);

    ui->recentPostsTable->setHorizontalHeaderLabels({
        QStringLiteral("Platform"),
        QStringLiteral("Account"),
        QStringLiteral("Content"),
        QStringLiteral("Date"),
        QStringLiteral("Interactions"),
        QStringLiteral("Views"),
        QStringLiteral("Rate")
    });

    ui->recentPostsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->recentPostsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->recentPostsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->recentPostsTable->setAlternatingRowColors(true);

    ui->recentPostsTable->verticalHeader()->setVisible(false);
    ui->recentPostsTable->horizontalHeader()->setStretchLastSection(true);
    ui->recentPostsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
}

/*
 * 刷新首页统计数据。
 *
 * DashboardPage 不直接写 SQL。
 * 它只调用 AnalyticsService 获取统计结果，然后把结果显示到 .ui 控件中。
 */
void DashboardPage::refreshDashboard()
{
    const DashboardSummary summary = analyticsService.loadDashboardSummary();

    ui->totalPostsValueLabel->setText(QString::number(summary.totalPosts));
    ui->interactionsValueLabel->setText(QString::number(summary.totalInteractions));
    ui->viewsValueLabel->setText(QString::number(summary.totalViews));
    ui->engagementValueLabel->setText(formatPercent(summary.averageEngagementRate));

    if (summary.totalPosts <= 0) {
        ui->topPostTitleLabel->setText(QStringLiteral("No post data yet."));
        ui->topPostDetailLabel->setText(
            QStringLiteral("Go to Post Data page and add some records first.")
            );
    } else {
        ui->topPostTitleLabel->setText(summary.topPostContent);
        ui->topPostDetailLabel->setText(
            QStringLiteral("Platform: %1 | Interactions: %2")
                .arg(summary.topPostPlatform)
                .arg(summary.topPostInteractions)
            );
    }

    fillRecentPostsTable(analyticsService.loadRecentPosts(8));
}

/*
 * 把最近帖子列表写入表格。
 *
 * 每次刷新前先 setRowCount(posts.size())，
 * Qt 会自动清理不再需要的旧行。
 */
void DashboardPage::fillRecentPostsTable(const QList<Post>& posts)
{
    ui->recentPostsTable->setRowCount(posts.size());

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

            /*
             * 数字列居中显示：
             * - Interactions
             * - Views
             * - Rate
             */
            if (column >= 4) {
                item->setTextAlignment(Qt::AlignCenter);
            }

            ui->recentPostsTable->setItem(row, column, item);
        }
    }
}

/*
 * 把小数形式的比例转成百分比文本。
 *
 * 例如：
 * 0.125 -> 12.50%
 */
QString DashboardPage::formatPercent(double value) const
{
    return QStringLiteral("%1%").arg(value * 100.0, 0, 'f', 2);
}
