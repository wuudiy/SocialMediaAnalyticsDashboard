#include "dashboardvisualizationservice.h"

#include "../infrastructure/databasemanager.h"

#include <QDate>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QtGlobal>

namespace
{
/*
 * Dashboard 数据权限 SQL 片段。
 *
 * admin：
 * - 不追加 owner 条件；
 *
 * 普通 user：
 * - 追加 created_by_user_id = :owner_user_id；
 *
 * 未登录：
 * - Controller / Service 一般不会让未登录用户进入 Dashboard；
 * - 但为了安全，这里仍然会把未登录当作普通用户处理，owner_user_id 为 -1，
 *   最终查不到数据。
 */
void appendOwnerFilter(QString& sql,
                       const User& currentUser)
{
    if (currentUser.isValid() && currentUser.isAdmin()) {
        return;
    }

    sql += QStringLiteral("AND created_by_user_id = :owner_user_id ");
}

void bindOwnerFilter(QSqlQuery& query,
                     const User& currentUser)
{
    if (currentUser.isValid() && currentUser.isAdmin()) {
        return;
    }

    const int ownerUserId = currentUser.isValid()
                                ? currentUser.userId
                                : -1;

    query.bindValue(QStringLiteral(":owner_user_id"), ownerUserId);
}
}

DashboardVisualizationService::DashboardVisualizationService()
{
}

DashboardVisualizationSummary DashboardVisualizationService::loadSummary(const User& currentUser) const
{
    DashboardVisualizationSummary summary;

    QString sql = QStringLiteral(
        "SELECT "
        "COUNT(*) AS total_posts, "
        "COALESCE(SUM(likes + comments + shares), 0) AS total_interactions, "
        "COALESCE(SUM(views), 0) AS total_views "
        "FROM posts "
        "WHERE 1 = 1 "
        );

    appendOwnerFilter(sql, currentUser);

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);
    bindOwnerFilter(query, currentUser);

    if (!query.exec()) {
        qDebug() << "Load dashboard summary failed:" << query.lastError().text();
        return summary;
    }

    if (query.next()) {
        summary.totalPosts = query.value(0).toInt();
        summary.totalInteractions = query.value(1).toLongLong();
        summary.totalViews = query.value(2).toLongLong();

        if (summary.totalViews > 0) {
            summary.engagementRate =
                static_cast<double>(summary.totalInteractions)
                / static_cast<double>(summary.totalViews);
        }
    }

    return summary;
}

QList<PlatformMetric> DashboardVisualizationService::loadPlatformMetrics(const User& currentUser) const
{
    QList<PlatformMetric> metrics;

    QString sql = QStringLiteral(
        "SELECT "
        "platform, "
        "COUNT(*) AS post_count, "
        "COALESCE(SUM(likes + comments + shares), 0) AS interactions, "
        "COALESCE(SUM(views), 0) AS views "
        "FROM posts "
        "WHERE 1 = 1 "
        );

    appendOwnerFilter(sql, currentUser);

    sql += QStringLiteral(
        "GROUP BY platform "
        "ORDER BY post_count DESC, interactions DESC, platform ASC"
        );

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);
    bindOwnerFilter(query, currentUser);

    if (!query.exec()) {
        qDebug() << "Load platform metrics failed:" << query.lastError().text();
        return metrics;
    }

    while (query.next()) {
        PlatformMetric metric;

        metric.platform = query.value(0).toString();
        metric.postCount = query.value(1).toInt();
        metric.interactions = query.value(2).toLongLong();
        metric.views = query.value(3).toLongLong();

        if (metric.views > 0) {
            metric.engagementRate =
                static_cast<double>(metric.interactions)
                / static_cast<double>(metric.views);
        }

        metrics.append(metric);
    }

    return metrics;
}

QList<DailyMetric> DashboardVisualizationService::loadDailyMetrics(const User& currentUser,
                                                                   int days) const
{
    QList<DailyMetric> metrics;

    const int safeDays = qMax(1, days);
    const QDate startDate = QDate::currentDate().addDays(-(safeDays - 1));
    const QDate endDate = QDate::currentDate();

    // 先补齐最近 N 天，默认值为 0
    for (int i = 0; i < safeDays; ++i) {
        DailyMetric metric;
        metric.date = startDate.addDays(i);
        metric.postCount = 0;
        metric.interactions = 0;
        metric.views = 0;
        metrics.append(metric);
    }

    QString sql = QStringLiteral(
        "SELECT "
        "publish_date, "
        "COUNT(*) AS post_count, "
        "COALESCE(SUM(likes + comments + shares), 0) AS interactions, "
        "COALESCE(SUM(views), 0) AS views "
        "FROM posts "
        "WHERE publish_date BETWEEN :start_date AND :end_date "
        );

    appendOwnerFilter(sql, currentUser);

    sql += QStringLiteral(
        "GROUP BY publish_date "
        "ORDER BY publish_date ASC"
        );

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);

    query.bindValue(QStringLiteral(":start_date"), startDate.toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":end_date"), endDate.toString(Qt::ISODate));

    bindOwnerFilter(query, currentUser);

    if (!query.exec()) {
        qDebug() << "Load daily metrics failed:" << query.lastError().text();
        return metrics; // 返回补 0 的 14 天，避免图表轴异常
    }

    while (query.next()) {
        const QDate date = query.value(0).toDate();
        const int index = static_cast<int>(startDate.daysTo(date));

        if (index < 0 || index >= metrics.size()) {
            continue;
        }

        metrics[index].date = date;
        metrics[index].postCount = query.value(1).toInt();
        metrics[index].interactions = query.value(2).toLongLong();
        metrics[index].views = query.value(3).toLongLong();
    }

    return metrics;
}

QList<TopPostMetric> DashboardVisualizationService::loadTopPosts(const User& currentUser,
                                                                 int limit) const
{
    QList<TopPostMetric> topPosts;

    QString sql = QStringLiteral(
        "SELECT "
        "post_id, "
        "platform, "
        "account_name, "
        "content, "
        "publish_date, "
        "(likes + comments + shares) AS interactions, "
        "views "
        "FROM posts "
        "WHERE 1 = 1 "
        );

    appendOwnerFilter(sql, currentUser);

    sql += QStringLiteral(
               "ORDER BY interactions DESC, views DESC, post_id DESC "
               "LIMIT %1"
               ).arg(qMax(1, limit));

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);
    bindOwnerFilter(query, currentUser);

    if (!query.exec()) {
        qDebug() << "Load top posts failed:" << query.lastError().text();
        return topPosts;
    }

    while (query.next()) {
        TopPostMetric post;

        post.postId = query.value(0).toInt();
        post.platform = query.value(1).toString();
        post.accountName = query.value(2).toString();
        post.content = query.value(3).toString();
        post.publishDate = query.value(4).toDate();
        post.interactions = query.value(5).toLongLong();
        post.views = query.value(6).toLongLong();

        if (post.views > 0) {
            post.engagementRate =
                static_cast<double>(post.interactions)
                / static_cast<double>(post.views);
        }

        topPosts.append(post);
    }

    return topPosts;
}

bool DashboardVisualizationService::shouldIncludeAllUsers(const User& currentUser) const
{
    return currentUser.isValid() && currentUser.isAdmin();
}

bool DashboardVisualizationService::shouldApplyOwnerFilter(const User& currentUser) const
{
    return !shouldIncludeAllUsers(currentUser);
}
