#include "analyticspage.h"
#include "ui_AnalyticsPage.h"

#include "../services/analyticsservice.h"

#include <QDate>
#include <QHeaderView>
#include <QTableWidgetItem>

AnalyticsPage::AnalyticsPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::AnalyticsPage),
      analyticsService(new AnalyticsService())
{
    ui->setupUi(this);
    setupComboBox();
    setupTables();
    refreshData();

    connect(ui->filterButton, &QPushButton::clicked, this, &AnalyticsPage::onFilterButtonClicked);
    connect(ui->refreshButton, &QPushButton::clicked, this, &AnalyticsPage::onRefreshButtonClicked);
}

AnalyticsPage::~AnalyticsPage()
{
    delete analyticsService;
    delete ui;
}

void AnalyticsPage::setupComboBox()
{
    ui->platformComboBox->addItem(tr("All Platforms"), QString());
    ui->platformComboBox->addItem(tr("Weibo"), QStringLiteral("Weibo"));
    ui->platformComboBox->addItem(tr("Douyin"), QStringLiteral("Douyin"));
    ui->platformComboBox->addItem(tr("Bilibili"), QStringLiteral("Bilibili"));
    ui->platformComboBox->addItem(tr("Xiaohongshu"), QStringLiteral("Xiaohongshu"));
    ui->platformComboBox->addItem(tr("WeChat"), QStringLiteral("WeChat"));

    ui->startDateEdit->setDate(QDate::currentDate().addDays(-30));
    ui->endDateEdit->setDate(QDate::currentDate());
}

void AnalyticsPage::setupTables()
{
    ui->platformTable->setColumnCount(7);
    ui->platformTable->setHorizontalHeaderLabels({
        tr("Platform"),
        tr("Post Count"),
        tr("Likes"),
        tr("Comments"),
        tr("Shares"),
        tr("Views"),
        tr("Engagement Rate")
    });
    ui->platformTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->platformTable->setSortingEnabled(true);

    ui->trendTable->setColumnCount(4);
    ui->trendTable->setHorizontalHeaderLabels({
        tr("Date"),
        tr("Post Count"),
        tr("Total Interactions"),
        tr("Engagement Rate")
    });
    ui->trendTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->trendTable->setSortingEnabled(true);

    ui->topPostsTable->setColumnCount(6);
    ui->topPostsTable->setHorizontalHeaderLabels({
        tr("Platform"),
        tr("Account"),
        tr("Content"),
        tr("Interactions"),
        tr("Views"),
        tr("Engagement Rate")
    });
    ui->topPostsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->topPostsTable->setSortingEnabled(true);
}

void AnalyticsPage::refreshData()
{
    loadSummaryData();
    loadPlatformStatistics();
    loadDateTrends();
    loadTopPosts();
}

void AnalyticsPage::loadSummaryData()
{
    DashboardSummary summary = analyticsService->loadDashboardSummary();

    ui->totalPostsValue->setText(QString::number(summary.totalPosts));
    ui->totalInteractionsValue->setText(QString::number(summary.totalInteractions));
    ui->totalViewsValue->setText(QString::number(summary.totalViews));
    ui->avgEngagementValue->setText(QStringLiteral("%1%").arg(summary.averageEngagementRate * 100, 0, 'f', 2));
}

void AnalyticsPage::loadPlatformStatistics()
{
    QList<PlatformStatistics> statsList = analyticsService->getPlatformStatistics(currentFilter.platform);

    ui->platformTable->setRowCount(statsList.size());

    int row = 0;
    for (const PlatformStatistics& stats : statsList) {
        ui->platformTable->setItem(row, 0, new QTableWidgetItem(stats.platform));
        ui->platformTable->setItem(row, 1, new QTableWidgetItem(QString::number(stats.postCount)));
        ui->platformTable->setItem(row, 2, new QTableWidgetItem(QString::number(stats.totalLikes)));
        ui->platformTable->setItem(row, 3, new QTableWidgetItem(QString::number(stats.totalComments)));
        ui->platformTable->setItem(row, 4, new QTableWidgetItem(QString::number(stats.totalShares)));
        ui->platformTable->setItem(row, 5, new QTableWidgetItem(QString::number(stats.totalViews)));
        ui->platformTable->setItem(row, 6, new QTableWidgetItem(QStringLiteral("%1%").arg(stats.averageEngagementRate * 100, 0, 'f', 2)));
        row++;
    }
}

void AnalyticsPage::loadDateTrends()
{
    QList<DateTrend> trends = analyticsService->getDateTrends(currentFilter.startDate, currentFilter.endDate);

    ui->trendTable->setRowCount(trends.size());

    int row = 0;
    for (const DateTrend& trend : trends) {
        ui->trendTable->setItem(row, 0, new QTableWidgetItem(trend.date.toString(QStringLiteral("yyyy-MM-dd"))));
        ui->trendTable->setItem(row, 1, new QTableWidgetItem(QString::number(trend.postCount)));
        ui->trendTable->setItem(row, 2, new QTableWidgetItem(QString::number(trend.totalInteractions)));
        ui->trendTable->setItem(row, 3, new QTableWidgetItem(QStringLiteral("%1%").arg(trend.averageEngagementRate * 100, 0, 'f', 2)));
        row++;
    }
}

void AnalyticsPage::loadTopPosts()
{
    QList<Post> posts = analyticsService->getTopPosts(10);

    ui->topPostsTable->setRowCount(posts.size());

    int row = 0;
    for (const Post& post : posts) {
        ui->topPostsTable->setItem(row, 0, new QTableWidgetItem(post.platform));
        ui->topPostsTable->setItem(row, 1, new QTableWidgetItem(post.accountName));
        ui->topPostsTable->setItem(row, 2, new QTableWidgetItem(post.content.left(50) + (post.content.size() > 50 ? QStringLiteral("...") : QString())));
        ui->topPostsTable->setItem(row, 3, new QTableWidgetItem(QString::number(post.interactionCount())));
        ui->topPostsTable->setItem(row, 4, new QTableWidgetItem(QString::number(post.views)));
        ui->topPostsTable->setItem(row, 5, new QTableWidgetItem(QStringLiteral("%1%").arg(post.engagementRate() * 100, 0, 'f', 2)));
        row++;
    }
}

void AnalyticsPage::onFilterButtonClicked()
{
    currentFilter.platform = ui->platformComboBox->currentData().toString();
    currentFilter.startDate = ui->startDateEdit->date();
    currentFilter.endDate = ui->endDateEdit->date();

    refreshData();
}

void AnalyticsPage::onRefreshButtonClicked()
{
    currentFilter = AnalyticsFilter();
    ui->platformComboBox->setCurrentIndex(0);
    ui->startDateEdit->setDate(QDate::currentDate().addDays(-30));
    ui->endDateEdit->setDate(QDate::currentDate());

    refreshData();
}
