#include "postrepository.h"
#include "databasemanager.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QtGlobal>

PostRepository::PostRepository()
{
}

bool PostRepository::insertPost(const Post& post)
{
    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "INSERT INTO posts "
        "(platform, account_name, content, publish_date, likes, comments, shares, views) "
        "VALUES "
        "(:platform, :account_name, :content, :publish_date, :likes, :comments, :shares, :views)"
        );

    query.bindValue(":platform", post.platform.trimmed());
    query.bindValue(":account_name", post.accountName.trimmed());
    query.bindValue(":content", post.content.trimmed());
    query.bindValue(":publish_date", post.publishDate.toString(Qt::ISODate));
    query.bindValue(":likes", post.likes);
    query.bindValue(":comments", post.comments);
    query.bindValue(":shares", post.shares);
    query.bindValue(":views", post.views);

    if (!query.exec()) {
        qDebug() << "Insert post failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool PostRepository::deletePostById(int postId)
{
    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "DELETE FROM posts "
        "WHERE post_id = :post_id"
        );

    query.bindValue(":post_id", postId);

    if (!query.exec()) {
        qDebug() << "Delete post failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Post> PostRepository::findPosts(const QString& platform,
                                      const QString& keyword)
{
    QList<Post> posts;

    QString sql =
        "SELECT post_id, platform, account_name, content, publish_date, "
        "likes, comments, shares, views "
        "FROM posts "
        "WHERE 1 = 1 ";

    if (!platform.trimmed().isEmpty()) {
        sql += "AND platform = :platform ";
    }

    if (!keyword.trimmed().isEmpty()) {
        sql += "AND (content LIKE :keyword OR account_name LIKE :keyword) ";
    }

    sql += "ORDER BY publish_date DESC, post_id DESC";

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);

    if (!platform.trimmed().isEmpty()) {
        query.bindValue(":platform", platform.trimmed());
    }

    if (!keyword.trimmed().isEmpty()) {
        query.bindValue(":keyword", QStringLiteral("%%1%").arg(keyword.trimmed()));
    }

    if (!query.exec()) {
        qDebug() << "Find posts failed:" << query.lastError().text();
        return posts;
    }

    while (query.next()) {
        posts.append(buildPostFromQuery(query));
    }

    return posts;
}

QList<Post> PostRepository::findRecentPosts(int limit)
{
    QList<Post> posts;

    QSqlQuery query(DatabaseManager::database());

    const QString sql = QStringLiteral(
                            "SELECT post_id, platform, account_name, content, publish_date, "
                            "likes, comments, shares, views "
                            "FROM posts "
                            "ORDER BY publish_date DESC, post_id DESC "
                            "LIMIT %1"
                            ).arg(qMax(1, limit));

    if (!query.exec(sql)) {
        qDebug() << "Find recent posts failed:" << query.lastError().text();
        return posts;
    }

    while (query.next()) {
        posts.append(buildPostFromQuery(query));
    }

    return posts;
}

Post PostRepository::buildPostFromQuery(const QSqlQuery& query) const
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

    return post;
}
