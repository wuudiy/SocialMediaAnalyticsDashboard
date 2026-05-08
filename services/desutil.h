#ifndef DESUTIL_H
#define DESUTIL_H

#include <QByteArray>
#include <QString>
#include <QVector>

/*
 * DES 加密工具类。
 *
 * 负责：
 * - 加密普通字符串；
 * - 解密 Base64 密文字符串；
 * - 为 username 和 password 提供统一加密逻辑。
 *
 * 不负责：
 * - 保存用户；
 * - 查询数据库；
 * - 判断登录是否成功。
 *
 * 注意：
 * 当前 DES 是课程项目演示级实现。
 * 如果后续项目要进入真实生产环境，密码不应该使用可逆加密，
 * 应改为 bcrypt / Argon2 / PBKDF2 这类不可逆密码哈希。
 */
class DESUtil
{
public:
    // 加密普通字符串，返回 Base64 编码后的密文。
    static QString encrypt(const QString& plainText);

    // 解密 Base64 密文，返回普通字符串。
    static QString decrypt(const QString& cipherText);

private:
    // 加密单个 8 字节数据块。
    static QByteArray encryptBlock(const QByteArray& block,
                                   const QByteArray& key);

    // 解密单个 8 字节数据块。
    static QByteArray decryptBlock(const QByteArray& block,
                                   const QByteArray& key);

    // 字节数组转 bit 数组。
    static QVector<int> bytesToBits(const QByteArray& data);

    // bit 数组转字节数组。
    static QByteArray bitsToBytes(const QVector<int>& bits);

    // 按置换表进行置换。
    static QVector<int> permute(const QVector<int>& input,
                                const int* table,
                                int tableSize);

    // 循环左移，用于生成每一轮子密钥。
    static QVector<int> leftShift(const QVector<int>& input,
                                  int shiftCount);

    // 根据 64-bit key 生成 16 个 48-bit 子密钥。
    static QVector<QVector<int>> generateSubKeys(const QByteArray& key);

    // bit 异或。
    static QVector<int> xorBits(const QVector<int>& a,
                                const QVector<int>& b);

    // S-box 替换：48-bit -> 32-bit。
    static QVector<int> sBoxSubstitution(const QVector<int>& input);

    // Feistel 轮函数。
    static QVector<int> feistel(const QVector<int>& right,
                                const QVector<int>& subKey);

    // PKCS 风格填充，使数据长度变成 8 的倍数。
    static QByteArray addPadding(const QByteArray& data);

    // 去除填充。
    static QByteArray removePadding(const QByteArray& data);

    // 默认 DES 密钥，长度必须是 8 字节。
    static QByteArray defaultKey();
};

#endif // DESUTIL_H
