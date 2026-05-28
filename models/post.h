#ifndef POST_H
#define POST_H

#include <QDate>
#include <QString>

/*
 * 社交媒体帖子模型。
 *
 * 数据隔离改造后，每条帖子都记录创建者：
 * - createdByUserId：创建 / 导入这条帖子数据的用户 ID；
 * - createdByUsername：创建 / 导入这条帖子数据的用户名。
 *
 * 权限规则：
 * - admin 可以查看全部帖子；
 * - 普通 user 只能查看 createdByUserId 等于自己 userId 的帖子；
 * - 旧数据 createdByUserId 为 -1，只给 admin 查看。
 */
class Post
{
public:
    Post();

    // postId 大于 0 表示这是一条数据库中真实存在的帖子。
    bool isValid() const;

    // 互动量 = 点赞 + 评论 + 分享。
    int interactionCount() const;

    // 互动率 = 互动量 / 浏览量。
    double engagementRate() const;

public:
    int postId;
    QString platform;
    QString accountName;
    QString content;
    QDate publishDate;
    int likes;
    int comments;
    int shares;
    int views;

    int createdByUserId;
    QString createdByUsername;
};

#endif // POST_H
