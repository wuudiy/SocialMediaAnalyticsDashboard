#include "dashboardservice.h"

DashboardService::DashboardService()
{
}

DashboardViewModel DashboardService::loadDashboardData(const User& currentUser) const
{
    DashboardViewModel viewModel;

    /*
     * Dashboard 首页的默认展示规则集中放在这里：
     * - 顶部统计卡片：按当前用户权限统计；
     * - 平台图表：按当前用户权限聚合；
     * - 趋势图：最近 14 天；
     * - 热门帖子：Top 5。
     *
     * 数据隔离规则：
     * - admin：查询全部数据；
     * - 普通 user：只查询自己的数据。
     */
    viewModel.summary = visualizationService.loadSummary(currentUser);
    viewModel.platformMetrics = visualizationService.loadPlatformMetrics(currentUser);
    viewModel.dailyMetrics = visualizationService.loadDailyMetrics(currentUser, 14);
    viewModel.topPosts = visualizationService.loadTopPosts(currentUser, 5);

    return viewModel;
}
