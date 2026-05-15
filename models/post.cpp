#include "post.h"

Post::Post()
    : postId(-1),
    platform(""),
    accountName(""),
    content(""),
    publishDate(QDate::currentDate()),
    likes(0),
    comments(0),
    shares(0),
    views(0)
{
}

bool Post::isValid() const
{
    return postId != -1;
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
