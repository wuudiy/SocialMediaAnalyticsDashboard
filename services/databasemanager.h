#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

/*
 * 数据库管理类。
 *
 * 负责：
 * - 连接 MySQL；
 * - 初始化 users 表；
 * - 初始化 posts 表；
 * - 创建默认管理员；
 * - 提供全局数据库连接；
 * - 保存最近一次数据库错误。
 *
 * 不负责：
 * - 登录校验；
 * - 用户注册业务；
 * - 页面显示逻辑。
 */
class DatabaseManager
{
public:
    // 程序启动时调用一次，完成数据库连接和基础表初始化。
    static bool initialize();

    // 获取项目主数据库连接。
    static QSqlDatabase database();

    // 程序退出时关闭数据库连接。
    static void closeDatabase();

    // 获取最近一次数据库错误信息。
    static QString lastError();

private:
    // 连接 MySQL。
    static bool connectDatabase();

    // 创建 users 表。
    static bool createUsersTable();

    // 创建 posts 表，用来保存社交媒体帖子数据。
    static bool createPostsTable();

    // 创建默认管理员账号。
    static bool createDefaultAdmin();

    // 数据库连接名统一收口。
    static QString connectionName();

    // 保存最近一次错误。
    static void setLastError(const QString& error);

private:
    static QString s_lastError;
};

#endif // DATABASEMANAGER_H
