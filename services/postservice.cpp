#include "postservice.h"

PostService::PostService()
{
}

QList<Post> PostService::findPosts(const User& currentUser,
                                   const QString& platform,
                                   const QString& keyword)
{
    return postRepository.findPosts(
        buildQueryFilter(currentUser, platform, keyword)
        );
}

QList<Post> PostService::findPosts(const QString& platform,
                                   const QString& keyword)
{
    return postRepository.findPosts(platform.trimmed(), keyword.trimmed());
}

Post PostService::findPostById(int postId)
{
    return postRepository.findPostById(postId);
}

PostOperationResult PostService::addPost(const User& currentUser,
                                         const Post& post)
{
    if (!currentUser.isValid()) {
        return {
            false,
            QStringLiteral("Please login before adding posts.")
        };
    }

    QString message;

    if (!validatePost(post, message)) {
        return { false, message };
    }

    /*
     * 新增帖子时自动绑定当前用户。
     * View 不需要知道 createdByUserId，也不应该自己写归属信息。
     */
    const Post cleanPost = attachOwner(currentUser, normalizedPost(post));

    if (!postRepository.insertPost(cleanPost)) {
        return {
            false,
            QStringLiteral("Failed to add post. Please check the database.")
        };
    }

    return {
        true,
        QStringLiteral("Post added successfully.")
    };
}

PostOperationResult PostService::updatePost(const User& currentUser,
                                            const Post& post)
{
    if (post.postId <= 0) {
        return {
            false,
            QStringLiteral("Invalid post ID. Please double-click one row first.")
        };
    }

    QString message;

    if (!validatePost(post, message)) {
        return { false, message };
    }

    const Post existingPost = postRepository.findPostById(post.postId);

    if (!existingPost.isValid()) {
        return {
            false,
            QStringLiteral("The selected post no longer exists.")
        };
    }

    /*
     * 权限判断放在 Service：
     * - admin 可以修改所有帖子；
     * - 普通 user 只能修改自己的帖子。
     */
    if (!canManagePost(currentUser, existingPost)) {
        return {
            false,
            QStringLiteral("You can only update your own posts.")
        };
    }

    Post cleanPost = normalizedPost(post);
    cleanPost.postId = post.postId;

    /*
     * 修改内容时不改变归属人。
     * 数据是谁创建的，仍然属于谁。
     */
    cleanPost.createdByUserId = existingPost.createdByUserId;
    cleanPost.createdByUsername = existingPost.createdByUsername;

    if (!postRepository.updatePost(cleanPost)) {
        return {
            false,
            QStringLiteral("Failed to update post. The record may have been deleted.")
        };
    }

    return {
        true,
        QStringLiteral("Post updated successfully.")
    };
}

PostOperationResult PostService::deletePost(const User& currentUser,
                                            int postId)
{
    if (postId <= 0) {
        return {
            false,
            QStringLiteral("Invalid post ID. Please select one post record first.")
        };
    }

    const Post existingPost = postRepository.findPostById(postId);

    if (!existingPost.isValid()) {
        return {
            false,
            QStringLiteral("The selected post no longer exists.")
        };
    }

    /*
     * 删除权限同样在 Service 判断：
     * - admin 可以删除所有帖子；
     * - 普通 user 只能删除自己的帖子。
     */
    if (!canManagePost(currentUser, existingPost)) {
        return {
            false,
            QStringLiteral("You can only delete your own posts.")
        };
    }

    if (!postRepository.deletePostById(postId)) {
        return {
            false,
            QStringLiteral("Failed to delete post.")
        };
    }

    return {
        true,
        QStringLiteral("Post deleted successfully.")
    };
}

bool PostService::validatePost(const Post& post,
                               QString& message) const
{
    if (post.platform.trimmed().isEmpty()) {
        message = QStringLiteral("Platform cannot be empty.");
        return false;
    }

    if (post.accountName.trimmed().isEmpty()) {
        message = QStringLiteral("Account name cannot be empty.");
        return false;
    }

    if (post.content.trimmed().isEmpty()) {
        message = QStringLiteral("Content cannot be empty.");
        return false;
    }

    if (!post.publishDate.isValid()) {
        message = QStringLiteral("Publish date is invalid.");
        return false;
    }

    return true;
}

Post PostService::normalizedPost(const Post& post) const
{
    Post cleanPost = post;

    /*
     * 入库前统一 trim，避免用户输入前后空格后，
     * 查询、统计、导出时出现看起来相同但实际不同的数据。
     */
    cleanPost.platform = post.platform.trimmed();
    cleanPost.accountName = post.accountName.trimmed();
    cleanPost.content = post.content.trimmed();
    cleanPost.createdByUsername = post.createdByUsername.trimmed();

    return cleanPost;
}

Post PostService::attachOwner(const User& currentUser,
                              const Post& post) const
{
    Post cleanPost = post;

    cleanPost.createdByUserId = currentUser.userId;
    cleanPost.createdByUsername = currentUser.username.trimmed();

    return cleanPost;
}

PostQueryFilter PostService::buildQueryFilter(const User& currentUser,
                                              const QString& platform,
                                              const QString& keyword) const
{
    PostQueryFilter filter;

    filter.platform = platform.trimmed();
    filter.keyword = keyword.trimmed();

    /*
     * 数据隔离规则：
     * - admin 查询所有数据；
     * - 普通用户只查询自己的数据；
     * - 未登录状态不返回任何用户数据。
     */
    filter.includeAllUsers = currentUser.isValid() && currentUser.isAdmin();
    filter.ownerUserId = currentUser.isValid() ? currentUser.userId : -1;

    return filter;
}

bool PostService::canManagePost(const User& currentUser,
                                const Post& post) const
{
    if (!currentUser.isValid()) {
        return false;
    }

    if (currentUser.isAdmin()) {
        return true;
    }

    return post.createdByUserId == currentUser.userId;
}
