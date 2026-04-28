#include "user.h"

/*
 * 默认构造函数
 * ----------------
 * userId 设置为 -1，表示“无效用户”或“未查询到用户”。
 * 后面 AuthController 可以通过 userId == -1 判断用户是否存在。
 */

User::User()
    : userId(-1),
    username(""),
    password(""),
    role(""),
    status("")
{}
