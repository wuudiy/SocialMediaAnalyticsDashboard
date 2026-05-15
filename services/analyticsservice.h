#ifndef ANALYTICSSERVICE_H
#define ANALYTICSSERVICE_H

#include "../models/post.h"

#include <QList>
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

    QString topPostContent;
    QString topPostPlatform;
    int topPostInteractions = 0;
};

/*
 * 数据统计服务。
 *
 * 负责：
 * - 汇总帖子数量；
 * - 计算互动量和互动率；
 * - 查询热门帖子；
 * - 给 Dashboard 返回最近帖子。
 */
class AnalyticsService
{
public:
    AnalyticsService();

    // 获取首页需要展示的总体统计数据。
    DashboardSummary loadDashboardSummary();

    // 获取最近帖子列表。
    QList<Post> loadRecentPosts(int limit = 8);
};

#endif // ANALYTICSSERVICE_H
