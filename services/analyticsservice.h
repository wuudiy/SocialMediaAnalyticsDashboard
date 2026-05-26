#ifndef ANALYTICSSERVICE_H
#define ANALYTICSSERVICE_H

#include "../models/post.h"

#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

/*
 * Dashboard 首页统计结果。
 *
 * 用一个简单结构体集中保存统计值。
 * 页面只负责显示，不需要知道每个指标怎么计算。
 */
struct DashboardSummary
{
    int totalPosts = 0;
    int totalLikes = 0;
    int totalComments = 0;
    int totalShares = 0;
    int totalViews = 0;
    int totalInteractions = 0;
    double averageEngagementRate = 0.0;
    double averageLikes = 0.0;

    QString topPostContent;
    QString topPostPlatform;
    int topPostInteractions = 0;
};

/*
 * 平台统计数据。
 */
struct PlatformStatistics
{
    QString platform;
    int postCount = 0;
    int totalLikes = 0;
    int totalComments = 0;
    int totalShares = 0;
    int totalViews = 0;
    int totalInteractions = 0;
    double averageEngagementRate = 0.0;
};

/*
 * 日期趋势数据。
 */
struct DateTrend
{
    QDate date;
    int postCount = 0;
    int totalInteractions = 0;
    int totalViews = 0;
    double averageEngagementRate = 0.0;
};

/*
 * 筛选条件。
 */
struct AnalyticsFilter
{
    QString platform;
    QDate startDate;
    QDate endDate;
};

/*
 * 完整的分析报告。
 */
struct AnalyticsReport
{
    DashboardSummary summary;
    QList<PlatformStatistics> platformStats;
    QList<DateTrend> dateTrends;
    QList<Post> topPosts;
};

/*
 * 数据统计服务。
 *
 * 负责：
 * - 汇总帖子数量；
 * - 计算互动量和互动率；
 * - 查询热门帖子；
 * - 按平台统计数据；
 * - 按日期统计趋势；
 * - 支持筛选条件。
 */
class AnalyticsService
{
public:
    AnalyticsService();

    // 获取首页需要展示的总体统计数据。
    DashboardSummary loadDashboardSummary();

    // 获取最近帖子列表。
    QList<Post> loadRecentPosts(int limit = 8);

    // 获取完整的分析报告。
    AnalyticsReport generateReport(const AnalyticsFilter& filter = AnalyticsFilter());

    // 按平台统计数据。
    QList<PlatformStatistics> getPlatformStatistics(const QString& platform = QString());

    // 按日期统计趋势。
    QList<DateTrend> getDateTrends(QDate startDate = QDate(), QDate endDate = QDate());

    // 获取热门帖子列表。
    QList<Post> getTopPosts(int limit = 10);
};

#endif // ANALYTICSSERVICE_H
