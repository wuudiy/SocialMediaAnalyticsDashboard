#ifndef DASHBOARDMODELS_H
#define DASHBOARDMODELS_H

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
 * Dashboard 页面一次刷新所需的完整数据。
 *
 * 说明：
 * Controller 从 DashboardService 获取这个结构体，
 * View 收到后只负责显示和绘制，不再主动查询数据库。
 */
struct DashboardViewModel
{
    DashboardVisualizationSummary summary;
    QList<PlatformMetric> platformMetrics;
    QList<DailyMetric> dailyMetrics;
    QList<TopPostMetric> topPosts;
};

#endif // DASHBOARDMODELS_H
