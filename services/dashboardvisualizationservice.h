#ifndef DASHBOARDVISUALIZATIONSERVICE_H
#define DASHBOARDVISUALIZATIONSERVICE_H

#include "../models/dashboardmodels.h"
#include "../models/user.h"

#include <QList>

/*
 * DashboardVisualizationService
 *
 * 只负责 Dashboard 可视化模块需要的数据库统计查询。
 *
 * 数据隔离规则：
 * - admin 用户查询全部 posts；
 * - 普通 user 只查询 created_by_user_id = 当前用户 ID 的 posts；
 * - 未登录或无效用户不查询任何普通用户数据。
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

    DashboardVisualizationSummary loadSummary(const User& currentUser) const;

    QList<PlatformMetric> loadPlatformMetrics(const User& currentUser) const;

    QList<DailyMetric> loadDailyMetrics(const User& currentUser,
                                        int days = 14) const;

    QList<TopPostMetric> loadTopPosts(const User& currentUser,
                                      int limit = 5) const;

private:
    bool shouldIncludeAllUsers(const User& currentUser) const;

    bool shouldApplyOwnerFilter(const User& currentUser) const;
};

#endif // DASHBOARDVISUALIZATIONSERVICE_H
