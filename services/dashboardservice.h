#ifndef DASHBOARDSERVICE_H
#define DASHBOARDSERVICE_H

#include "../models/dashboardmodels.h"
#include "../models/user.h"
#include "dashboardvisualizationservice.h"

/*
 * Dashboard 业务服务。
 *
 * 职责：
 * - 聚合 Dashboard 首页所需的全部数据；
 * - 统一控制默认查询范围，例如最近 14 天、Top 5；
 * - 根据当前用户执行数据隔离；
 * - 为 Controller 返回完整 DashboardViewModel。
 *
 * 注意：
 * DashboardVisualizationService 负责具体 SQL 查询；
 * DashboardService 负责把多个查询结果组织成页面需要的整体数据。
 */
class DashboardService
{
public:
    DashboardService();

    DashboardViewModel loadDashboardData(const User& currentUser) const;

private:
    DashboardVisualizationService visualizationService;
};

#endif // DASHBOARDSERVICE_H
