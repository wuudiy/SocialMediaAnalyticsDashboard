#include "sha256util.h"

#include <QCryptographicHash>

/*
 * 计算密码哈希
 * ----------------
 * 处理流程：
 * 1. 明文密码转 UTF-8 字节；
 * 2. 使用 SHA-256 计算摘要；
 * 3. 转成十六进制字符串，方便保存到 MySQL 的 VARCHAR 字段。
 */
QString SHA256Util::hashPassword(const QString& password)
{
    const QByteArray hash = QCryptographicHash::hash(
        password.toUtf8(),
        QCryptographicHash::Sha256
        );

    return QString::fromUtf8(hash.toHex());
}

/*
 * 校验密码
 * ----------------
 * 处理流程：
 * 1. 对用户输入的密码重新计算 SHA-256；
 * 2. 和数据库中保存的哈希值比较；
 * 3. 相同则说明密码正确。
 */
bool SHA256Util::verifyPassword(const QString& password,
                                const QString& storedHash)
{
    return hashPassword(password) == storedHash.trimmed().toLower();
}

/*
 * 判断 SHA-256 格式
 * ----------------
 * SHA-256 的十六进制结果固定是 64 位。
 * 每一位只可能是 0-9 / a-f / A-F。
 */
bool SHA256Util::isSha256Hash(const QString& text)
{
    const QString value = text.trimmed();

    if (value.length() != 64) {
        return false;
    }

    for (const QChar& ch : value) {
        const bool isNumber = ch >= QChar('0') && ch <= QChar('9');
        const bool isLowerHex = ch >= QChar('a') && ch <= QChar('f');
        const bool isUpperHex = ch >= QChar('A') && ch <= QChar('F');

        if (!isNumber && !isLowerHex && !isUpperHex) {
            return false;
        }
    }

    return true;
}
