#ifndef POSTSERVICE_H
#define POSTSERVICE_H

#include "../models/post.h"
#include "../models/user.h"
#include "../repositories/postrepository.h"

#include <QList>
#include <QString>

/*
 * 帖子操作结果。
 *
 * Controller 调用 Service 后，通过这个结构体判断：
 * - 是否成功；
 * - 失败时给页面显示什么提示；
 * - 成功时是否需要刷新表格。
 */
struct PostOperationResult
{
    bool success = false;
    QString message;
};

/*
 * 帖子业务服务。
 *
 * 数据隔离改造后，本类负责：
 * - 根据当前用户构造查询权限；
 * - 普通用户只能查询自己的帖子；
 * - 普通用户只能修改 / 删除自己的帖子；
 * - 管理员可以管理全部帖子；
 * - 新增帖子时自动写入创建人信息。
 *
 * 不负责：
 * - 弹窗；
 * - 表格渲染；
 * - 按钮状态；
 * - 写操作日志。
 */
class PostService
{
public:
    PostService();

    QList<Post> findPosts(const User& currentUser,
                          const QString& platform = QString(),
                          const QString& keyword = QString());

    // 兼容旧调用。默认查询全部数据。
    QList<Post> findPosts(const QString& platform = QString(),
                          const QString& keyword = QString());

    Post findPostById(int postId);

    PostOperationResult addPost(const User& currentUser,
                                const Post& post);

    PostOperationResult updatePost(const User& currentUser,
                                   const Post& post);

    PostOperationResult deletePost(const User& currentUser,
                                   int postId);

    /*
     * 校验帖子输入。
     *
     * 这个函数从 View 中迁移出来。
     * 后续无论是手动新增，还是 CSV 导入，都可以复用同一套校验规则。
     */
    bool validatePost(const Post& post,
                      QString& message) const;

private:
    Post normalizedPost(const Post& post) const;

    Post attachOwner(const User& currentUser,
                     const Post& post) const;

    PostQueryFilter buildQueryFilter(const User& currentUser,
                                     const QString& platform,
                                     const QString& keyword) const;

    bool canManagePost(const User& currentUser,
                       const Post& post) const;

private:
    PostRepository postRepository;
};

#endif // POSTSERVICE_H
