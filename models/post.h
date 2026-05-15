#ifndef POST_H
#define POST_H

#include <QDate>
#include <QString>

/*
 * 社交媒体帖子数据模型。
 *
 * 这个类只保存一条帖子的基础数据。
 * 它不负责操作数据库，也不负责页面显示。
 */
class Post
{
public:
    Post();

    // postId 为 -1，说明这条数据还没有保存到数据库。
    bool isValid() const;

    // 点赞、评论、转发加起来就是互动量。
    int interactionCount() const;

    // 互动率 = 互动量 / 浏览量。
    // 浏览量为 0 时直接返回 0，避免除 0。
    double engagementRate() const;

public:
    int postId;              // 帖子 ID，对应 posts.post_id
    QString platform;        // 平台名称，例如 Weibo、Douyin、Bilibili
    QString accountName;     // 账号名称
    QString content;         // 帖子标题或内容摘要
    QDate publishDate;       // 发布时间
    int likes;               // 点赞数
    int comments;            // 评论数
    int shares;              // 转发数
    int views;               // 浏览量 / 曝光量
};

#endif // POST_H
