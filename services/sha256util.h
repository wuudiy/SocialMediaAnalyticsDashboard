#ifndef SHA256UTIL_H
#define SHA256UTIL_H

#include <QString>

/*
 * SHA-256 哈希工具类。
 *
 * 负责：
 * - 把明文密码转换成 SHA-256 哈希；
 * - 校验明文密码和数据库中的哈希是否一致；
 * - 判断数据库中的密码是否已经是 SHA-256 格式。
 *
 * 注意：
 * SHA-256 是不可逆哈希，不能像 DES 一样解密。
 * 登录时应该重新计算输入密码的哈希，再和数据库中的哈希比对。
 */
class SHA256Util
{
public:
    // 计算密码的 SHA-256 哈希，返回 64 位十六进制字符串。
    static QString hashPassword(const QString& password);

    // 校验明文密码和已保存的 SHA-256 哈希是否一致。
    static bool verifyPassword(const QString& password,
                               const QString& storedHash);

    // 判断字符串是否符合 SHA-256 十六进制哈希格式。
    static bool isSha256Hash(const QString& text);
};

#endif // SHA256UTIL_H
