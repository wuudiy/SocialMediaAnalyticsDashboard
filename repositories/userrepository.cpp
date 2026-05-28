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

// 查询全部用户。用户列表页面需要显示明文用户名。
QList<User> UserRepository::findAllUsers()
{
    QList<User> users;

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT user_id, username, password, role, status "
        "FROM users "
        "ORDER BY "
        "CASE WHEN role = 'admin' THEN 0 ELSE 1 END, "
        "user_id ASC"
        );

    if (!query.exec()) {
        qDebug() << "Find all users failed:" << query.lastError().text();
        return users;
    }

    while (query.next()) {
        User user = buildUserFromQuery(query);
        user.username = decryptUsername(user.username);
        users.append(user);
    }

    return users;
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

// 插入新用户。用户名在 Repository 内部加密。
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
    query.bindValue(":role", role.trimmed().toLower());
    query.bindValue(":status", status.trimmed().toLower());

    if (!query.exec()) {
        qDebug() << "Insert user failed:" << query.lastError().text();
        return false;
    }

    return true;
}

// 更新用户密码。新密码应该已经是 SHA-256 哈希。
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

// 启用或禁用用户，只允许写入固定状态值。
bool UserRepository::updateStatusByUserId(int userId,
                                          const QString& status)
{
    const QString normalizedStatus = status.trimmed().toLower();

    if (normalizedStatus != QStringLiteral("active")
        && normalizedStatus != QStringLiteral("disabled")) {
        qDebug() << "Update user status failed: invalid status" << status;
        return false;
    }

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "UPDATE users "
        "SET status = :status "
        "WHERE user_id = :user_id"
        );

    query.bindValue(":status", normalizedStatus);
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qDebug() << "Update user status failed:" << query.lastError().text();
        return false;
    }

    return true;
}

// 禁用管理员前使用，保证系统至少保留一个可用管理员。
int UserRepository::countActiveAdmins()
{
    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT COUNT(*) "
        "FROM users "
        "WHERE role = 'admin' "
        "AND status = 'active'"
        );

    if (!query.exec()) {
        qDebug() << "Count active admins failed:" << query.lastError().text();
        return 0;
    }

    if (!query.next()) {
        return 0;
    }

    return query.value(0).toInt();
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

QString UserRepository::encryptUsername(const QString& username) const
{
    return DESUtil::encrypt(username.trimmed());
}

QString UserRepository::decryptUsername(const QString& encryptedUsername) const
{
    return DESUtil::decrypt(encryptedUsername.trimmed());
}
