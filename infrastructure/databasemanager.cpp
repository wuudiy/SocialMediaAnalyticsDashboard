#include "databasemanager.h"
#include "desutil.h"
#include "sha256util.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

QString DatabaseManager::s_lastError;

namespace
{
// 数据库配置集中放这里，后续如果改库名、账号、密码，只改这一处。
constexpr const char* kDatabaseHost = "127.0.0.1";
constexpr int kDatabasePort = 3306;
constexpr const char* kDatabaseName = "social_media_system";
constexpr const char* kDatabaseUser = "root";
constexpr const char* kDatabasePassword = "123456";

// 默认管理员账号。程序第一次启动时自动创建。
constexpr const char* kDefaultAdminUsername = "admin";
constexpr const char* kDefaultAdminPassword = "123456";
constexpr const char* kDefaultAdminRole = "admin";
constexpr const char* kDefaultAdminStatus = "active";
}

bool DatabaseManager::initialize()
{
    s_lastError.clear();

    if (!connectDatabase()) {
        return false;
    }

    if (!createUsersTable()) {
        return false;
    }

    if (!createPostsTable()) {
        return false;
    }

    /*
     * 兼容旧数据库。
     *
     * 如果 posts 表是旧版本创建的，里面没有数据归属字段，
     * 这里会自动补充 created_by_user_id 和 created_by_username。
     */
    if (!ensurePostOwnerColumns()) {
        return false;
    }

    if (!createOperationLogsTable()) {
        return false;
    }

    if (!createDefaultAdmin()) {
        return false;
    }

    return true;
}

QSqlDatabase DatabaseManager::database()
{
    return QSqlDatabase::database(connectionName());
}

void DatabaseManager::closeDatabase()
{
    const QString name = connectionName();

    if (!QSqlDatabase::contains(name)) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(name);

    if (db.isOpen()) {
        db.close();
    }
}

QString DatabaseManager::lastError()
{
    return s_lastError;
}

// 连接 MySQL。如果连接已经存在，就复用原来的连接。
bool DatabaseManager::connectDatabase()
{
    QSqlDatabase db;

    if (QSqlDatabase::contains(connectionName())) {
        db = QSqlDatabase::database(connectionName());
    } else {
        db = QSqlDatabase::addDatabase(
            QStringLiteral("QMYSQL"),
            connectionName()
            );
    }

    if (db.isOpen()) {
        return true;
    }

    db.setHostName(QString::fromUtf8(kDatabaseHost));
    db.setPort(kDatabasePort);
    db.setDatabaseName(QString::fromUtf8(kDatabaseName));
    db.setUserName(QString::fromUtf8(kDatabaseUser));
    db.setPassword(QString::fromUtf8(kDatabasePassword));

    if (!db.open()) {
        setLastError(db.lastError().text());

        qDebug().noquote() << QStringLiteral("Database connection failed: %1")
                                  .arg(db.lastError().text());

        return false;
    }

    return true;
}

// 创建 users 表。
bool DatabaseManager::createUsersTable()
{
    QSqlQuery query(database());

    const QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS users ("
        "    user_id INT PRIMARY KEY AUTO_INCREMENT,"
        "    username VARCHAR(255) NOT NULL UNIQUE,"
        "    password VARCHAR(255) NOT NULL,"
        "    role VARCHAR(20) NOT NULL DEFAULT 'user',"
        "    status VARCHAR(20) NOT NULL DEFAULT 'active',"
        "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")"
        );

    if (!query.exec(sql)) {
        setLastError(query.lastError().text());

        qDebug().noquote() << QStringLiteral("Create users table failed: %1")
                                  .arg(query.lastError().text());

        return false;
    }

    return true;
}

/*
 * 创建 posts 表。
 *
 * 新版本增加了数据归属字段：
 * - created_by_user_id：创建 / 导入数据的用户 ID；
 * - created_by_username：创建 / 导入数据的用户名。
 *
 * 注意：
 * 如果 posts 表已经存在，CREATE TABLE IF NOT EXISTS 不会改变旧表结构。
 * 所以 initialize() 里还会调用 ensurePostOwnerColumns() 做旧表升级。
 */
bool DatabaseManager::createPostsTable()
{
    QSqlQuery query(database());

    const QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS posts ("
        "    post_id INT PRIMARY KEY AUTO_INCREMENT,"
        "    platform VARCHAR(50) NOT NULL,"
        "    account_name VARCHAR(100) NOT NULL,"
        "    content TEXT NOT NULL,"
        "    publish_date DATE NOT NULL,"
        "    likes INT NOT NULL DEFAULT 0,"
        "    comments INT NOT NULL DEFAULT 0,"
        "    shares INT NOT NULL DEFAULT 0,"
        "    views INT NOT NULL DEFAULT 0,"
        "    created_by_user_id INT NULL,"
        "    created_by_username VARCHAR(255),"
        "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "    INDEX idx_posts_platform (platform),"
        "    INDEX idx_posts_publish_date (publish_date),"
        "    INDEX idx_posts_created_by_user_id (created_by_user_id)"
        ")"
        );

    if (!query.exec(sql)) {
        setLastError(query.lastError().text());

        qDebug().noquote() << QStringLiteral("Create posts table failed: %1")
                                  .arg(query.lastError().text());

        return false;
    }

    return true;
}

// 创建 operation_logs 表。
// 这个表只记录关键操作，不做复杂日志级别，符合期末项目的简化要求。
bool DatabaseManager::createOperationLogsTable()
{
    QSqlQuery query(database());

    const QString sql = QStringLiteral(
        "CREATE TABLE IF NOT EXISTS operation_logs ("
        "    log_id INT PRIMARY KEY AUTO_INCREMENT,"
        "    user_id INT NULL,"
        "    username VARCHAR(255) NOT NULL,"
        "    action VARCHAR(50) NOT NULL,"
        "    detail TEXT,"
        "    result VARCHAR(20) NOT NULL DEFAULT 'success',"
        "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "    INDEX idx_operation_logs_username (username),"
        "    INDEX idx_operation_logs_action (action),"
        "    INDEX idx_operation_logs_created_at (created_at)"
        ")"
        );

    if (!query.exec(sql)) {
        setLastError(query.lastError().text());

        qDebug().noquote() << QStringLiteral("Create operation_logs table failed: %1")
                                  .arg(query.lastError().text());

        return false;
    }

    return true;
}

// 创建默认管理员。如果 admin 已经存在，就不重复插入。
bool DatabaseManager::createDefaultAdmin()
{
    const QString encryptedUsername = DESUtil::encrypt(
        QString::fromUtf8(kDefaultAdminUsername)
        );

    const QString passwordHash = SHA256Util::hashPassword(
        QString::fromUtf8(kDefaultAdminPassword)
        );

    QSqlQuery checkQuery(database());

    checkQuery.prepare(
        QStringLiteral(
            "SELECT 1 "
            "FROM users "
            "WHERE username = :username "
            "LIMIT 1"
            )
        );

    checkQuery.bindValue(QStringLiteral(":username"), encryptedUsername);

    if (!checkQuery.exec()) {
        setLastError(checkQuery.lastError().text());

        qDebug().noquote() << QStringLiteral("Check default admin failed: %1")
                                  .arg(checkQuery.lastError().text());

        return false;
    }

    if (checkQuery.next()) {
        return true;
    }

    QSqlQuery insertQuery(database());

    insertQuery.prepare(
        QStringLiteral(
            "INSERT INTO users (username, password, role, status) "
            "VALUES (:username, :password, :role, :status)"
            )
        );

    insertQuery.bindValue(QStringLiteral(":username"), encryptedUsername);
    insertQuery.bindValue(QStringLiteral(":password"), passwordHash);
    insertQuery.bindValue(QStringLiteral(":role"), QString::fromUtf8(kDefaultAdminRole));
    insertQuery.bindValue(QStringLiteral(":status"), QString::fromUtf8(kDefaultAdminStatus));

    if (!insertQuery.exec()) {
        setLastError(insertQuery.lastError().text());

        qDebug().noquote() << QStringLiteral("Insert default admin failed: %1")
                                  .arg(insertQuery.lastError().text());

        return false;
    }

    return true;
}

/*
 * 旧数据库升级：补充 posts 表的数据归属字段。
 *
 * 旧数据不会强行分配给某个用户：
 * - created_by_user_id 默认为 NULL；
 * - admin 可以看到这些旧数据；
 * - 普通 user 看不到这些旧数据。
 */
bool DatabaseManager::ensurePostOwnerColumns()
{
    QSqlQuery query(database());

    if (!columnExists(QStringLiteral("posts"), QStringLiteral("created_by_user_id"))) {
        if (!query.exec(QStringLiteral(
                "ALTER TABLE posts "
                "ADD COLUMN created_by_user_id INT NULL"
                ))) {
            setLastError(query.lastError().text());

            qDebug().noquote() << QStringLiteral("Add posts.created_by_user_id failed: %1")
                                      .arg(query.lastError().text());

            return false;
        }
    }

    if (!columnExists(QStringLiteral("posts"), QStringLiteral("created_by_username"))) {
        if (!query.exec(QStringLiteral(
                "ALTER TABLE posts "
                "ADD COLUMN created_by_username VARCHAR(255)"
                ))) {
            setLastError(query.lastError().text());

            qDebug().noquote() << QStringLiteral("Add posts.created_by_username failed: %1")
                                      .arg(query.lastError().text());

            return false;
        }
    }

    return true;
}

/*
 * 判断某个表是否存在指定字段。
 *
 * 使用 INFORMATION_SCHEMA.COLUMNS 查询当前数据库结构。
 */
bool DatabaseManager::columnExists(const QString& tableName,
                                   const QString& columnName)
{
    QSqlQuery query(database());

    query.prepare(
        QStringLiteral(
            "SELECT COUNT(*) "
            "FROM INFORMATION_SCHEMA.COLUMNS "
            "WHERE TABLE_SCHEMA = DATABASE() "
            "AND TABLE_NAME = :table_name "
            "AND COLUMN_NAME = :column_name"
            )
        );

    query.bindValue(QStringLiteral(":table_name"), tableName);
    query.bindValue(QStringLiteral(":column_name"), columnName);

    if (!query.exec()) {
        setLastError(query.lastError().text());

        qDebug().noquote() << QStringLiteral("Check column exists failed: %1")
                                  .arg(query.lastError().text());

        return false;
    }

    if (!query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

QString DatabaseManager::connectionName()
{
    return QStringLiteral("main_database_connection");
}

void DatabaseManager::setLastError(const QString& error)
{
    s_lastError = error;
}
