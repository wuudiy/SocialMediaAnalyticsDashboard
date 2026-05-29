#ifndef CSVIMPORTSERVICE_H
#define CSVIMPORTSERVICE_H

#include "postservice.h"

#include "../models/post.h"
#include "../models/user.h"

#include <QByteArray>
#include <QDate>
#include <QString>
#include <QStringList>

/*
 * CSV 导入结果。
 *
 * Controller 根据这个结果：
 * - 刷新表格；
 * - 显示导入成功 / 失败信息；
 * - 写操作日志。
 */
struct CsvImportResult
{
    bool fileOpened = false;
    int successCount = 0;
    int failedCount = 0;
    QString errorMessage;
    QStringList errorDetails;

    bool hasFailedRows() const
    {
        return !fileOpened || failedCount > 0;
    }

    QString toDisplayMessage() const
    {
        if (!fileOpened) {
            return errorMessage.isEmpty()
            ? QStringLiteral("Cannot open selected CSV file.")
            : errorMessage;
        }

        QString message = QStringLiteral("CSV import finished. Success: %1, Failed: %2.")
                              .arg(successCount)
                              .arg(failedCount);

        if (!errorDetails.isEmpty()) {
            message += QStringLiteral("\n") + errorDetails.join(QStringLiteral("\n"));
        }

        return message;
    }
};

/*
 * CSV 导入服务。
 *
 * 负责：
 * - 打开 CSV 文件；
 * - 判断 CSV 格式；
 * - 解析 CSV 行；
 * - 构造 Post；
 * - 调用 PostService 保存数据。
 *
 * 数据隔离改造后：
 * - CSV 导入的数据会自动归属于当前登录用户；
 * - Service 不关心页面，也不写日志。
 */
class CsvImportService
{
public:
    CsvImportService();

    CsvImportResult importFromFile(const User& currentUser,
                                   const QString& fileName);

private:
    enum class CsvFormat
    {
        Unknown,
        StandardPost,
        BilibiliTrend,
        DouyinWorkList
    };

private:
    bool readCsvTextFile(const QString& fileName,
                         QString& content,
                         QString& message) const;

    bool looksLikeExcelWorkbook(const QByteArray& bytes) const;

    QString decodeCsvBytes(const QByteArray& bytes) const;

    QString cleanCsvField(const QString& value) const;

    QStringList splitCsvRows(const QString& content) const;

    QStringList splitCsvLine(const QString& line) const;

    QDate parseCsvDate(const QString& value) const;

    CsvFormat detectCsvFormat(const QStringList& fields) const;

    bool buildPostFromCsvFields(const QStringList& fields,
                                CsvFormat format,
                                Post& post,
                                QString& message) const;

    int csvNumber(const QString& value) const;

private:
    PostService postService;
};

#endif // CSVIMPORTSERVICE_H
