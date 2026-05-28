#include "analyticspage.h"
#include "ui_AnalyticsPage.h"

#include "../controllers/analyticscontroller.h"
#include "../styles/appstyle.h"
#include "../utils/platformconstants.h"

#include <QAbstractItemView>
#include <QDate>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidgetItem>

AnalyticsPage::AnalyticsPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::AnalyticsPage),
    analyticsController(nullptr)
{
    ui->setupUi(this);

    prepareUiObjects();
    setupComboBox();
    setupTables();
    connectSignals();

    /*
     * Controller 在这里创建，但此时 MainWindow 还没有把当前登录用户传进来。
     *
     * 注意：
     * 这里不能立刻 refreshData()。
     * 否则 AnalyticsController 还没有 currentUser，
     * 会误判为 “Current user is invalid”，导致登录过程中弹出警告框。
     *
     * 正确刷新时机：
     * MainWindow 登录成功后调用 AnalyticsPage::setCurrentUser()，
     * setCurrentUser() 会把用户传给 Controller，然后再刷新数据。
     */
    analyticsController = new AnalyticsController(this, this);

    currentFilter = readFilterFromUi();
}


AnalyticsPage::~AnalyticsPage()
{
    delete ui;
}

void AnalyticsPage::setCurrentUser(const User& user)
{
    if (analyticsController) {
        analyticsController->setCurrentUser(user);
    }

    /*
     * 当前用户变化后刷新一次。
     *
     * 这样 admin / 普通用户切换登录时，
     * Analytics 页面会自动根据新用户权限重新查询数据。
     */
    refreshData();
}

void AnalyticsPage::prepareUiObjects()
{
    setObjectName(QStringLiteral("analyticsPage"));

    ui->filterFrame->setObjectName(QStringLiteral("card"));
    ui->summaryTitle->setObjectName(QStringLiteral("fieldLabel"));
    ui->platformLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->startDateLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->endDateLabel->setObjectName(QStringLiteral("fieldLabel"));

    ui->filterButton->setObjectName(QStringLiteral("primaryButton"));
    ui->refreshButton->setObjectName(QStringLiteral("secondaryButton"));

    ui->filterButton->setCursor(Qt::PointingHandCursor);
    ui->refreshButton->setCursor(Qt::PointingHandCursor);

    setStyleSheet(AppStyle::dataManagementPageStyle());
}

void AnalyticsPage::setupComboBox()
{
    ui->platformComboBox->clear();

    ui->platformComboBox->addItem(QStringLiteral("All Platforms"), QString());

    for (const QString& platform : PlatformConstants::availablePlatforms()) {
        ui->platformComboBox->addItem(platform, platform);
    }

    resetFilterControls();
}

void AnalyticsPage::setupTables()
{
    ui->platformTable->setColumnCount(7);
    ui->platformTable->setHorizontalHeaderLabels({
        QStringLiteral("Platform"),
        QStringLiteral("Post Count"),
        QStringLiteral("Likes"),
        QStringLiteral("Comments"),
        QStringLiteral("Shares"),
        QStringLiteral("Views"),
        QStringLiteral("Engagement Rate")
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
        QStringLiteral("Date"),
        QStringLiteral("Post Count"),
        QStringLiteral("Total Interactions"),
        QStringLiteral("Views"),
        QStringLiteral("Engagement Rate")
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
        QStringLiteral("Platform"),
        QStringLiteral("Account"),
        QStringLiteral("Content"),
        QStringLiteral("Interactions"),
        QStringLiteral("Views"),
        QStringLiteral("Engagement Rate")
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

void AnalyticsPage::connectSignals()
{
    connect(ui->filterButton, &QPushButton::clicked,
            this, &AnalyticsPage::onFilterButtonClicked);

    connect(ui->refreshButton, &QPushButton::clicked,
            this, &AnalyticsPage::onRefreshButtonClicked);
}

void AnalyticsPage::refreshData()
{
    emit reportRequested(currentFilter);
}

void AnalyticsPage::showReport(const AnalyticsReport& report)
{
    showSummaryData(report.summary);
    showPlatformStatistics(report.platformStats);
    showDateTrends(report.dateTrends);
    showTopPosts(report.topPosts);
}

void AnalyticsPage::showSummaryData(const DashboardSummary& summary)
{
    ui->totalPostsValue->setText(QString::number(summary.totalPosts));
    ui->totalInteractionsValue->setText(QString::number(summary.totalInteractions));
    ui->totalViewsValue->setText(QString::number(summary.totalViews));

    ui->avgEngagementValue->setText(
        QStringLiteral("%1%")
            .arg(summary.averageEngagementRate * 100.0, 0, 'f', 2)
        );
}

void AnalyticsPage::showPlatformStatistics(const QList<PlatformStatistics>& statsList)
{
    ui->platformTable->setSortingEnabled(false);
    ui->platformTable->clearContents();
    ui->platformTable->setRowCount(statsList.size());

    for (int row = 0; row < statsList.size(); ++row) {
        const PlatformStatistics& stats = statsList.at(row);

        ui->platformTable->setItem(row, 0, createTableItem(stats.platform, Qt::AlignLeft | Qt::AlignVCenter));
        ui->platformTable->setItem(row, 1, createTableItem(QString::number(stats.postCount)));
        ui->platformTable->setItem(row, 2, createTableItem(QString::number(stats.totalLikes)));
        ui->platformTable->setItem(row, 3, createTableItem(QString::number(stats.totalComments)));
        ui->platformTable->setItem(row, 4, createTableItem(QString::number(stats.totalShares)));
        ui->platformTable->setItem(row, 5, createTableItem(QString::number(stats.totalViews)));

        ui->platformTable->setItem(
            row,
            6,
            createTableItem(
                QStringLiteral("%1%")
                    .arg(stats.averageEngagementRate * 100.0, 0, 'f', 2)
                )
            );
    }

    ui->platformTable->setSortingEnabled(true);
}

void AnalyticsPage::showDateTrends(const QList<DateTrend>& trends)
{
    ui->trendTable->setSortingEnabled(false);
    ui->trendTable->clearContents();
    ui->trendTable->setRowCount(trends.size());

    for (int row = 0; row < trends.size(); ++row) {
        const DateTrend& trend = trends.at(row);

        ui->trendTable->setItem(
            row,
            0,
            createTableItem(trend.date.toString(QStringLiteral("yyyy-MM-dd")))
            );

        ui->trendTable->setItem(row, 1, createTableItem(QString::number(trend.postCount)));
        ui->trendTable->setItem(row, 2, createTableItem(QString::number(trend.totalInteractions)));
        ui->trendTable->setItem(row, 3, createTableItem(QString::number(trend.totalViews)));

        ui->trendTable->setItem(
            row,
            4,
            createTableItem(
                QStringLiteral("%1%")
                    .arg(trend.averageEngagementRate * 100.0, 0, 'f', 2)
                )
            );
    }

    ui->trendTable->setSortingEnabled(true);
}

void AnalyticsPage::showTopPosts(const QList<Post>& posts)
{
    ui->topPostsTable->setSortingEnabled(false);
    ui->topPostsTable->clearContents();
    ui->topPostsTable->setRowCount(posts.size());

    for (int row = 0; row < posts.size(); ++row) {
        const Post& post = posts.at(row);

        QString shortContent = post.content.left(50);

        if (post.content.size() > 50) {
            shortContent += QStringLiteral("...");
        }

        ui->topPostsTable->setItem(row, 0, createTableItem(post.platform, Qt::AlignLeft | Qt::AlignVCenter));
        ui->topPostsTable->setItem(row, 1, createTableItem(post.accountName, Qt::AlignLeft | Qt::AlignVCenter));
        ui->topPostsTable->setItem(row, 2, createTableItem(shortContent, Qt::AlignLeft | Qt::AlignVCenter));
        ui->topPostsTable->setItem(row, 3, createTableItem(QString::number(post.interactionCount())));
        ui->topPostsTable->setItem(row, 4, createTableItem(QString::number(post.views)));

        ui->topPostsTable->setItem(
            row,
            5,
            createTableItem(
                QStringLiteral("%1%")
                    .arg(post.engagementRate() * 100.0, 0, 'f', 2)
                )
            );
    }

    ui->topPostsTable->setSortingEnabled(true);
}

void AnalyticsPage::onFilterButtonClicked()
{
    /*
     * View 只读取筛选控件，然后发出请求。
     * 日期是否合法、数据权限如何追加，由 AnalyticsController 判断。
     */
    currentFilter = readFilterFromUi();

    refreshData();
}

void AnalyticsPage::onRefreshButtonClicked()
{
    resetFilterControls();

    currentFilter = readFilterFromUi();

    refreshData();
}

AnalyticsFilter AnalyticsPage::readFilterFromUi() const
{
    AnalyticsFilter filter;

    filter.platform = ui->platformComboBox->currentData().toString();
    filter.startDate = ui->startDateEdit->date();
    filter.endDate = ui->endDateEdit->date();

    /*
     * 不在 View 层写 ownerUserId / includeAllUsers。
     * 这两个字段由 AnalyticsController 根据当前登录用户补充。
     */
    return filter;
}

void AnalyticsPage::resetFilterControls()
{
    ui->platformComboBox->setCurrentIndex(0);
    ui->startDateEdit->setDate(QDate::currentDate().addDays(-30));
    ui->endDateEdit->setDate(QDate::currentDate());

    ui->startDateEdit->setCalendarPopup(true);
    ui->endDateEdit->setCalendarPopup(true);
}

QTableWidgetItem *AnalyticsPage::createTableItem(const QString& text,
                                                 Qt::Alignment alignment) const
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setTextAlignment(alignment);
    return item;
}

void AnalyticsPage::showWarningMessage(const QString& title,
                                       const QString& message)
{
    QMessageBox::warning(this, title, message);
}
