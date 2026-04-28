#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>

/*
 * Service 层：DatabaseManager
 * ---------------------------
 * 作用：
 * 1. 负责连接 MySQL 数据库；
 * 2. 负责创建 users 表；
 * 3. 负责插入系统初始管理员账号。
 * 注意：
 * 它只管理数据库连接和初始化，不负责具体用户查询。
 * 用户查询交给 UserRepository。
 */
class DatabaseManager
{
public:
    // 连接 MySQL 数据库
    static bool connectDatabase();

    // 创建项目需要的数据库表
    static bool createTables();

    // 如果数据库中没有管理员，则插入默认管理员 admin / 123456
    static bool insertInitialAdmin();

    // 获取数据库连接，供 Repository 使用
    static QSqlDatabase database();

private:
    static QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
