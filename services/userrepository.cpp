#include "userrepository.h"
#include "databasemanager.h"
#include "desutil.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

UserRepository::UserRepository()
{
}

// 根据用户名查询用户。数据库中的 username 存的是 DES 密文。
User UserRepository::findByUsername(const QString& username)
{
    User user;

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT user_id, username, password, role, status "
        "FROM users "
        "WHERE username = :username"
        );

    query.bindValue(":username", encryptUsername(username));

    if (!query.exec()) {
        qDebug() << "Find user failed:" << query.lastError().text();
        return user;
    }

    if (query.next()) {
        user = buildUserFromQuery(query);
    }

    return user;
}

// 判断用户名是否存在。这里只查 SELECT 1，避免取不需要的字段。
bool UserRepository::usernameExists(const QString& username)
{
    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT 1 "
        "FROM users "
        "WHERE username = :username "
        "LIMIT 1"
        );

    query.bindValue(":username", encryptUsername(username));

    if (!query.exec()) {
        qDebug() << "Check username failed:" << query.lastError().text();
        return false;
    }

    return query.next();
}

// 插入新用户。用户名在 Repository 内部加密，调用方不用关心数据库存储细节。
bool UserRepository::insertUser(const QString& username,
                                const QString& passwordHash,
                                const QString& role,
                                const QString& status)
{
    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "INSERT INTO users (username, password, role, status) "
        "VALUES (:username, :password, :role, :status)"
        );

    query.bindValue(":username", encryptUsername(username));
    query.bindValue(":password", passwordHash);
    query.bindValue(":role", role);
    query.bindValue(":status", status);

    if (!query.exec()) {
        qDebug() << "Insert user failed:" << query.lastError().text();
        return false;
    }

    return true;
}

// 更新用户密码。旧 DES 密码登录成功后会被替换成 SHA-256 哈希。
bool UserRepository::updatePasswordByUserId(int userId,
                                            const QString& passwordHash)
{
    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "UPDATE users "
        "SET password = :password "
        "WHERE user_id = :user_id"
        );

    query.bindValue(":password", passwordHash);
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Update password failed:" << query.lastError().text();
        return false;
    }

    return true;
}

// 将查询结果转成 User。字段顺序必须和 SELECT 语句保持一致。
User UserRepository::buildUserFromQuery(const QSqlQuery& query) const
{
    User user;

    user.userId = query.value(0).toInt();
    user.username = query.value(1).toString();
    user.password = query.value(2).toString();
    user.role = query.value(3).toString();
    user.status = query.value(4).toString();

    return user;
}

// 统一处理用户名加密，确保查询和插入使用同一套规则。
QString UserRepository::encryptUsername(const QString& username) const
{
    return DESUtil::encrypt(username.trimmed());
}
