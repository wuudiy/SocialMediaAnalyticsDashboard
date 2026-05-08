#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "../models/User.h"

#include <QString>

class QSqlQuery;

/*
 * users 表的数据访问层。
 *
 * 负责：
 * - 查询用户；
 * - 判断用户名是否存在；
 * - 插入新用户。
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

    // 插入用户。username 是明文，encryptedPassword 是已加密密码。
    bool insertUser(const QString& username,
                    const QString& encryptedPassword,
                    const QString& role,
                    const QString& status);

private:
    // 把当前 SQL 查询行转换成 User 对象。
    User buildUserFromQuery(const QSqlQuery& query) const;

    // 用户名加密统一放这里，避免多个查询函数重复写 DESUtil::encrypt。
    QString encryptUsername(const QString& username) const;
};

#endif // USERREPOSITORY_H
