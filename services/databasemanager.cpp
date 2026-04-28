#include "DatabaseManager.h"
#include "DESUtil.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVariant>
#include <QSqlDatabase>

/*
 * 静态数据库连接对象
 * ----------------
 * 整个程序共用这一条 MySQL 连接。
 */
QSqlDatabase DatabaseManager::db;

/*
 * 连接 MySQL 数据库
 * - hostName
 * - port
 * - databaseName
 * - userName
 * - password
 */
bool DatabaseManager::connectDatabase()
{
    qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

    if (QSqlDatabase::contains("mysql_connection")) {
        db = QSqlDatabase::database("mysql_connection");
    } else {
        db = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection");
    }

    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("social_media_system");
    db.setUserName("root");
    db.setPassword("123456");

    if (!db.open()) {
        qDebug() << "Failed to connect MySQL:" << db.lastError().text();
        return false;
    }

    qDebug() << "MySQL connected successfully.";
    return true;
}

/*
 * 创建 users 表
 * ----------------
 * username 和 password 都保存 DES 加密后的字符串。
 * 因为加密后使用 Base64 保存，所以 VARCHAR(255) 足够第一阶段使用。
 */
bool DatabaseManager::createTables()
{
    QSqlQuery query(db);

    QString sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "user_id INT PRIMARY KEY AUTO_INCREMENT,"
        "username VARCHAR(255) NOT NULL UNIQUE,"
        "password VARCHAR(255) NOT NULL,"
        "role VARCHAR(20) NOT NULL,"
        "status VARCHAR(20) NOT NULL"
        ")";

    if (!query.exec(sql)) {
        qDebug() << "Failed to create users table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Users table is ready.";
    return true;
}

/*
 * 插入初始管理员账号
 * 只有管理员可以注册新用户。
 * 所以程序第一次运行时必须先有一个管理员账号。
 *
 * 默认登录信息：
 * username: admin
 * password: 123456
 *
 * 但保存进数据库前，username 和 password 都会先 DES 加密。
 */
bool DatabaseManager::insertInitialAdmin()
{
    QSqlQuery checkQuery(db);

    checkQuery.prepare("SELECT COUNT(*) FROM users WHERE role = :role");
    checkQuery.bindValue(":role", "admin");

    if (!checkQuery.exec()) {
        qDebug() << "Failed to check admin:" << checkQuery.lastError().text();
        return false;
    }

    int adminCount = 0;

    if (checkQuery.next()) {
        adminCount = checkQuery.value(0).toInt();
    }

    // 已经存在管理员时，不重复插入
    if (adminCount > 0) {
        qDebug() << "Admin account already exists.";
        return true;
    }

    QString encryptedUsername = DESUtil::encrypt("admin");
    QString encryptedPassword = DESUtil::encrypt("123456");

    QSqlQuery insertQuery(db);

    insertQuery.prepare(
        "INSERT INTO users (username, password, role, status) "
        "VALUES (:username, :password, :role, :status)"
        );

    insertQuery.bindValue(":username", encryptedUsername);
    insertQuery.bindValue(":password", encryptedPassword);
    insertQuery.bindValue(":role", "admin");
    insertQuery.bindValue(":status", "active");

    if (!insertQuery.exec()) {
        qDebug() << "Failed to insert initial admin:" << insertQuery.lastError().text();
        return false;
    }

    qDebug() << "Initial admin account created.";
    qDebug() << "login username: admin";
    qDebug() << "login password: 123456";
    qDebug() << "encrypted username:" << encryptedUsername;
    qDebug() << "encrypted password:" << encryptedPassword;

    return true;
}

/*
 * 返回当前数据库连接
 */
QSqlDatabase DatabaseManager::database()
{
    return db;
}
