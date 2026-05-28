#ifndef POSTSERVICE_H
#define POSTSERVICE_H

#include "../models/post.h"
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
 * 负责：
 * - 表单数据校验；
 * - 新增帖子；
 * - 修改帖子；
 * - 删除帖子；
 * - 查询帖子。
 *
 * 不负责：
 * - 弹窗；
 * - 表格渲染；
 * - 按钮状态；
 * - 写操作日志。
 *
 * Repository 只处理 SQL；
 * Service 处理业务规则；
 * Controller 负责协调页面和日志。
 */
class PostService
{
public:
    PostService();

    QList<Post> findPosts(const QString& platform = QString(),
                          const QString& keyword = QString());

    Post findPostById(int postId);

    PostOperationResult addPost(const Post& post);

    PostOperationResult updatePost(const Post& post);

    PostOperationResult deletePost(int postId);

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

private:
    PostRepository postRepository;
};

#endif // POSTSERVICE_H
