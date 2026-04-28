#include "UserRepository.h"
#include "DatabaseManager.h"
#include "DESUtil.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

UserRepository::UserRepository()
{
}

/*
 * 根据用户名查找用户
 * ----------------
 * 注意：
 * 数据库中的 username 是 DES 加密后的密文。
 * 所以不能直接用明文 username 查询。
 *
 * 查询流程：
 * 1. 将用户输入的 username 加密；
 * 2. 使用加密后的 username 去数据库查询；
 * 3. 如果查到，则返回 User 对象；
 * 4. 如果没查到，则返回 userId = -1 的默认 User。
 */
User UserRepository::findByUsername(const QString& username)
{
    User user;

    QString encryptedUsername = DESUtil::encrypt(username);

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "SELECT user_id, username, password, role, status "
        "FROM users "
        "WHERE username = :username"
        );

    query.bindValue(":username", encryptedUsername);

    if (!query.exec()) {
        qDebug() << "Find user failed:" << query.lastError().text();
        return user;
    }

    if (query.next()) {
        user.userId = query.value("user_id").toInt();
        user.username = query.value("username").toString();
        user.password = query.value("password").toString();
        user.role = query.value("role").toString();
        user.status = query.value("status").toString();
    }

    return user;
}

/*
 * 检查用户名是否存在
 * ----------------
 * 注册用户时使用。
 * 因为数据库保存的是加密用户名，所以这里也要先加密再查。
 */
bool UserRepository::usernameExists(const QString& username)
{
    QString encryptedUsername = DESUtil::encrypt(username);

    QSqlQuery query(DatabaseManager::database());

    query.prepare("SELECT COUNT(*) FROM users WHERE username = :username");
    query.bindValue(":username", encryptedUsername);

    if (!query.exec()) {
        qDebug() << "Check username failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

/*
 * 插入新用户
 * ----------------
 * 参数说明：
 * username：明文用户名，函数内部会加密后再保存；
 * encryptedPassword：已经加密好的密码；
 * role：admin 或 user；
 * status：active 或 disabled。
 */
bool UserRepository::insertUser(const QString& username,
                                const QString& encryptedPassword,
                                const QString& role,
                                const QString& status)
{
    QString encryptedUsername = DESUtil::encrypt(username);

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        "INSERT INTO users (username, password, role, status) "
        "VALUES (:username, :password, :role, :status)"
        );

    query.bindValue(":username", encryptedUsername);
    query.bindValue(":password", encryptedPassword);
    query.bindValue(":role", role);
    query.bindValue(":status", status);

    if (!query.exec()) {
        qDebug() << "Insert user failed:" << query.lastError().text();
        return false;
    }

    return true;
}
