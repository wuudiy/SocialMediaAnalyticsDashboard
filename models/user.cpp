#include "user.h"

User::User()
    : userId(-1),
    username(""),
    password(""),
    role(""),
    status("")
{
}

// userId 由数据库自增生成，-1 统一表示无效用户。
bool User::isValid() const
{
    return userId != -1;
}

// 角色判断统一放在模型里，避免业务代码到处比较字符串。
bool User::isAdmin() const
{
    return role.trimmed().toLower() == QStringLiteral("admin");
}

// 状态判断统一放在模型里，后续状态值变化时只改这里。
bool User::isActive() const
{
    return status.trimmed().toLower() == QStringLiteral("active");
}
