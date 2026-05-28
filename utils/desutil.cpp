#include "DESUtil.h"

#include <algorithm>

/*
 * DES 标准初始置换表 IP
 * 作用：加密前重新排列 64-bit 明文。
 */
static const int IP[64] = {
    58,50,42,34,26,18,10,2,
    60,52,44,36,28,20,12,4,
    62,54,46,38,30,22,14,6,
    64,56,48,40,32,24,16,8,
    57,49,41,33,25,17,9,1,
    59,51,43,35,27,19,11,3,
    61,53,45,37,29,21,13,5,
    63,55,47,39,31,23,15,7
};

/*
 * DES 最终置换表 FP
 * 作用：16 轮 Feistel 处理结束后重新排列 64-bit 数据。
 */
static const int FP[64] = {
    40,8,48,16,56,24,64,32,
    39,7,47,15,55,23,63,31,
    38,6,46,14,54,22,62,30,
    37,5,45,13,53,21,61,29,
    36,4,44,12,52,20,60,28,
    35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26,
    33,1,41,9,49,17,57,25
};

/*
 * 扩展置换表 E
 * 作用：把右半部分 32-bit 扩展为 48-bit，方便和 48-bit 子密钥异或。
 */
static const int E[48] = {
    32,1,2,3,4,5,
    4,5,6,7,8,9,
    8,9,10,11,12,13,
    12,13,14,15,16,17,
    16,17,18,19,20,21,
    20,21,22,23,24,25,
    24,25,26,27,28,29,
    28,29,30,31,32,1
};

/*
 * P-box 置换表
 * 作用：S-box 输出 32-bit 后再次打乱顺序。
 */
static const int P[32] = {
    16,7,20,21,
    29,12,28,17,
    1,15,23,26,
    5,18,31,10,
    2,8,24,14,
    32,27,3,9,
    19,13,30,6,
    22,11,4,25
};

/*
 * PC-1 密钥置换表
 * 作用：从 64-bit key 中选出 56-bit，并去掉校验位。
 */
static const int PC1[56] = {
    57,49,41,33,25,17,9,
    1,58,50,42,34,26,18,
    10,2,59,51,43,35,27,
    19,11,3,60,52,44,36,
    63,55,47,39,31,23,15,
    7,62,54,46,38,30,22,
    14,6,61,53,45,37,29,
    21,13,5,28,20,12,4
};

/*
 * PC-2 压缩置换表
 * 作用：从 56-bit 密钥中选出 48-bit 子密钥。
 */
static const int PC2[48] = {
    14,17,11,24,1,5,
    3,28,15,6,21,10,
    23,19,12,4,26,8,
    16,7,27,20,13,2,
    41,52,31,37,47,55,
    30,40,51,45,33,48,
    44,49,39,56,34,53,
    46,42,50,36,29,32
};

/*
 * 每一轮密钥循环左移位数
 * 第 1、2、9、16 轮左移 1 位，其余轮左移 2 位。
 */
static const int SHIFT_SCHEDULE[16] = {
    1,1,2,2,
    2,2,2,2,
    1,2,2,2,
    2,2,2,1
};

/*
 * DES 的 8 个 S-box
 * 每个 S-box 输入 6-bit，输出 4-bit。
 */
static const int S_BOX[8][4][16] = {
    {
        {14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7},
        {0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8},
        {4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0},
        {15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13}
    },
    {
        {15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10},
        {3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5},
        {0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15},
        {13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9}
    },
    {
        {10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8},
        {13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1},
        {13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7},
        {1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12}
    },
    {
        {7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15},
        {13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9},
        {10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4},
        {3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14}
    },
    {
        {2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9},
        {14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6},
        {4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14},
        {11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3}
    },
    {
        {12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11},
        {10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8},
        {9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6},
        {4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13}
    },
    {
        {4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1},
        {13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6},
        {1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2},
        {6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12}
    },
    {
        {13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7},
        {1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2},
        {7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8},
        {2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11}
    }
};

/*
 * 默认 DES 密钥
 * ----------------
 * DES key 必须是 8 字节，即 64-bit。
 * 这里先写死，方便课程项目第一阶段测试。
 */
QByteArray DESUtil::defaultKey()
{
    return QByteArray("A1B2C3D4");
}

/*
 * 加密字符串
 * ----------------
 * 处理流程：
 * 1. 明文转 UTF-8 字节；
 * 2. 按 8 字节分组，不足部分填充；
 * 3. 每组执行 DES 加密；
 * 4. 最后转 Base64，方便保存到 MySQL 的 VARCHAR 字段。
 */
QString DESUtil::encrypt(const QString& plainText)
{
    QByteArray data = addPadding(plainText.toUtf8());
    QByteArray key = defaultKey();
    QByteArray encrypted;

    for (int i = 0; i < data.size(); i += 8) {
        encrypted.append(encryptBlock(data.mid(i, 8), key));
    }

    return QString::fromUtf8(encrypted.toBase64());
}

/*
 * 解密字符串
 * ----------------
 * 处理流程：
 * 1. 从 Base64 还原密文字节；
 * 2. 每 8 字节执行 DES 解密；
 * 3. 去掉填充；
 * 4. 转回 QString 明文。
 */
QString DESUtil::decrypt(const QString& cipherText)
{
    QByteArray encrypted = QByteArray::fromBase64(cipherText.toUtf8());
    QByteArray key = defaultKey();
    QByteArray decrypted;

    for (int i = 0; i < encrypted.size(); i += 8) {
        decrypted.append(decryptBlock(encrypted.mid(i, 8), key));
    }

    decrypted = removePadding(decrypted);
    return QString::fromUtf8(decrypted);
}

/*
 * 加密单个 64-bit 数据块
 * ----------------
 * DES 核心加密过程：
 * 1. 初始置换 IP；
 * 2. 分成 L 和 R 两个 32-bit；
 * 3. 执行 16 轮 Feistel 运算；
 * 4. 合并 R 和 L；
 * 5. 最终置换 FP。
 */
QByteArray DESUtil::encryptBlock(const QByteArray& block, const QByteArray& key)
{
    QVector<int> bits = bytesToBits(block);
    QVector<int> permuted = permute(bits, IP, 64);

    QVector<int> left = permuted.mid(0, 32);
    QVector<int> right = permuted.mid(32, 32);

    QVector<QVector<int>> subKeys = generateSubKeys(key);

    for (int round = 0; round < 16; ++round) {
        QVector<int> oldRight = right;
        QVector<int> fResult = feistel(right, subKeys[round]);

        right = xorBits(left, fResult);
        left = oldRight;
    }

    QVector<int> combined;
    combined += right;
    combined += left;

    QVector<int> finalBits = permute(combined, FP, 64);
    return bitsToBytes(finalBits);
}

/*
 * 解密单个 64-bit 数据块
 * ----------------
 * DES 解密流程和加密基本相同，
 * 区别是 16 个子密钥使用顺序反过来。
 */
QByteArray DESUtil::decryptBlock(const QByteArray& block, const QByteArray& key)
{
    QVector<int> bits = bytesToBits(block);
    QVector<int> permuted = permute(bits, IP, 64);

    QVector<int> left = permuted.mid(0, 32);
    QVector<int> right = permuted.mid(32, 32);

    QVector<QVector<int>> subKeys = generateSubKeys(key);
    std::reverse(subKeys.begin(), subKeys.end());

    for (int round = 0; round < 16; ++round) {
        QVector<int> oldRight = right;
        QVector<int> fResult = feistel(right, subKeys[round]);

        right = xorBits(left, fResult);
        left = oldRight;
    }

    QVector<int> combined;
    combined += right;
    combined += left;

    QVector<int> finalBits = permute(combined, FP, 64);
    return bitsToBytes(finalBits);
}

/*
 * 字节转 bit
 * ----------------
 * 每个字节拆成 8 个 bit，从高位到低位。
 */
QVector<int> DESUtil::bytesToBits(const QByteArray& data)
{
    QVector<int> bits;

    for (unsigned char byte : data) {
        for (int i = 7; i >= 0; --i) {
            bits.append((byte >> i) & 1);
        }
    }

    return bits;
}

/*
 * bit 转字节
 * ----------------
 * 每 8 个 bit 合成一个字节。
 */
QByteArray DESUtil::bitsToBytes(const QVector<int>& bits)
{
    QByteArray data;

    for (int i = 0; i < bits.size(); i += 8) {
        unsigned char byte = 0;

        for (int j = 0; j < 8; ++j) {
            byte = static_cast<unsigned char>((byte << 1) | bits[i + j]);
        }

        data.append(static_cast<char>(byte));
    }

    return data;
}

/*
 * 通用置换函数
 * ----------------
 * table 中的编号从 1 开始，
 * QVector 下标从 0 开始，所以要减 1。
 */
QVector<int> DESUtil::permute(const QVector<int>& input, const int* table, int tableSize)
{
    QVector<int> output;

    for (int i = 0; i < tableSize; ++i) {
        output.append(input[table[i] - 1]);
    }

    return output;
}

/*
 * 循环左移
 * ----------------
 * 用于密钥调度阶段。
 */
QVector<int> DESUtil::leftShift(const QVector<int>& input, int shiftCount)
{
    QVector<int> output = input;

    for (int i = 0; i < shiftCount; ++i) {
        int first = output.first();
        output.removeFirst();
        output.append(first);
    }

    return output;
}

/*
 * 生成 16 个子密钥
 * ----------------
 * 1. 64-bit key 经过 PC-1 得到 56-bit；
 * 2. 分成 C 和 D 两个 28-bit；
 * 3. 每一轮循环左移；
 * 4. 经过 PC-2 得到 48-bit 子密钥。
 */
QVector<QVector<int>> DESUtil::generateSubKeys(const QByteArray& key)
{
    QVector<int> keyBits = bytesToBits(key);
    QVector<int> permutedKey = permute(keyBits, PC1, 56);

    QVector<int> left = permutedKey.mid(0, 28);
    QVector<int> right = permutedKey.mid(28, 28);

    QVector<QVector<int>> subKeys;

    for (int round = 0; round < 16; ++round) {
        left = leftShift(left, SHIFT_SCHEDULE[round]);
        right = leftShift(right, SHIFT_SCHEDULE[round]);

        QVector<int> combined;
        combined += left;
        combined += right;

        subKeys.append(permute(combined, PC2, 48));
    }

    return subKeys;
}

/*
 * bit 异或
 */
QVector<int> DESUtil::xorBits(const QVector<int>& a, const QVector<int>& b)
{
    QVector<int> result;

    for (int i = 0; i < a.size(); ++i) {
        result.append(a[i] ^ b[i]);
    }

    return result;
}

/*
 * S-box 替换
 * ----------------
 * 输入 48-bit，分成 8 组，每组 6-bit。
 * 每个 S-box 输出 4-bit。
 * 最终输出 32-bit。
 */
QVector<int> DESUtil::sBoxSubstitution(const QVector<int>& input)
{
    QVector<int> output;

    for (int box = 0; box < 8; ++box) {
        int start = box * 6;

        int row = input[start] * 2 + input[start + 5];
        int column = input[start + 1] * 8
                     + input[start + 2] * 4
                     + input[start + 3] * 2
                     + input[start + 4];

        int value = S_BOX[box][row][column];

        for (int bit = 3; bit >= 0; --bit) {
            output.append((value >> bit) & 1);
        }
    }

    return output;
}

/*
 * Feistel 轮函数
 * ----------------
 * 1. 右半部分 32-bit 扩展为 48-bit；
 * 2. 与当前轮子密钥异或；
 * 3. S-box 压缩回 32-bit；
 * 4. P-box 置换。
 */
QVector<int> DESUtil::feistel(const QVector<int>& right, const QVector<int>& subKey)
{
    QVector<int> expanded = permute(right, E, 48);
    QVector<int> xored = xorBits(expanded, subKey);
    QVector<int> sBoxed = sBoxSubstitution(xored);
    QVector<int> pBoxed = permute(sBoxed, P, 32);

    return pBoxed;
}

/*
 * 添加填充
 * ----------------
 * DES 每次处理 8 字节。
 * 如果明文不是 8 的倍数，需要补齐。
 */
QByteArray DESUtil::addPadding(const QByteArray& data)
{
    QByteArray padded = data;
    int paddingSize = 8 - (data.size() % 8);

    if (paddingSize == 0) {
        paddingSize = 8;
    }

    for (int i = 0; i < paddingSize; ++i) {
        padded.append(static_cast<char>(paddingSize));
    }

    return padded;
}

/*
 * 移除填充
 */
QByteArray DESUtil::removePadding(const QByteArray& data)
{
    if (data.isEmpty()) {
        return data;
    }

    int paddingSize = static_cast<unsigned char>(data.at(data.size() - 1));

    if (paddingSize <= 0 || paddingSize > 8 || paddingSize > data.size()) {
        return data;
    }

    return data.left(data.size() - paddingSize);
}
