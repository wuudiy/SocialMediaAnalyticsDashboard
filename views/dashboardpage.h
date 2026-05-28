#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include "../models/dashboardmodels.h"
#include "../models/user.h"

#include <QList>
#include <QString>
#include <QWidget>

class QChart;
class QChartView;
class QTableWidgetItem;

QT_BEGIN_NAMESPACE
namespace Ui {
class DashboardPage;
}
QT_END_NAMESPACE

class DashboardController;

/*
 * Dashboard 首页可视化模块。
 *
 * MVC 重构后，本类只负责 View 层：
 * - 显示欢迎语；
 * - 显示核心统计卡片；
 * - 绘制平台帖子占比饼图；
 * - 绘制平台互动量柱状图；
 * - 绘制最近 14 天互动趋势折线图；
 * - 显示 Top 5 热门帖子排行；
 * - 发出刷新请求信号。
 *
 * 数据隔离由 DashboardController / DashboardService 根据当前用户处理。
 * View 不直接判断 admin / user，不直接拼 SQL。
 */
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage();

    // MainWindow 登录成功后调用，用于显示欢迎语，并把当前用户交给 Controller。
    void setCurrentUser(const User& user);

public slots:
    void refreshDashboard();

    void showDashboard(const DashboardViewModel& viewModel);

    void showMessage(const QString& message,
                     bool error = false);

signals:
    void dashboardRefreshRequested();

private slots:
    void onRefreshClicked();

private:
    void prepareUiObjects();
    void connectSignals();
    void applyStyleSheet();

    void createChartViews();
    void setupTopPostsTable();

    void renderSummaryCards(const DashboardVisualizationSummary& summary);
    void renderPlatformPieChart(const QList<PlatformMetric>& metrics);
    void renderPlatformBarChart(const QList<PlatformMetric>& metrics);
    void renderDailyTrendChart(const QList<DailyMetric>& metrics);
    void renderTopPostsTable(const QList<TopPostMetric>& posts);

    void replaceChart(QChartView *chartView,
                      QChart *newChart);

    QTableWidgetItem *createTableItem(const QString& text,
                                      Qt::Alignment alignment = Qt::AlignCenter) const;

    QString formatNumber(qint64 value) const;
    QString formatPercent(double value) const;
    QString shortText(const QString& text,
                      int maxLength) const;

private:
    Ui::DashboardPage *ui;

    DashboardController *dashboardController;

    QChartView *platformPieChartView;
    QChartView *platformBarChartView;
    QChartView *dailyTrendChartView;
};

#endif // DASHBOARDPAGE_H
