#ifndef POSTREPOSITORY_H
#define POSTREPOSITORY_H

#include "../models/post.h"

#include <QList>
#include <QString>

class QSqlQuery;

/*
 * 帖子查询条件。
 *
 * includeAllUsers = true：
 * - 管理员查询全部帖子；
 * - 不追加 created_by_user_id 条件。
 *
 * includeAllUsers = false：
 * - 普通用户只查询 ownerUserId 对应的帖子；
 * - 会追加 created_by_user_id = :owner_user_id。
 */
struct PostQueryFilter
{
    QString platform;
    QString keyword;
    int ownerUserId = -1;
    bool includeAllUsers = true;
};

/*
 * posts 表的数据访问层。
 *
 * 负责：
 * - 新增帖子；
 * - 修改帖子；
 * - 删除帖子；
 * - 根据 ID 查询帖子；
 * - 查询帖子列表；
 * - 给 Dashboard / Analytics 提供最近帖子数据。
 *
 * 不负责：
 * - 当前登录用户是谁；
 * - 用户有没有权限；
 * - 页面提示；
 * - 操作日志。
 */
class PostRepository
{
public:
    PostRepository();

    // 插入一条帖子数据。
    bool insertPost(const Post& post);

    // 修改一条帖子数据。post.postId 必须是有效 ID。
    bool updatePost(const Post& post);

    // 根据帖子 ID 删除数据。
    bool deletePostById(int postId);

    // 根据帖子 ID 查询一条数据。查不到时返回默认无效 Post。
    Post findPostById(int postId);

    // 按筛选条件查询帖子，支持用户级数据隔离。
    QList<Post> findPosts(const PostQueryFilter& filter);

    // 兼容旧调用。默认查询全部数据。
    QList<Post> findPosts(const QString& platform = QString(),
                          const QString& keyword = QString());

    // 查询最近发布的帖子，用于旧统计逻辑兼容。当前仍默认查询全部数据。
    QList<Post> findRecentPosts(int limit = 8);

private:
    // 把 SQL 当前行转成 Post 对象。
    Post buildPostFromQuery(const QSqlQuery& query) const;
};

#endif // POSTREPOSITORY_H
