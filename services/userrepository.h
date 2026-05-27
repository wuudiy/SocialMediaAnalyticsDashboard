#ifndef USERREPOSITORY_H
#define USERREPOSITORY_H

#include "../models/user.h"

#include <QList>
#include <QString>

class QSqlQuery;

/*
 * users 表的数据访问层。
 * 负责用户查询、新增、密码更新、状态更新等数据库操作。
 */
class UserRepository
{
public:
    UserRepository();

    // 根据明文用户名查询用户，内部会先加密用户名。
    User findByUsername(const QString& username);

    // 查询全部用户，返回的 username 已解密，供界面展示。
    QList<User> findAllUsers();

    // 判断明文用户名是否已经存在。
    bool usernameExists(const QString& username);

    // 插入新用户。passwordHash 应该已经是 SHA-256 哈希。
    bool insertUser(const QString& username,
                    const QString& passwordHash,
                    const QString& role,
                    const QString& status);

    // 根据用户 ID 更新密码。
    bool updatePasswordByUserId(int userId,
                                const QString& passwordHash);

    // 根据用户 ID 更新状态：active / disabled。
    bool updateStatusByUserId(int userId,
                              const QString& status);

    // 统计当前 active admin 数量。
    int countActiveAdmins();

private:
    // 把当前 SQL 查询行转换成 User 对象。
    User buildUserFromQuery(const QSqlQuery& query) const;

    // 用户名加密 / 解密统一放这里，避免多处重复写 DES 逻辑。
    QString encryptUsername(const QString& username) const;
    QString decryptUsername(const QString& encryptedUsername) const;
};

#endif // USERREPOSITORY_H
