#include "analyticsservice.h"
#include "databasemanager.h"
#include "postrepository.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

AnalyticsService::AnalyticsService()
{
}

DashboardSummary AnalyticsService::loadDashboardSummary()
{
    DashboardSummary summary;

    QSqlQuery query(DatabaseManager::database());

    if (!query.exec(
            "SELECT "
            "COUNT(*), "
            "COALESCE(SUM(likes), 0), "
            "COALESCE(SUM(comments), 0), "
            "COALESCE(SUM(shares), 0), "
            "COALESCE(SUM(views), 0) "
            "FROM posts"
            )) {
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

    QSqlQuery topQuery(DatabaseManager::database());

    if (!topQuery.exec(
            "SELECT content, platform, (likes + comments + shares) AS interactions "
            "FROM posts "
            "ORDER BY interactions DESC, views DESC, post_id DESC "
            "LIMIT 1"
            )) {
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

QList<Post> AnalyticsService::loadRecentPosts(int limit)
{
    PostRepository repository;
    return repository.findRecentPosts(limit);
}

AnalyticsReport AnalyticsService::generateReport(const AnalyticsFilter& filter)
{
    AnalyticsReport report;

    report.summary = loadDashboardSummary();
    report.platformStats = getPlatformStatistics(filter.platform);
    report.dateTrends = getDateTrends(filter.startDate, filter.endDate);
    report.topPosts = getTopPosts(10);

    return report;
}

QList<PlatformStatistics> AnalyticsService::getPlatformStatistics(const QString& platform)
{
    QList<PlatformStatistics> statsList;

    QString sql = QStringLiteral(
        "SELECT "
        "platform, "
        "COUNT(*) as post_count, "
        "COALESCE(SUM(likes), 0) as total_likes, "
        "COALESCE(SUM(comments), 0) as total_comments, "
        "COALESCE(SUM(shares), 0) as total_shares, "
        "COALESCE(SUM(views), 0) as total_views "
        "FROM posts "
    );

    QSqlQuery query(DatabaseManager::database());

    if (!platform.isEmpty()) {
        sql += QStringLiteral("WHERE platform = :platform ");
        query.prepare(sql);
        query.bindValue(QStringLiteral(":platform"), platform);
    } else {
        sql += QStringLiteral("GROUP BY platform ");
        query.prepare(sql);
    }

    sql += QStringLiteral("ORDER BY post_count DESC");

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
        stats.totalInteractions = stats.totalLikes + stats.totalComments + stats.totalShares;

        if (stats.totalViews > 0) {
            stats.averageEngagementRate =
                static_cast<double>(stats.totalInteractions)
                / static_cast<double>(stats.totalViews);
        }

        statsList.append(stats);
    }

    return statsList;
}

QList<DateTrend> AnalyticsService::getDateTrends(QDate startDate, QDate endDate)
{
    QList<DateTrend> trends;

    QString sql = QStringLiteral(
        "SELECT "
        "publish_date, "
        "COUNT(*) as post_count, "
        "COALESCE(SUM(likes + comments + shares), 0) as total_interactions, "
        "COALESCE(SUM(views), 0) as total_views "
        "FROM posts "
        "WHERE 1=1 "
    );

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);

    if (startDate.isValid()) {
        sql += QStringLiteral("AND publish_date >= :start_date ");
        query.bindValue(QStringLiteral(":start_date"), startDate);
    }

    if (endDate.isValid()) {
        sql += QStringLiteral("AND publish_date <= :end_date ");
        query.bindValue(QStringLiteral(":end_date"), endDate);
    }

    sql += QStringLiteral("GROUP BY publish_date ORDER BY publish_date ASC");

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

QList<Post> AnalyticsService::getTopPosts(int limit)
{
    QList<Post> posts;

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        QStringLiteral(
            "SELECT "
            "post_id, platform, account_name, content, publish_date, "
            "likes, comments, shares, views "
            "FROM posts "
            "ORDER BY (likes + comments + shares) DESC, views DESC "
            "LIMIT :limit"
        )
    );
    query.bindValue(QStringLiteral(":limit"), limit);

    if (!query.exec()) {
        qDebug() << "Get top posts failed:" << query.lastError().text();
        return posts;
    }

    while (query.next()) {
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
        posts.append(post);
    }

    return posts;
}
