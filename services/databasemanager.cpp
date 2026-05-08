#include "databasemanager.h"
#include "desutil.h"

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

// 默认管理员账号。程序首次启动时自动创建。
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

// 连接 MySQL。如果连接已经存在，则复用同一个连接。
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

// 创建 users 表。这里保持原有表结构，只统一 SQL 风格和错误处理。
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

// 创建默认管理员。如果 admin 已存在，则不重复插入。
bool DatabaseManager::createDefaultAdmin()
{
    const QString encryptedUsername = DESUtil::encrypt(
        QString::fromUtf8(kDefaultAdminUsername)
        );

    const QString encryptedPassword = DESUtil::encrypt(
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
    insertQuery.bindValue(QStringLiteral(":password"), encryptedPassword);
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

QString DatabaseManager::connectionName()
{
    return QStringLiteral("main_database_connection");
}

void DatabaseManager::setLastError(const QString& error)
{
    s_lastError = error;
}
