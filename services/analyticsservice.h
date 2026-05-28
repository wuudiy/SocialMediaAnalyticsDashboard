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
