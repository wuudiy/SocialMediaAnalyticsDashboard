#include "post.h"

Post::Post()
    : postId(-1),
    likes(0),
    comments(0),
    shares(0),
    views(0),
    createdByUserId(-1)
{
}

// postId 由数据库自增生成，-1 统一表示无效帖子。
bool Post::isValid() const
{
    return postId > 0;
}

int Post::interactionCount() const
{
    return likes + comments + shares;
}

double Post::engagementRate() const
{
    if (views <= 0) {
        return 0.0;
    }

    return static_cast<double>(interactionCount()) / static_cast<double>(views);
}
