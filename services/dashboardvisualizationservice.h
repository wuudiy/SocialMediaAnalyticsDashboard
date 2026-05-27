#ifndef DASHBOARDVISUALIZATIONSERVICE_H
#define DASHBOARDVISUALIZATIONSERVICE_H

#include <QDate>
#include <QList>
#include <QString>

/*
 * Dashboard 首页统计卡片数据。
 *
 * 说明：
 * - totalPosts：帖子总数；
 * - totalInteractions：总互动量，likes + comments + shares；
 * - totalViews：总浏览量；
 * - engagementRate：整体互动率，totalInteractions / totalViews。
 */
struct DashboardVisualizationSummary
{
    int totalPosts = 0;
    qint64 totalInteractions = 0;
    qint64 totalViews = 0;
    double engagementRate = 0.0;
};

/*
 * 按平台聚合的数据。
 *
 * 用途：
 * - 平台帖子占比饼图；
 * - 平台互动量柱状图。
 */
struct PlatformMetric
{
    QString platform;
    int postCount = 0;
    qint64 interactions = 0;
    qint64 views = 0;
    double engagementRate = 0.0;
};

/*
 * 按日期聚合的数据。
 *
 * 用途：
 * - 近 14 天互动趋势折线图。
 */
struct DailyMetric
{
    QDate date;
    int postCount = 0;
    qint64 interactions = 0;
    qint64 views = 0;
};

/*
 * 热门帖子排行数据。
 *
 * 用途：
 * - Dashboard 底部 Top 5 热门帖子表格。
 */
struct TopPostMetric
{
    int postId = 0;
    QString platform;
    QString accountName;
    QString content;
    QDate publishDate;
    qint64 interactions = 0;
    qint64 views = 0;
    double engagementRate = 0.0;
};

/*
 * DashboardVisualizationService
 *
 * 只负责 Dashboard 可视化模块需要的数据库统计查询。
 *
 * 注意：
 * - 不负责界面；
 * - 不负责图表绘制；
 * - 不修改 posts 表数据；
 * - 不影响 AnalyticsPage。
 */
class DashboardVisualizationService
{
public:
    DashboardVisualizationService();

    DashboardVisualizationSummary loadSummary() const;

    QList<PlatformMetric> loadPlatformMetrics() const;

    QList<DailyMetric> loadDailyMetrics(int days = 14) const;

    QList<TopPostMetric> loadTopPosts(int limit = 5) const;
};

#endif // DASHBOARDVISUALIZATIONSERVICE_H
