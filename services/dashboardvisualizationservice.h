#ifndef DASHBOARDVISUALIZATIONSERVICE_H
#define DASHBOARDVISUALIZATIONSERVICE_H

#include "../models/dashboardmodels.h"

#include <QList>

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
