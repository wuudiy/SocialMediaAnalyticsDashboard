#include "analyticspage.h"
#include "ui_AnalyticsPage.h"

#include "../services/analyticsservice.h"

#include <QAbstractItemView>
#include <QDate>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidgetItem>

AnalyticsPage::AnalyticsPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::AnalyticsPage),
    analyticsService(new AnalyticsService())
{
    ui->setupUi(this);

    setupComboBox();
    setupTables();

    currentFilter.platform = ui->platformComboBox->currentData().toString();
    currentFilter.startDate = ui->startDateEdit->date();
    currentFilter.endDate = ui->endDateEdit->date();

    refreshData();

    connect(ui->filterButton, &QPushButton::clicked,
            this, &AnalyticsPage::onFilterButtonClicked);

    connect(ui->refreshButton, &QPushButton::clicked,
            this, &AnalyticsPage::onRefreshButtonClicked);
}

AnalyticsPage::~AnalyticsPage()
{
    delete analyticsService;
    delete ui;
}

void AnalyticsPage::setupComboBox()
{
    ui->platformComboBox->clear();

    ui->platformComboBox->addItem(tr("All Platforms"), QString());
    ui->platformComboBox->addItem(tr("Weibo"), QStringLiteral("Weibo"));
    ui->platformComboBox->addItem(tr("Douyin"), QStringLiteral("Douyin"));
    ui->platformComboBox->addItem(tr("Bilibili"), QStringLiteral("Bilibili"));
    ui->platformComboBox->addItem(tr("Xiaohongshu"), QStringLiteral("Xiaohongshu"));

    // 注意：当前 Post Data 页面使用的是 Wechat。
    // 这里保持一致，否则 Analytics 选择该平台时查不到数据。
    ui->platformComboBox->addItem(tr("Wechat"), QStringLiteral("Wechat"));

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
    ui->platformTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->platformTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->platformTable->setSortingEnabled(true);
    ui->platformTable->verticalHeader()->setVisible(false);
    ui->platformTable->verticalHeader()->setDefaultSectionSize(34);
    ui->platformTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->platformTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    ui->trendTable->setColumnCount(5);
    ui->trendTable->setHorizontalHeaderLabels({
        tr("Date"),
        tr("Post Count"),
        tr("Total Interactions"),
        tr("Views"),
        tr("Engagement Rate")
    });
    ui->trendTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->trendTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->trendTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->trendTable->setSortingEnabled(true);
    ui->trendTable->verticalHeader()->setVisible(false);
    ui->trendTable->verticalHeader()->setDefaultSectionSize(34);
    ui->trendTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->trendTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

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
    ui->topPostsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->topPostsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->topPostsTable->setSortingEnabled(true);
    ui->topPostsTable->verticalHeader()->setVisible(false);
    ui->topPostsTable->verticalHeader()->setDefaultSectionSize(34);
    ui->topPostsTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->topPostsTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
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
    DashboardSummary summary = analyticsService->loadDashboardSummary(currentFilter);

    ui->totalPostsValue->setText(QString::number(summary.totalPosts));
    ui->totalInteractionsValue->setText(QString::number(summary.totalInteractions));
    ui->totalViewsValue->setText(QString::number(summary.totalViews));
    ui->avgEngagementValue->setText(
        QStringLiteral("%1%").arg(summary.averageEngagementRate * 100.0, 0, 'f', 2)
        );
}

void AnalyticsPage::loadPlatformStatistics()
{
    QList<PlatformStatistics> statsList =
        analyticsService->getPlatformStatistics(currentFilter);

    ui->platformTable->setSortingEnabled(false);
    ui->platformTable->clearContents();
    ui->platformTable->setRowCount(statsList.size());

    for (int row = 0; row < statsList.size(); ++row) {
        const PlatformStatistics& stats = statsList.at(row);

        ui->platformTable->setItem(row, 0, new QTableWidgetItem(stats.platform));
        ui->platformTable->setItem(row, 1, new QTableWidgetItem(QString::number(stats.postCount)));
        ui->platformTable->setItem(row, 2, new QTableWidgetItem(QString::number(stats.totalLikes)));
        ui->platformTable->setItem(row, 3, new QTableWidgetItem(QString::number(stats.totalComments)));
        ui->platformTable->setItem(row, 4, new QTableWidgetItem(QString::number(stats.totalShares)));
        ui->platformTable->setItem(row, 5, new QTableWidgetItem(QString::number(stats.totalViews)));
        ui->platformTable->setItem(
            row,
            6,
            new QTableWidgetItem(QStringLiteral("%1%").arg(stats.averageEngagementRate * 100.0, 0, 'f', 2))
            );
    }

    ui->platformTable->setSortingEnabled(true);
}

void AnalyticsPage::loadDateTrends()
{
    QList<DateTrend> trends =
        analyticsService->getDateTrends(currentFilter);

    ui->trendTable->setSortingEnabled(false);
    ui->trendTable->clearContents();
    ui->trendTable->setRowCount(trends.size());

    for (int row = 0; row < trends.size(); ++row) {
        const DateTrend& trend = trends.at(row);

        ui->trendTable->setItem(row, 0, new QTableWidgetItem(trend.date.toString(QStringLiteral("yyyy-MM-dd"))));
        ui->trendTable->setItem(row, 1, new QTableWidgetItem(QString::number(trend.postCount)));
        ui->trendTable->setItem(row, 2, new QTableWidgetItem(QString::number(trend.totalInteractions)));
        ui->trendTable->setItem(row, 3, new QTableWidgetItem(QString::number(trend.totalViews)));
        ui->trendTable->setItem(
            row,
            4,
            new QTableWidgetItem(QStringLiteral("%1%").arg(trend.averageEngagementRate * 100.0, 0, 'f', 2))
            );
    }

    ui->trendTable->setSortingEnabled(true);
}

void AnalyticsPage::loadTopPosts()
{
    QList<Post> posts =
        analyticsService->getTopPosts(10, currentFilter);

    ui->topPostsTable->setSortingEnabled(false);
    ui->topPostsTable->clearContents();
    ui->topPostsTable->setRowCount(posts.size());

    for (int row = 0; row < posts.size(); ++row) {
        const Post& post = posts.at(row);

        QString shortContent = post.content.left(50);
        if (post.content.size() > 50) {
            shortContent += QStringLiteral("...");
        }

        ui->topPostsTable->setItem(row, 0, new QTableWidgetItem(post.platform));
        ui->topPostsTable->setItem(row, 1, new QTableWidgetItem(post.accountName));
        ui->topPostsTable->setItem(row, 2, new QTableWidgetItem(shortContent));
        ui->topPostsTable->setItem(row, 3, new QTableWidgetItem(QString::number(post.interactionCount())));
        ui->topPostsTable->setItem(row, 4, new QTableWidgetItem(QString::number(post.views)));
        ui->topPostsTable->setItem(
            row,
            5,
            new QTableWidgetItem(QStringLiteral("%1%").arg(post.engagementRate() * 100.0, 0, 'f', 2))
            );
    }

    ui->topPostsTable->setSortingEnabled(true);
}

void AnalyticsPage::onFilterButtonClicked()
{
    AnalyticsFilter filter;
    filter.platform = ui->platformComboBox->currentData().toString();
    filter.startDate = ui->startDateEdit->date();
    filter.endDate = ui->endDateEdit->date();

    if (filter.startDate.isValid()
        && filter.endDate.isValid()
        && filter.startDate > filter.endDate) {
        QMessageBox::warning(
            this,
            tr("Invalid Date Range"),
            tr("Start date cannot be later than end date.")
            );
        return;
    }

    currentFilter = filter;
    refreshData();
}

void AnalyticsPage::onRefreshButtonClicked()
{
    ui->platformComboBox->setCurrentIndex(0);
    ui->startDateEdit->setDate(QDate::currentDate().addDays(-30));
    ui->endDateEdit->setDate(QDate::currentDate());

    currentFilter.platform = ui->platformComboBox->currentData().toString();
    currentFilter.startDate = ui->startDateEdit->date();
    currentFilter.endDate = ui->endDateEdit->date();

    refreshData();
}
