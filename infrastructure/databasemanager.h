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
 * - 初始化 operation_logs 表；
 * - 创建默认管理员；
 * - 兼容旧数据库结构升级；
 * - 提供全局数据库连接；
 * - 保存最近一次数据库错误。
 *
 * 不负责：
 * - 登录校验；
 * - 用户注册业务；
 * - 页面显示逻辑；
 * - 具体业务数据写入。
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

    // 创建 operation_logs 表，用来保存用户关键操作记录。
    static bool createOperationLogsTable();

    // 创建默认管理员账号。
    static bool createDefaultAdmin();

    /*
     * 数据库升级：为 posts 表补充数据归属字段。
     *
     * 说明：
     * 如果用户本地已经存在旧 posts 表，CREATE TABLE IF NOT EXISTS 不会修改旧表结构。
     * 所以这里额外检查字段是否存在，不存在就 ALTER TABLE 补上。
     */
    static bool ensurePostOwnerColumns();

    // 判断某个表是否已经存在指定字段。
    static bool columnExists(const QString& tableName,
                             const QString& columnName);

    // 数据库连接名统一收口。
    static QString connectionName();

    // 保存最近一次错误。
    static void setLastError(const QString& error);

private:
    static QString s_lastError;
};

#endif // DATABASEMANAGER_H
