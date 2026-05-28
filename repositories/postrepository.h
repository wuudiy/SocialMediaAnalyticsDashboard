#ifndef POSTREPOSITORY_H
#define POSTREPOSITORY_H

#include "../models/post.h"

#include <QList>
#include <QString>

class QSqlQuery;

/*
 * posts 表的数据访问层。
 *
 * 负责：
 * - 新增帖子；
 * - 修改帖子；
 * - 删除帖子；
 * - 根据 ID 查询帖子；
 * - 查询帖子列表；
 * - 给 Dashboard 提供最近帖子数据。
 *
 * 不负责：
 * - 创建界面控件；
 * - 计算复杂统计指标；
 * - 弹窗提示用户。
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

    // 查询帖子。platform 和 keyword 为空时表示不过滤。
    QList<Post> findPosts(const QString& platform = QString(),
                          const QString& keyword = QString());

    // 查询最近发布的帖子，用于首页展示。
    QList<Post> findRecentPosts(int limit = 8);

private:
    // 把 SQL 当前行转成 Post 对象。
    Post buildPostFromQuery(const QSqlQuery& query) const;
};

#endif // POSTREPOSITORY_H
