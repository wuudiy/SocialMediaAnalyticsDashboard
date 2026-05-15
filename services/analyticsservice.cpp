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
