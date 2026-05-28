#ifndef ANALYTICSSERVICE_H
#define ANALYTICSSERVICE_H

#include "../models/analyticsmodels.h"

#include <QList>
#include <QString>

/*
 * 数据分析服务。
 *
 * 页面层只负责显示和交互；
 * SQL 查询、筛选、统计、排序都放在这里。
 *
 * 数据隔离规则：
 * - AnalyticsFilter::includeAllUsers = true 时查询全部 posts；
 * - AnalyticsFilter::includeAllUsers = false 时只查询 ownerUserId 对应的数据。
 */
class AnalyticsService
{
public:
    AnalyticsService();

    DashboardSummary loadDashboardSummary(const AnalyticsFilter& filter = AnalyticsFilter());

    QList<Post> loadRecentPosts(int limit = 8);

    AnalyticsReport generateReport(const AnalyticsFilter& filter = AnalyticsFilter());

    // 兼容旧调用：只按平台筛选，默认查询全部数据。
    QList<PlatformStatistics> getPlatformStatistics(const QString& platform);

    QList<PlatformStatistics> getPlatformStatistics(const AnalyticsFilter& filter = AnalyticsFilter());

    QList<DateTrend> getDateTrends(const AnalyticsFilter& filter = AnalyticsFilter());

    QList<Post> getTopPosts(int limit = 10,
                            const AnalyticsFilter& filter = AnalyticsFilter());

    QList<Post> getPostsForExport(const AnalyticsFilter& filter = AnalyticsFilter());
};

#endif // ANALYTICSSERVICE_H
