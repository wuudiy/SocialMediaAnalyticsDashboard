#include "dashboardpage.h"
#include "ui_dashboardpage.h"

#include "../styles/appstyle.h"
#include "../infrastructure/databasemanager.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include <QAbstractItemView>
#include <QBarCategoryAxis>
#include <QBarSeries>
#include <QBarSet>
#include <QChart>
#include <QChartView>
#include <QDateTime>
#include <QDateTimeAxis>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QLineSeries>
#include <QList>
#include <QPainter>
#include <QPieSeries>
#include <QPieSlice>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTime>
#include <QValueAxis>

#include <algorithm>

DashboardPage::DashboardPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::DashboardPage),
    platformPieChartView(nullptr),
    platformBarChartView(nullptr),
    dailyTrendChartView(nullptr)
{
    /*
     * setupUi() 读取 forms/dashboardpage.ui。
     *
     * .ui 负责固定布局：
     * - 顶部标题；
     * - 4 个统计卡片；
     * - 3 个图表容器；
     * - Top 5 表格。
     */
    ui->setupUi(this);

    prepareUiObjects();
    createChartViews();
    setupTopPostsTable();
    connectSignals();
    applyStyleSheet();

    refreshDashboard();
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

/*
 * 设置当前登录用户。
 *
 * 这里只更新欢迎语，不做权限控制。
 * 权限控制仍由 MainWindow 负责。
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

    refreshDashboard();
}

/*
 * 初始化 .ui 中已有控件的运行时属性。
 *
 * objectName 用于复用 AppStyle 中已有的 QSS。
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
        ui->platformPieCard,
        ui->platformBarCard,
        ui->dailyTrendCard,
        ui->topPostsCard
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

    const QList<QLabel*> panelTitles = {
        ui->platformPieTitleLabel,
        ui->platformBarTitleLabel,
        ui->dailyTrendTitleLabel,
        ui->topPostsTitleLabel
    };

    for (QLabel *label : panelTitles) {
        if (label) {
            label->setObjectName(QStringLiteral("panelTitle"));
        }
    }

    ui->refreshButton->setObjectName(QStringLiteral("primaryButton"));
    ui->refreshButton->setCursor(Qt::PointingHandCursor);

    ui->messageLabel->setObjectName(QStringLiteral("messageLabel"));
    ui->messageLabel->setWordWrap(true);

    /*
     * 让顶部 4 个统计卡片平均分配宽度。
     */
    ui->summaryGrid->setColumnStretch(0, 1);
    ui->summaryGrid->setColumnStretch(1, 1);
    ui->summaryGrid->setColumnStretch(2, 1);
    ui->summaryGrid->setColumnStretch(3, 1);

    /*
     * 让两个并排图表平均分配宽度。
     */
    ui->chartsGrid->setColumnStretch(0, 1);
    ui->chartsGrid->setColumnStretch(1, 1);
}

void DashboardPage::connectSignals()
{
    connect(ui->refreshButton, &QPushButton::clicked,
            this, &DashboardPage::onRefreshClicked);
}

void DashboardPage::applyStyleSheet()
{
    setStyleSheet(AppStyle::dashboardPageStyle());
}

/*
 * 创建 QChartView 并放入 .ui 中预留的容器。
 *
 * QChartView 是 QWidget，因此可以直接 addWidget 到 layout 里。
 */
void DashboardPage::createChartViews()
{
    platformPieChartView = new QChartView(this);
    platformBarChartView = new QChartView(this);
    dailyTrendChartView = new QChartView(this);

    const QList<QChartView*> chartViews = {
        platformPieChartView,
        platformBarChartView,
        dailyTrendChartView
    };

    for (QChartView *view : chartViews) {
        view->setRenderHint(QPainter::Antialiasing);
        view->setMinimumHeight(210);
    }

    ui->platformPieChartContainerLayout->addWidget(platformPieChartView);
    ui->platformBarChartContainerLayout->addWidget(platformBarChartView);
    ui->dailyTrendChartContainerLayout->addWidget(dailyTrendChartView);
}

/*
 * 初始化 Top 5 热门帖子表格。
 */
void DashboardPage::setupTopPostsTable()
{
    ui->topPostsTable->setColumnCount(7);

    ui->topPostsTable->setHorizontalHeaderLabels({
        QStringLiteral("Rank"),
        QStringLiteral("Platform"),
        QStringLiteral("Account"),
        QStringLiteral("Content"),
        QStringLiteral("Date"),
        QStringLiteral("Interactions"),
        QStringLiteral("Rate")
    });

    ui->topPostsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->topPostsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->topPostsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->topPostsTable->setAlternatingRowColors(true);
    ui->topPostsTable->verticalHeader()->setVisible(false);

    ui->topPostsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->topPostsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->topPostsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->topPostsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->topPostsTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->topPostsTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->topPostsTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
}

/*
 * 刷新整个 Dashboard。
 *
 * 页面切换回来、点击 Refresh、导入 CSV 后都可以调用。
 */
void DashboardPage::refreshDashboard()
{

    refreshSummaryCards();
    refreshPlatformPieChart();
    refreshPlatformBarChart();
    refreshDailyTrendChart();
    refreshTopPostsTable();

    setMessage(QStringLiteral("Dashboard visualization refreshed."));
}

void DashboardPage::onRefreshClicked()
{
    refreshDashboard();
}

/*
 * 刷新顶部 4 个统计卡片。
 */
void DashboardPage::refreshSummaryCards()
{
    const DashboardVisualizationSummary summary =
        visualizationService.loadSummary();

    ui->totalPostsValueLabel->setText(formatNumber(summary.totalPosts));
    ui->interactionsValueLabel->setText(formatNumber(summary.totalInteractions));
    ui->viewsValueLabel->setText(formatNumber(summary.totalViews));
    ui->engagementValueLabel->setText(formatPercent(summary.engagementRate));
}

/*
 * 刷新平台帖子占比饼图。
 *
 * QPieSeries 会根据每个 slice 的 value 自动计算占比。
 */
void DashboardPage::refreshPlatformPieChart()
{
    const QList<PlatformMetric> metrics =
        visualizationService.loadPlatformMetrics();

    auto *series = new QPieSeries();

    if (metrics.isEmpty()) {
        series->append(QStringLiteral("No Data"), 1);
    } else {
        for (const PlatformMetric& metric : metrics) {
            series->append(metric.platform, metric.postCount);
        }
    }

    for (QPieSlice *slice : series->slices()) {
        slice->setLabelVisible(true);

        if (metrics.isEmpty()) {
            slice->setLabel(QStringLiteral("No Data"));
        } else {
            slice->setLabel(
                QStringLiteral("%1: %2")
                    .arg(slice->label())
                    .arg(static_cast<int>(slice->value()))
                );
        }

        /*
         * 鼠标悬浮时轻微突出切片。
         * 这是纯 UI 交互，不影响数据。
         */
        connect(slice, &QPieSlice::hovered,
                slice, [slice](bool hovered) {
                    slice->setExploded(hovered);
                    slice->setLabelVisible(true);
                });
    }

    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QStringLiteral("Posts by Platform"));
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundVisible(false);

    replaceChart(platformPieChartView, chart);
}

/*
 * 刷新平台互动量柱状图。
 *
 * 每个平台显示一个柱子：
 * interactions = likes + comments + shares。
 */
void DashboardPage::refreshPlatformBarChart()
{
    const QList<PlatformMetric> metrics =
        visualizationService.loadPlatformMetrics();

    auto *barSet = new QBarSet(QStringLiteral("Interactions"));
    QStringList categories;

    qreal maxValue = 0;

    if (metrics.isEmpty()) {
        *barSet << 0;
        categories << QStringLiteral("No Data");
        maxValue = 1;
    } else {
        for (const PlatformMetric& metric : metrics) {
            const qreal value = static_cast<qreal>(metric.interactions);

            *barSet << value;
            categories << metric.platform;

            if (value > maxValue) {
                maxValue = value;
            }
        }
    }

    auto *series = new QBarSeries();
    series->append(barSet);

    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QStringLiteral("Interactions by Platform"));
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundVisible(false);

    auto *axisX = new QBarCategoryAxis();
    axisX->append(categories);

    auto *axisY = new QValueAxis();
    axisY->setLabelFormat(QStringLiteral("%.0f"));
    axisY->setRange(0, std::max<qreal>(1, maxValue * 1.15));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    replaceChart(platformBarChartView, chart);
}

/*
 * 刷新最近 14 天互动趋势折线图。
 *
 * 横轴：日期；
 * 纵轴：当天互动量。
 */
void DashboardPage::refreshDailyTrendChart()
{
    const QList<DailyMetric> metrics =
        visualizationService.loadDailyMetrics(14);

    auto *series = new QLineSeries();
    series->setName(QStringLiteral("Interactions"));

    qreal maxValue = 0;

    for (const DailyMetric& metric : metrics) {
        const QDateTime dateTime(metric.date, QTime(0, 0, 0));
        const qreal x = static_cast<qreal>(dateTime.toMSecsSinceEpoch());
        const qreal y = static_cast<qreal>(metric.interactions);

        series->append(x, y);

        if (y > maxValue) {
            maxValue = y;
        }
    }

    auto *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle(QStringLiteral("Recent 14-Day Interaction Trend"));
    chart->legend()->setAlignment(Qt::AlignBottom);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->setBackgroundVisible(false);

    auto *axisX = new QDateTimeAxis();
    axisX->setFormat(QStringLiteral("MM-dd"));
    axisX->setTitleText(QStringLiteral("Date"));
    axisX->setTickCount(7);

    if (!metrics.isEmpty()) {
        axisX->setRange(
            QDateTime(metrics.first().date, QTime(0, 0, 0)),
            QDateTime(metrics.last().date, QTime(23, 59, 59))
            );
    }

    auto *axisY = new QValueAxis();
    axisY->setTitleText(QStringLiteral("Interactions"));
    axisY->setLabelFormat(QStringLiteral("%.0f"));
    axisY->setRange(0, std::max<qreal>(1, maxValue * 1.15));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    replaceChart(dailyTrendChartView, chart);
}

/*
 * 刷新 Top 5 热门帖子表格。
 */
void DashboardPage::refreshTopPostsTable()
{
    const QList<TopPostMetric> posts =
        visualizationService.loadTopPosts(5);

    ui->topPostsTable->setRowCount(posts.size());

    for (int row = 0; row < posts.size(); ++row) {
        const TopPostMetric post = posts.at(row);

        const QString dateText = post.publishDate.isValid()
                                     ? post.publishDate.toString(QStringLiteral("yyyy-MM-dd"))
                                     : QStringLiteral("-");

        const QStringList values = {
            QString::number(row + 1),
            post.platform,
            post.accountName,
            shortText(post.content, 60),
            dateText,
            formatNumber(post.interactions),
            formatPercent(post.engagementRate)
        };

        for (int column = 0; column < values.size(); ++column) {
            auto *item = new QTableWidgetItem(values.at(column));

            if (column == 0 || column == 4 || column == 5 || column == 6) {
                item->setTextAlignment(Qt::AlignCenter);
            }

            ui->topPostsTable->setItem(row, column, item);
        }
    }
}

/*
 * 替换 QChartView 中的图表。
 *
 * QChartView 接管 newChart 的所有权。
 * 旧 chart 需要主动删除，避免多次刷新造成内存泄漏。
 */
void DashboardPage::replaceChart(QChartView *chartView,
                                 QChart *newChart)
{
    if (!chartView || !newChart) {
        return;
    }

    QChart *oldChart = chartView->chart();

    chartView->setChart(newChart);

    if (oldChart) {
        oldChart->deleteLater();
    }
}

/*
 * 简单数字格式化。
 *
 * 这里不用本地化分隔符，避免不同系统显示不一致。
 */
QString DashboardPage::formatNumber(qint64 value) const
{
    return QString::number(value);
}

/*
 * 百分比格式化。
 *
 * 例如：
 * 0.125 -> 12.50%
 */
QString DashboardPage::formatPercent(double value) const
{
    return QStringLiteral("%1%").arg(value * 100.0, 0, 'f', 2);
}

/*
 * 表格中显示长内容时截断，避免撑爆列宽。
 */
QString DashboardPage::shortText(const QString& text,
                                 int maxLength) const
{
    const QString cleaned = text.simplified();

    if (cleaned.length() <= maxLength) {
        return cleaned;
    }

    return cleaned.left(maxLength - 3) + QStringLiteral("...");
}

void DashboardPage::setMessage(const QString& message,
                               bool error)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}