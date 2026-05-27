#ifndef ANALYTICSSERVICE_H
#define ANALYTICSSERVICE_H

#include "../models/post.h"

#include <QDate>
#include <QList>
#include <QString>

/*
 * Dashboard / Analytics 总体统计结果。
 *
 * totalInteractions = likes + comments + shares
 * averageEngagementRate = totalInteractions / totalViews
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
 * 平台维度统计。
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
 * 日期趋势统计。
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
 * Analytics 页面统一筛选条件。
 *
 * platform 为空：全部平台。
 * startDate 无效：不限制开始日期。
 * endDate 无效：不限制结束日期。
 */
struct AnalyticsFilter
{
    QString platform;
    QDate startDate;
    QDate endDate;
};

/*
 * Analytics 页面一次刷新所需的完整数据。
 */
struct AnalyticsReport
{
    DashboardSummary summary;
    QList<PlatformStatistics> platformStats;
    QList<DateTrend> dateTrends;
    QList<Post> topPosts;
};

/*
 * 数据分析服务。
 *
 * 页面层只负责显示和交互；
 * SQL 查询、筛选、统计、排序都放在这里。
 */
class AnalyticsService
{
public:
    AnalyticsService();

    DashboardSummary loadDashboardSummary(const AnalyticsFilter& filter = AnalyticsFilter());

    QList<Post> loadRecentPosts(int limit = 8);

    AnalyticsReport generateReport(const AnalyticsFilter& filter = AnalyticsFilter());

    // 兼容 ExportPage：只按平台筛选。
    QList<PlatformStatistics> getPlatformStatistics(const QString& platform);

    // Analytics 页面使用：按平台 + 日期范围筛选。
    QList<PlatformStatistics> getPlatformStatistics(const AnalyticsFilter& filter = AnalyticsFilter());

    QList<DateTrend> getDateTrends(const AnalyticsFilter& filter = AnalyticsFilter());

    QList<Post> getTopPosts(int limit = 10, const AnalyticsFilter& filter = AnalyticsFilter());

    // 给后续报表导出模块使用。当前你这边不实现导出，只提供数据。
    QList<Post> getPostsForExport(const AnalyticsFilter& filter = AnalyticsFilter());
};

#endif // ANALYTICSSERVICE_H
