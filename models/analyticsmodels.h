#ifndef ANALYTICSMODELS_H
#define ANALYTICSMODELS_H

#include "post.h"

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
 * Analytics / Export 页面统一筛选条件。
 *
 * platform 为空：全部平台。
 * startDate 无效：不限制开始日期。
 * endDate 无效：不限制结束日期。
 *
 * 数据隔离字段：
 * - includeAllUsers = true：管理员查看全部数据；
 * - includeAllUsers = false：普通用户只查看 ownerUserId 对应的数据；
 * - ownerUserId：当前普通用户 ID。
 */
struct AnalyticsFilter
{
    QString platform;
    QDate startDate;
    QDate endDate;

    int ownerUserId = -1;
    bool includeAllUsers = true;
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

#endif // ANALYTICSMODELS_H
