#include "dashboardvisualizationservice.h"
#include "databasemanager.h"

#include <QDebug>
#include <QMap>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

DashboardVisualizationService::DashboardVisualizationService()
{
}

/*
 * 读取 Dashboard 顶部 4 个核心指标：
 * - 总帖子数；
 * - 总互动量；
 * - 总浏览量；
 * - 整体互动率。
 */
DashboardVisualizationSummary DashboardVisualizationService::loadSummary() const
{
    DashboardVisualizationSummary summary;

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT "
        "COUNT(*) AS total_posts, "
        "COALESCE(SUM(likes + comments + shares), 0) AS total_interactions, "
        "COALESCE(SUM(views), 0) AS total_views "
        "FROM posts"
        );

    if (!query.exec()) {
        qWarning() << "Dashboard summary query failed:" << query.lastError().text();
        return summary;
    }

    if (query.next()) {
        summary.totalPosts = query.value("total_posts").toInt();
        summary.totalInteractions = query.value("total_interactions").toLongLong();
        summary.totalViews = query.value("total_views").toLongLong();

        if (summary.totalViews > 0) {
            summary.engagementRate =
                static_cast<double>(summary.totalInteractions)
                / static_cast<double>(summary.totalViews);
        }
    }

    return summary;
}

/*
 * 按平台统计数据。
 *
 * 饼图使用 postCount；
 * 柱状图使用 interactions。
 */
QList<PlatformMetric> DashboardVisualizationService::loadPlatformMetrics() const
{
    QList<PlatformMetric> result;

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT "
        "platform, "
        "COUNT(*) AS post_count, "
        "COALESCE(SUM(likes + comments + shares), 0) AS interactions, "
        "COALESCE(SUM(views), 0) AS views "
        "FROM posts "
        "GROUP BY platform "
        "ORDER BY interactions DESC"
        );

    if (!query.exec()) {
        qWarning() << "Platform metrics query failed:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        PlatformMetric metric;

        metric.platform = query.value("platform").toString();
        metric.postCount = query.value("post_count").toInt();
        metric.interactions = query.value("interactions").toLongLong();
        metric.views = query.value("views").toLongLong();

        if (metric.views > 0) {
            metric.engagementRate =
                static_cast<double>(metric.interactions)
                / static_cast<double>(metric.views);
        }

        result.append(metric);
    }

    return result;
}

/*
 * 读取最近 days 天的每日趋势数据。
 *
 * 这里会主动补齐没有数据的日期。
 * 例如某天没有发帖，也会返回 interactions = 0。
 * 这样折线图不会断开。
 */
QList<DailyMetric> DashboardVisualizationService::loadDailyMetrics(int days) const
{
    QList<DailyMetric> result;

    if (days <= 0) {
        days = 14;
    }

    const QDate endDate = QDate::currentDate();
    const QDate startDate = endDate.addDays(-(days - 1));

    QMap<QDate, DailyMetric> metricMap;

    for (int i = 0; i < days; ++i) {
        const QDate date = startDate.addDays(i);

        DailyMetric emptyMetric;
        emptyMetric.date = date;

        metricMap.insert(date, emptyMetric);
    }

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT "
        "publish_date, "
        "COUNT(*) AS post_count, "
        "COALESCE(SUM(likes + comments + shares), 0) AS interactions, "
        "COALESCE(SUM(views), 0) AS views "
        "FROM posts "
        "WHERE date(publish_date) >= date(:start_date) "
        "AND date(publish_date) <= date(:end_date) "
        "GROUP BY publish_date "
        "ORDER BY publish_date ASC"
        );

    query.bindValue(":start_date", startDate.toString("yyyy-MM-dd"));
    query.bindValue(":end_date", endDate.toString("yyyy-MM-dd"));

    if (!query.exec()) {
        qWarning() << "Daily metrics query failed:" << query.lastError().text();
    } else {
        while (query.next()) {
            const QDate date = QDate::fromString(
                query.value("publish_date").toString(),
                "yyyy-MM-dd"
                );

            if (!date.isValid() || !metricMap.contains(date)) {
                continue;
            }

            DailyMetric metric;
            metric.date = date;
            metric.postCount = query.value("post_count").toInt();
            metric.interactions = query.value("interactions").toLongLong();
            metric.views = query.value("views").toLongLong();

            metricMap[date] = metric;
        }
    }

    for (auto it = metricMap.constBegin(); it != metricMap.constEnd(); ++it) {
        result.append(it.value());
    }

    return result;
}

/*
 * 读取 Top N 热门帖子。
 *
 * 排序规则：
 * interactions = likes + comments + shares
 */
QList<TopPostMetric> DashboardVisualizationService::loadTopPosts(int limit) const
{
    QList<TopPostMetric> result;

    if (limit <= 0) {
        limit = 5;
    }

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT "
        "post_id, "
        "platform, "
        "account_name, "
        "content, "
        "publish_date, "
        "(likes + comments + shares) AS interactions, "
        "views "
        "FROM posts "
        "ORDER BY interactions DESC, views DESC "
        "LIMIT :limit"
        );

    query.bindValue(":limit", limit);

    if (!query.exec()) {
        qWarning() << "Top posts query failed:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        TopPostMetric metric;

        metric.postId = query.value("post_id").toInt();
        metric.platform = query.value("platform").toString();
        metric.accountName = query.value("account_name").toString();
        metric.content = query.value("content").toString();
        metric.publishDate = QDate::fromString(
            query.value("publish_date").toString(),
            "yyyy-MM-dd"
            );
        metric.interactions = query.value("interactions").toLongLong();
        metric.views = query.value("views").toLongLong();

        if (metric.views > 0) {
            metric.engagementRate =
                static_cast<double>(metric.interactions)
                / static_cast<double>(metric.views);
        }

        result.append(metric);
    }

    return result;
}
