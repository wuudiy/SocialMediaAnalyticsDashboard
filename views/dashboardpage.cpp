#include "dashboardpage.h"
#include "ui_DashboardPage.h"

#include "../controllers/dashboardcontroller.h"
#include "../styles/appstyle.h"

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
    dashboardController(nullptr),
    platformPieChartView(nullptr),
    platformBarChartView(nullptr),
    dailyTrendChartView(nullptr)
{
    ui->setupUi(this);

    prepareUiObjects();
    createChartViews();
    setupTopPostsTable();
    connectSignals();
    applyStyleSheet();

    /*
     * Controller 由页面内部创建，保持当前项目 MVC 风格一致。
     * 当前用户会在 MainWindow::applyCurrentUserState() 中通过 setCurrentUser() 注入。
     */
    dashboardController = new DashboardController(this, this);
}

DashboardPage::~DashboardPage()
{
    delete ui;
}

void DashboardPage::setCurrentUser(const User& user)
{
    if (user.isValid() && !user.username.trimmed().isEmpty()) {
        ui->welcomeLabel->setText(
            QStringLiteral("Welcome back, %1").arg(user.username.trimmed())
            );
    } else {
        ui->welcomeLabel->setText(QStringLiteral("Welcome back"));
    }

    /*
     * 数据隔离关键点：
     * Dashboard 的当前用户必须交给 Controller，
     * 由 Controller -> Service -> Repository/SQL 完成权限过滤。
     */
    if (dashboardController) {
        dashboardController->setCurrentUser(user);
    }

    refreshDashboard();
}

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

    ui->summaryGrid->setColumnStretch(0, 1);
    ui->summaryGrid->setColumnStretch(1, 1);
    ui->summaryGrid->setColumnStretch(2, 1);
    ui->summaryGrid->setColumnStretch(3, 1);

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

void DashboardPage::refreshDashboard()
{
    emit dashboardRefreshRequested();
}

void DashboardPage::onRefreshClicked()
{
    refreshDashboard();
}

void DashboardPage::showDashboard(const DashboardViewModel& viewModel)
{
    renderSummaryCards(viewModel.summary);
    renderPlatformPieChart(viewModel.platformMetrics);
    renderPlatformBarChart(viewModel.platformMetrics);
    renderDailyTrendChart(viewModel.dailyMetrics);
    renderTopPostsTable(viewModel.topPosts);
}

void DashboardPage::renderSummaryCards(const DashboardVisualizationSummary& summary)
{
    ui->totalPostsValueLabel->setText(formatNumber(summary.totalPosts));
    ui->interactionsValueLabel->setText(formatNumber(summary.totalInteractions));
    ui->viewsValueLabel->setText(formatNumber(summary.totalViews));
    ui->engagementValueLabel->setText(formatPercent(summary.engagementRate));
}

void DashboardPage::renderPlatformPieChart(const QList<PlatformMetric>& metrics)
{
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

void DashboardPage::renderPlatformBarChart(const QList<PlatformMetric>& metrics)
{
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

void DashboardPage::renderDailyTrendChart(const QList<DailyMetric>& metrics)
{
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

void DashboardPage::renderTopPostsTable(const QList<TopPostMetric>& posts)
{
    ui->topPostsTable->clearContents();
    ui->topPostsTable->setRowCount(posts.size());

    for (int row = 0; row < posts.size(); ++row) {
        const TopPostMetric post = posts.at(row);

        const QString dateText = post.publishDate.isValid()
                                     ? post.publishDate.toString(QStringLiteral("yyyy-MM-dd"))
                                     : QStringLiteral("-");

        ui->topPostsTable->setItem(row, 0, createTableItem(QString::number(row + 1)));
        ui->topPostsTable->setItem(row, 1, createTableItem(post.platform, Qt::AlignLeft | Qt::AlignVCenter));
        ui->topPostsTable->setItem(row, 2, createTableItem(post.accountName, Qt::AlignLeft | Qt::AlignVCenter));
        ui->topPostsTable->setItem(row, 3, createTableItem(shortText(post.content, 60), Qt::AlignLeft | Qt::AlignVCenter));
        ui->topPostsTable->setItem(row, 4, createTableItem(dateText));
        ui->topPostsTable->setItem(row, 5, createTableItem(formatNumber(post.interactions)));
        ui->topPostsTable->setItem(row, 6, createTableItem(formatPercent(post.engagementRate)));
    }
}

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

QTableWidgetItem *DashboardPage::createTableItem(const QString& text,
                                                 Qt::Alignment alignment) const
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setTextAlignment(alignment);
    return item;
}

QString DashboardPage::formatNumber(qint64 value) const
{
    return QString::number(value);
}

QString DashboardPage::formatPercent(double value) const
{
    return QStringLiteral("%1%").arg(value * 100.0, 0, 'f', 2);
}

QString DashboardPage::shortText(const QString& text,
                                 int maxLength) const
{
    const QString cleaned = text.simplified();

    if (cleaned.length() <= maxLength) {
        return cleaned;
    }

    return cleaned.left(maxLength - 3) + QStringLiteral("...");
}

void DashboardPage::showMessage(const QString& message,
                                bool error)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}
