#include "postrepository.h"

#include "../infrastructure/databasemanager.h"

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
        QStringLiteral(
            "INSERT INTO posts "
            "(platform, account_name, content, publish_date, "
            "likes, comments, shares, views, "
            "created_by_user_id, created_by_username) "
            "VALUES "
            "(:platform, :account_name, :content, :publish_date, "
            ":likes, :comments, :shares, :views, "
            ":created_by_user_id, :created_by_username)"
            )
        );

    query.bindValue(QStringLiteral(":platform"), post.platform.trimmed());
    query.bindValue(QStringLiteral(":account_name"), post.accountName.trimmed());
    query.bindValue(QStringLiteral(":content"), post.content.trimmed());
    query.bindValue(QStringLiteral(":publish_date"), post.publishDate.toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":likes"), post.likes);
    query.bindValue(QStringLiteral(":comments"), post.comments);
    query.bindValue(QStringLiteral(":shares"), post.shares);
    query.bindValue(QStringLiteral(":views"), post.views);

    /*
     * createdByUserId <= 0 时写 NULL。
     * 正常新增和 CSV 导入都会由 PostService 自动补当前用户 ID。
     */
    if (post.createdByUserId > 0) {
        query.bindValue(QStringLiteral(":created_by_user_id"), post.createdByUserId);
    } else {
        query.bindValue(QStringLiteral(":created_by_user_id"), QVariant());
    }

    query.bindValue(QStringLiteral(":created_by_username"), post.createdByUsername.trimmed());

    if (!query.exec()) {
        qDebug() << "Insert post failed:" << query.lastError().text();
        return false;
    }

    return true;
}

bool PostRepository::updatePost(const Post& post)
{
    if (post.postId <= 0) {
        qDebug() << "Update post failed: invalid post id.";
        return false;
    }

    QSqlQuery query(DatabaseManager::database());

    /*
     * 修改帖子内容时不修改创建人。
     * 帖子归属只在新增 / 导入时确定。
     */
    query.prepare(
        QStringLiteral(
            "UPDATE posts SET "
            "platform = :platform, "
            "account_name = :account_name, "
            "content = :content, "
            "publish_date = :publish_date, "
            "likes = :likes, "
            "comments = :comments, "
            "shares = :shares, "
            "views = :views "
            "WHERE post_id = :post_id"
            )
        );

    query.bindValue(QStringLiteral(":platform"), post.platform.trimmed());
    query.bindValue(QStringLiteral(":account_name"), post.accountName.trimmed());
    query.bindValue(QStringLiteral(":content"), post.content.trimmed());
    query.bindValue(QStringLiteral(":publish_date"), post.publishDate.toString(Qt::ISODate));
    query.bindValue(QStringLiteral(":likes"), post.likes);
    query.bindValue(QStringLiteral(":comments"), post.comments);
    query.bindValue(QStringLiteral(":shares"), post.shares);
    query.bindValue(QStringLiteral(":views"), post.views);
    query.bindValue(QStringLiteral(":post_id"), post.postId);

    if (!query.exec()) {
        qDebug() << "Update post failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool PostRepository::deletePostById(int postId)
{
    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        QStringLiteral(
            "DELETE FROM posts "
            "WHERE post_id = :post_id"
            )
        );

    query.bindValue(QStringLiteral(":post_id"), postId);

    if (!query.exec()) {
        qDebug() << "Delete post failed:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

Post PostRepository::findPostById(int postId)
{
    Post post;

    if (postId <= 0) {
        return post;
    }

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        QStringLiteral(
            "SELECT post_id, platform, account_name, content, publish_date, "
            "likes, comments, shares, views, "
            "created_by_user_id, created_by_username "
            "FROM posts "
            "WHERE post_id = :post_id "
            "LIMIT 1"
            )
        );

    query.bindValue(QStringLiteral(":post_id"), postId);

    if (!query.exec()) {
        qDebug() << "Find post by id failed:" << query.lastError().text();
        return post;
    }

    if (query.next()) {
        post = buildPostFromQuery(query);
    }

    return post;
}

QList<Post> PostRepository::findPosts(const PostQueryFilter& filter)
{
    QList<Post> posts;

    QString sql =
        "SELECT post_id, platform, account_name, content, publish_date, "
        "likes, comments, shares, views, "
        "created_by_user_id, created_by_username "
        "FROM posts "
        "WHERE 1 = 1 ";

    const QString cleanPlatform = filter.platform.trimmed();
    const QString cleanKeyword = filter.keyword.trimmed();

    if (!cleanPlatform.isEmpty()) {
        sql += "AND platform = :platform ";
    }

    if (!cleanKeyword.isEmpty()) {
        sql += "AND (content LIKE :keyword OR account_name LIKE :keyword) ";
    }

    /*
     * 用户级数据隔离核心条件：
     * - admin：includeAllUsers = true，不加用户过滤；
     * - 普通 user：includeAllUsers = false，只查自己的数据。
     */
    if (!filter.includeAllUsers) {
        sql += "AND created_by_user_id = :owner_user_id ";
    }

    sql += "ORDER BY publish_date DESC, post_id DESC";

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);

    if (!cleanPlatform.isEmpty()) {
        query.bindValue(QStringLiteral(":platform"), cleanPlatform);
    }

    if (!cleanKeyword.isEmpty()) {
        query.bindValue(QStringLiteral(":keyword"),
                        QStringLiteral("%%1%").arg(cleanKeyword));
    }

    if (!filter.includeAllUsers) {
        query.bindValue(QStringLiteral(":owner_user_id"), filter.ownerUserId);
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

QList<Post> PostRepository::findPosts(const QString& platform,
                                      const QString& keyword)
{
    PostQueryFilter filter;
    filter.platform = platform;
    filter.keyword = keyword;
    filter.includeAllUsers = true;

    return findPosts(filter);
}

QList<Post> PostRepository::findRecentPosts(int limit)
{
    QList<Post> posts;

    QSqlQuery query(DatabaseManager::database());

    const QString sql = QStringLiteral(
                            "SELECT post_id, platform, account_name, content, publish_date, "
                            "likes, comments, shares, views, "
                            "created_by_user_id, created_by_username "
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

    post.createdByUserId = query.value(9).isNull()
                               ? -1
                               : query.value(9).toInt();

    post.createdByUsername = query.value(10).toString();

    return post;
}
