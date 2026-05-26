#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include "../models/user.h"
#include "../services/dashboardvisualizationservice.h"

#include <QList>
#include <QString>
#include <QWidget>

class QChart;
class QChartView;

QT_BEGIN_NAMESPACE
namespace Ui {
class DashboardPage;
}
QT_END_NAMESPACE

/*
 * Dashboard 首页可视化模块。
 *
 * 当前页面职责：
 * 1. 显示核心统计卡片；
 * 2. 显示平台帖子占比饼图；
 * 3. 显示平台互动量柱状图；
 * 4. 显示最近 14 天互动趋势折线图；
 * 5. 显示 Top 5 热门帖子排行。
 *
 * 注意：
 * - 本页面不直接写复杂 SQL；
 * - 数据查询交给 DashboardVisualizationService；
 * - 图表绘制交给 Qt Charts；
 */
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage();

    // MainWindow 登录成功后调用，用于显示欢迎语。
    void setCurrentUser(const User& user);

public slots:
    // 主窗口切回 Dashboard 或数据变化后调用。
    void refreshDashboard();

private slots:
    void onRefreshClicked();

private:
    void prepareUiObjects();
    void connectSignals();
    void applyStyleSheet();

    void createChartViews();
    void setupTopPostsTable();

    void refreshSummaryCards();
    void refreshPlatformPieChart();
    void refreshPlatformBarChart();
    void refreshDailyTrendChart();
    void refreshTopPostsTable();

    void replaceChart(QChartView *chartView,
                      QChart *newChart);

    QString formatNumber(qint64 value) const;
    QString formatPercent(double value) const;
    QString shortText(const QString& text,
                      int maxLength) const;

    void setMessage(const QString& message,
                    bool error = false);

private:
    Ui::DashboardPage *ui;

    DashboardVisualizationService visualizationService;

    QChartView *platformPieChartView;
    QChartView *platformBarChartView;
    QChartView *dailyTrendChartView;
};

#endif // DASHBOARDPAGE_H
