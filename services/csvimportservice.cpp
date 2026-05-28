#include "csvimportservice.h"

#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

CsvImportService::CsvImportService()
{
}

CsvImportResult CsvImportService::importFromFile(const User& currentUser,
                                                 const QString& fileName)
{
    CsvImportResult result;

    if (!currentUser.isValid()) {
        result.fileOpened = false;
        result.errorMessage = QStringLiteral("Please login before importing CSV.");
        return result;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.fileOpened = false;
        result.errorMessage = QStringLiteral("Cannot open selected CSV file.");
        return result;
    }

    result.fileOpened = true;

    QTextStream stream(&file);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    stream.setEncoding(QStringConverter::Utf8);
#else
    stream.setCodec("UTF-8");
#endif

    int lineNumber = 0;
    CsvFormat csvFormat = CsvFormat::Unknown;

    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;

        if (line.isEmpty()) {
            continue;
        }

        const QStringList fields = splitCsvLine(line);

        /*
         * 第一行优先尝试识别表头：
         * - 如果识别到通用帖子 CSV 或 Bilibili 趋势 CSV，则跳过表头；
         * - 如果没有识别到表头，则把第一行当作通用帖子数据处理。
         */
        if (csvFormat == CsvFormat::Unknown) {
            csvFormat = detectCsvFormat(fields);

            if (csvFormat != CsvFormat::Unknown) {
                continue;
            }

            csvFormat = CsvFormat::StandardPost;
        }

        /*
         * Bilibili 趋势文件中可能有 “累计” 汇总行。
         * 这行不是单日数据，不能作为帖子导入。
         */
        if (csvFormat == CsvFormat::BilibiliTrend
            && cleanCsvField(fields.value(0)) == QStringLiteral("累计")) {
            continue;
        }

        Post post;
        QString message;

        if (!buildPostFromCsvFields(fields, csvFormat, post, message)) {
            ++result.failedCount;

            if (result.errorDetails.size() < 5) {
                result.errorDetails.append(
                    QStringLiteral("Line %1: %2")
                        .arg(lineNumber)
                        .arg(message.isEmpty()
                                 ? QStringLiteral("Invalid CSV row.")
                                 : message)
                    );
            }

            continue;
        }

        /*
         * 数据隔离关键点：
         * 导入的每条帖子都通过 PostService::addPost(currentUser, post) 保存，
         * PostService 会自动给帖子补 createdByUserId / createdByUsername。
         */
        const PostOperationResult saveResult = postService.addPost(currentUser, post);

        if (!saveResult.success) {
            ++result.failedCount;

            if (result.errorDetails.size() < 5) {
                result.errorDetails.append(
                    QStringLiteral("Line %1: %2")
                        .arg(lineNumber)
                        .arg(saveResult.message.isEmpty()
                                 ? QStringLiteral("Insert database failed.")
                                 : saveResult.message)
                    );
            }

            continue;
        }

        ++result.successCount;
    }

    return result;
}

QString CsvImportService::cleanCsvField(const QString& value) const
{
    QString field = value.trimmed();

    // 去掉 UTF-8 BOM，避免 CSV 第一列头部出现不可见字符。
    if (!field.isEmpty() && field.at(0).unicode() == 0xFEFF) {
        field.remove(0, 1);
    }

    // 支持 "a,b" 这种带引号的 CSV 字段，并把 CSV 转义的 "" 还原为 "。
    if (field.size() >= 2
        && field.startsWith(QChar('"'))
        && field.endsWith(QChar('"'))) {
        field = field.mid(1, field.size() - 2);
        field.replace(QStringLiteral("\"\""), QStringLiteral("\""));
    }

    return field.trimmed();
}

QStringList CsvImportService::splitCsvLine(const QString& line) const
{
    QStringList fields;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        const QChar ch = line.at(i);

        if (ch == QChar('"')) {
            if (inQuotes
                && i + 1 < line.size()
                && line.at(i + 1) == QChar('"')) {
                current.append(QChar('"'));
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == QChar(',') && !inQuotes) {
            fields.append(cleanCsvField(current));
            current.clear();
        } else {
            current.append(ch);
        }
    }

    fields.append(cleanCsvField(current));
    return fields;
}

QDate CsvImportService::parseCsvDate(const QString& value) const
{
    QString dateText = cleanCsvField(value);

    // 有些 CSV 日期会带时间，例如 2026-05-25 12:00:00，这里只保留日期部分。
    if (dateText.contains(QChar(' '))) {
        dateText = dateText.section(QChar(' '), 0, 0);
    }

    const QStringList formats = {
        QStringLiteral("yyyy-MM-dd"),
        QStringLiteral("yyyy-M-d"),
        QStringLiteral("yyyy/MM/dd"),
        QStringLiteral("yyyy/M/d")
    };

    for (const QString& format : formats) {
        const QDate date = QDate::fromString(dateText, format);

        if (date.isValid()) {
            return date;
        }
    }

    return QDate();
}

CsvImportService::CsvFormat CsvImportService::detectCsvFormat(const QStringList& fields) const
{
    QStringList headers;

    for (const QString& field : fields) {
        headers.append(cleanCsvField(field).toLower());
    }

    const QString joinedHeader = headers.join(QStringLiteral(","));

    // 通用帖子 CSV：platform, account, content, date, likes, comments, shares, views
    if (joinedHeader.contains(QStringLiteral("platform"))
        && joinedHeader.contains(QStringLiteral("account"))
        && joinedHeader.contains(QStringLiteral("views"))) {
        return CsvFormat::StandardPost;
    }

    // Bilibili 趋势 CSV：按中文表头识别。
    if (joinedHeader.contains(QStringLiteral("时间"))
        && joinedHeader.contains(QStringLiteral("播放量"))
        && joinedHeader.contains(QStringLiteral("点赞"))) {
        return CsvFormat::BilibiliTrend;
    }

    return CsvFormat::Unknown;
}

bool CsvImportService::buildPostFromCsvFields(const QStringList& fields,
                                              CsvFormat format,
                                              Post& post,
                                              QString& message) const
{
    if (format == CsvFormat::StandardPost) {
        if (fields.size() < 8) {
            message = QStringLiteral("Standard CSV row must contain 8 fields.");
            return false;
        }

        post.platform = cleanCsvField(fields.at(0));
        post.accountName = cleanCsvField(fields.at(1));
        post.content = cleanCsvField(fields.at(2));
        post.publishDate = parseCsvDate(fields.at(3));
        post.likes = csvNumber(fields.at(4));
        post.comments = csvNumber(fields.at(5));
        post.shares = csvNumber(fields.at(6));
        post.views = csvNumber(fields.at(7));

        return postService.validatePost(post, message);
    }

    if (format == CsvFormat::BilibiliTrend) {
        if (fields.size() < 10) {
            message = QStringLiteral("Bilibili trend CSV row must contain 10 fields.");
            return false;
        }

        const QString dateText = cleanCsvField(fields.at(0));

        post.platform = QStringLiteral("Bilibili");
        post.accountName = QStringLiteral("Bilibili Video");
        post.content = QStringLiteral("Bilibili playback trend - %1").arg(dateText);
        post.publishDate = parseCsvDate(dateText);
        post.views = csvNumber(fields.at(1));
        post.likes = csvNumber(fields.at(4));
        post.comments = csvNumber(fields.at(5)) + csvNumber(fields.at(6));
        post.shares = csvNumber(fields.at(7));

        return postService.validatePost(post, message);
    }

    message = QStringLiteral("Unsupported CSV format.");
    return false;
}

int CsvImportService::csvNumber(const QString& value) const
{
    QString numberText = cleanCsvField(value);
    numberText.remove(QChar(','));
    return numberText.toInt();
}
