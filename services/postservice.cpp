#include "postservice.h"

PostService::PostService()
{
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

PostOperationResult PostService::addPost(const Post& post)
{
    QString message;

    if (!validatePost(post, message)) {
        return { false, message };
    }

    const Post cleanPost = normalizedPost(post);

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

PostOperationResult PostService::updatePost(const Post& post)
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

    Post cleanPost = normalizedPost(post);
    cleanPost.postId = post.postId;

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

PostOperationResult PostService::deletePost(int postId)
{
    if (postId <= 0) {
        return {
            false,
            QStringLiteral("Invalid post ID. Please select one post record first.")
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

    return cleanPost;
}
