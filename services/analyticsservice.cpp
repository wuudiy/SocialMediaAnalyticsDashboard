#include "analyticsservice.h"

#include "../infrastructure/databasemanager.h"
#include "../repositories/postrepository.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QtGlobal>

namespace
{
QString normalizedPlatform(const AnalyticsFilter& filter)
{
    return filter.platform.trimmed();
}

bool hasPlatformFilter(const AnalyticsFilter& filter)
{
    return !normalizedPlatform(filter).isEmpty();
}

bool hasStartDateFilter(const AnalyticsFilter& filter)
{
    return filter.startDate.isValid();
}

bool hasEndDateFilter(const AnalyticsFilter& filter)
{
    return filter.endDate.isValid();
}

bool hasOwnerFilter(const AnalyticsFilter& filter)
{
    /*
     * 数据隔离核心判断：
     * - 管理员 includeAllUsers = true，不追加 created_by_user_id 条件；
     * - 普通用户 includeAllUsers = false，只查询 ownerUserId 对应数据。
     */
    return !filter.includeAllUsers;
}

/*
 * 统一追加 posts 表筛选条件。
 *
 * 必须先 appendPostFilters(sql, filter)，再 query.prepare(sql)。
 */
void appendPostFilters(QString& sql,
                       const AnalyticsFilter& filter)
{
    sql += QStringLiteral("WHERE 1 = 1 ");

    if (hasPlatformFilter(filter)) {
        sql += QStringLiteral("AND platform = :platform ");
    }

    if (hasStartDateFilter(filter)) {
        sql += QStringLiteral("AND publish_date >= :start_date ");
    }

    if (hasEndDateFilter(filter)) {
        sql += QStringLiteral("AND publish_date <= :end_date ");
    }

    if (hasOwnerFilter(filter)) {
        sql += QStringLiteral("AND created_by_user_id = :owner_user_id ");
    }
}

/*
 * 统一绑定筛选参数。
 */
void bindPostFilters(QSqlQuery& query,
                     const AnalyticsFilter& filter)
{
    if (hasPlatformFilter(filter)) {
        query.bindValue(QStringLiteral(":platform"), normalizedPlatform(filter));
    }

    if (hasStartDateFilter(filter)) {
        query.bindValue(QStringLiteral(":start_date"),
                        filter.startDate.toString(Qt::ISODate));
    }

    if (hasEndDateFilter(filter)) {
        query.bindValue(QStringLiteral(":end_date"),
                        filter.endDate.toString(Qt::ISODate));
    }

    if (hasOwnerFilter(filter)) {
        query.bindValue(QStringLiteral(":owner_user_id"), filter.ownerUserId);
    }
}

/*
 * SELECT 字段顺序必须是：
 * post_id, platform, account_name, content, publish_date,
 * likes, comments, shares, views,
 * created_by_user_id, created_by_username
 */
Post buildPostFromQuery(const QSqlQuery& query)
{
    Post post;

    post.postId = query.value(0).toInt();
    post.platform = query.value(1).toString();
    post.accountName = query.value(2).toString();
    post.content = query.value(3).toString();
    post.publishDate = query.value(4).toDate();
    post.likes = query.value(5).toInt();
    post.comments = query.value(6).toInt();
    post.shares = query.value(7).toInt();
    post.views = query.value(8).toInt();

    post.createdByUserId = query.value(9).isNull()
                               ? -1
                               : query.value(9).toInt();

    post.createdByUsername = query.value(10).toString();

    return post;
}
}

AnalyticsService::AnalyticsService()
{
}

DashboardSummary AnalyticsService::loadDashboardSummary(const AnalyticsFilter& filter)
{
    DashboardSummary summary;

    QString sql = QStringLiteral(
        "SELECT "
        "COUNT(*), "
        "COALESCE(SUM(likes), 0), "
        "COALESCE(SUM(comments), 0), "
        "COALESCE(SUM(shares), 0), "
        "COALESCE(SUM(views), 0) "
        "FROM posts "
        );

    appendPostFilters(sql, filter);

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);
    bindPostFilters(query, filter);

    if (!query.exec()) {
        qDebug() << "Load dashboard summary failed:" << query.lastError().text();
        return summary;
    }

    if (query.next()) {
        summary.totalPosts = query.value(0).toInt();
        summary.totalLikes = query.value(1).toInt();
        summary.totalComments = query.value(2).toInt();
        summary.totalShares = query.value(3).toInt();
        summary.totalViews = query.value(4).toInt();

        summary.totalInteractions =
            summary.totalLikes + summary.totalComments + summary.totalShares;

        if (summary.totalViews > 0) {
            summary.averageEngagementRate =
                static_cast<double>(summary.totalInteractions)
                / static_cast<double>(summary.totalViews);
        }

        if (summary.totalPosts > 0) {
            summary.averageLikes =
                static_cast<double>(summary.totalLikes)
                / static_cast<double>(summary.totalPosts);
        }
    }

    QString topSql = QStringLiteral(
        "SELECT "
        "content, platform, (likes + comments + shares) AS interactions "
        "FROM posts "
        );

    appendPostFilters(topSql, filter);

    topSql += QStringLiteral(
        "ORDER BY interactions DESC, views DESC, post_id DESC "
        "LIMIT 1"
        );

    QSqlQuery topQuery(DatabaseManager::database());
    topQuery.prepare(topSql);
    bindPostFilters(topQuery, filter);

    if (!topQuery.exec()) {
        qDebug() << "Load top post failed:" << topQuery.lastError().text();
        return summary;
    }

    if (topQuery.next()) {
        summary.topPostContent = topQuery.value(0).toString();
        summary.topPostPlatform = topQuery.value(1).toString();
        summary.topPostInteractions = topQuery.value(2).toInt();
    }

    return summary;
}

/*
 * 兼容旧调用。
 *
 * 注意：
 * 当前函数仍然默认查全局最近帖子。
 * Dashboard 的完整数据隔离会在下一批单独处理。
 */
QList<Post> AnalyticsService::loadRecentPosts(int limit)
{
    PostRepository repository;
    return repository.findRecentPosts(limit);
}

AnalyticsReport AnalyticsService::generateReport(const AnalyticsFilter& filter)
{
    AnalyticsReport report;

    report.summary = loadDashboardSummary(filter);
    report.platformStats = getPlatformStatistics(filter);
    report.dateTrends = getDateTrends(filter);
    report.topPosts = getTopPosts(10, filter);

    return report;
}

QList<PlatformStatistics> AnalyticsService::getPlatformStatistics(const QString& platform)
{
    AnalyticsFilter filter;

    const QString trimmedPlatform = platform.trimmed();

    if (!trimmedPlatform.isEmpty()
        && trimmedPlatform != QStringLiteral("All Platforms")) {
        filter.platform = trimmedPlatform;
    }

    return getPlatformStatistics(filter);
}

QList<PlatformStatistics> AnalyticsService::getPlatformStatistics(const AnalyticsFilter& filter)
{
    QList<PlatformStatistics> statsList;

    QString sql = QStringLiteral(
        "SELECT "
        "platform, "
        "COUNT(*) AS post_count, "
        "COALESCE(SUM(likes), 0) AS total_likes, "
        "COALESCE(SUM(comments), 0) AS total_comments, "
        "COALESCE(SUM(shares), 0) AS total_shares, "
        "COALESCE(SUM(views), 0) AS total_views, "
        "COALESCE(SUM(likes + comments + shares), 0) AS total_interactions "
        "FROM posts "
        );

    appendPostFilters(sql, filter);

    sql += QStringLiteral(
        "GROUP BY platform "
        "ORDER BY post_count DESC, total_interactions DESC, platform ASC"
        );

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);
    bindPostFilters(query, filter);

    if (!query.exec()) {
        qDebug() << "Get platform statistics failed:" << query.lastError().text();
        return statsList;
    }

    while (query.next()) {
        PlatformStatistics stats;

        stats.platform = query.value(0).toString();
        stats.postCount = query.value(1).toInt();
        stats.totalLikes = query.value(2).toInt();
        stats.totalComments = query.value(3).toInt();
        stats.totalShares = query.value(4).toInt();
        stats.totalViews = query.value(5).toInt();
        stats.totalInteractions = query.value(6).toInt();

        if (stats.totalViews > 0) {
            stats.averageEngagementRate =
                static_cast<double>(stats.totalInteractions)
                / static_cast<double>(stats.totalViews);
        }

        statsList.append(stats);
    }

    return statsList;
}

QList<DateTrend> AnalyticsService::getDateTrends(const AnalyticsFilter& filter)
{
    QList<DateTrend> trends;

    QString sql = QStringLiteral(
        "SELECT "
        "publish_date, "
        "COUNT(*) AS post_count, "
        "COALESCE(SUM(likes + comments + shares), 0) AS total_interactions, "
        "COALESCE(SUM(views), 0) AS total_views "
        "FROM posts "
        );

    appendPostFilters(sql, filter);

    sql += QStringLiteral(
        "GROUP BY publish_date "
        "ORDER BY publish_date ASC"
        );

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);
    bindPostFilters(query, filter);

    if (!query.exec()) {
        qDebug() << "Get date trends failed:" << query.lastError().text();
        return trends;
    }

    while (query.next()) {
        DateTrend trend;

        trend.date = query.value(0).toDate();
        trend.postCount = query.value(1).toInt();
        trend.totalInteractions = query.value(2).toInt();
        trend.totalViews = query.value(3).toInt();

        if (trend.totalViews > 0) {
            trend.averageEngagementRate =
                static_cast<double>(trend.totalInteractions)
                / static_cast<double>(trend.totalViews);
        }

        trends.append(trend);
    }

    return trends;
}

QList<Post> AnalyticsService::getTopPosts(int limit,
                                          const AnalyticsFilter& filter)
{
    QList<Post> posts;

    QString sql = QStringLiteral(
        "SELECT "
        "post_id, platform, account_name, content, publish_date, "
        "likes, comments, shares, views, "
        "created_by_user_id, created_by_username "
        "FROM posts "
        );

    appendPostFilters(sql, filter);

    sql += QStringLiteral(
               "ORDER BY (likes + comments + shares) DESC, views DESC, post_id DESC "
               "LIMIT %1"
               ).arg(qMax(1, limit));

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);
    bindPostFilters(query, filter);

    if (!query.exec()) {
        qDebug() << "Get top posts failed:" << query.lastError().text();
        return posts;
    }

    while (query.next()) {
        posts.append(buildPostFromQuery(query));
    }

    return posts;
}

QList<Post> AnalyticsService::getPostsForExport(const AnalyticsFilter& filter)
{
    QList<Post> posts;

    QString sql = QStringLiteral(
        "SELECT "
        "post_id, platform, account_name, content, publish_date, "
        "likes, comments, shares, views, "
        "created_by_user_id, created_by_username "
        "FROM posts "
        );

    appendPostFilters(sql, filter);

    sql += QStringLiteral(
        "ORDER BY publish_date DESC, post_id DESC"
        );

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);
    bindPostFilters(query, filter);

    if (!query.exec()) {
        qDebug() << "Get posts for export failed:" << query.lastError().text();
        return posts;
    }

    while (query.next()) {
        posts.append(buildPostFromQuery(query));
    }

    return posts;
}
