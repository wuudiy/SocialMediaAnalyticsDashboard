#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "../models/user.h"

#include <QString>

class QSqlQuery;

/*
 * users 表的数据访问层。
 *
 * 负责：
 * - 查询用户；
 * - 判断用户名是否存在；
 * - 插入新用户；
 * - 更新用户密码哈希。
 *
 * 不负责：
 * - 登录流程判断；
 * - 界面提示；
 * - 当前登录状态维护。
 */
class UserRepository
{
public:
    UserRepository();

    // 根据明文用户名查询用户。函数内部会加密后再查数据库。
    User findByUsername(const QString& username);

    // 判断明文用户名是否已经存在。
    bool usernameExists(const QString& username);

    // 插入用户。username 是明文，passwordHash 是 SHA-256 哈希。
    bool insertUser(const QString& username,
                    const QString& passwordHash,
                    const QString& role,
                    const QString& status);

    // 根据用户 ID 更新密码。主要用于旧 DES 密码自动升级成 SHA-256。
    bool updatePasswordByUserId(int userId,
                                const QString& passwordHash);

private:
    // 把当前 SQL 查询行转换成 User 对象。
    User buildUserFromQuery(const QSqlQuery& query) const;

    // 用户名加密统一放这里，避免多个查询函数重复写 DESUtil::encrypt。
    QString encryptUsername(const QString& username) const;
};

#endif // USERREPOSITORY_H
