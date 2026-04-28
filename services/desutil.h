#ifndef DESUTIL_H
#define DESUTIL_H

#include <QString>
#include <QByteArray>
#include <QVector>

/*
 * Service / Utility 层：DESUtil
 * ----------------------------
 * 作用：
 * 1. 提供 DES 加密和解密方法；
 * 2. 用于加密保存 username 和 password；
 * 3. 登录验证时，负责解密数据库中的 password。
 *
 * 对外只暴露两个核心方法：
 * - encrypt()
 * - decrypt()
 *
 * 其余函数都是 DES 内部流程：
 * 初始置换、子密钥生成、Feistel 轮函数、S-box 替换、P-box 置换等。
 */
class DESUtil
{
public:
    // 加密普通字符串，返回 Base64 编码后的密文字符串
    static QString encrypt(const QString& plainText);

    // 解密 Base64 密文字符串，返回明文
    static QString decrypt(const QString& cipherText);

private:
    // 加密一个 8 字节数据块
    static QByteArray encryptBlock(const QByteArray& block, const QByteArray& key);

    // 解密一个 8 字节数据块
    static QByteArray decryptBlock(const QByteArray& block, const QByteArray& key);

    // 字节数组转 bit 数组
    static QVector<int> bytesToBits(const QByteArray& data);

    // bit 数组转字节数组
    static QByteArray bitsToBytes(const QVector<int>& bits);

    // 按置换表进行置换
    static QVector<int> permute(const QVector<int>& input, const int* table, int tableSize);

    // 循环左移，用于生成每一轮子密钥
    static QVector<int> leftShift(const QVector<int>& input, int shiftCount);

    // 根据 64-bit key 生成 16 个 48-bit 子密钥
    static QVector<QVector<int>> generateSubKeys(const QByteArray& key);

    // bit 异或
    static QVector<int> xorBits(const QVector<int>& a, const QVector<int>& b);

    // S-box 替换：48-bit -> 32-bit
    static QVector<int> sBoxSubstitution(const QVector<int>& input);

    // Feistel 轮函数
    static QVector<int> feistel(const QVector<int>& right, const QVector<int>& subKey);

    // PKCS 风格填充，使数据长度变成 8 的倍数
    static QByteArray addPadding(const QByteArray& data);

    // 去除填充
    static QByteArray removePadding(const QByteArray& data);

    // 默认 DES 密钥，长度必须是 8 字节
    static QByteArray defaultKey();
};

#endif // DESUTIL_H
