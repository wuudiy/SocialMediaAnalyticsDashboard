#include "csvimportservice.h"

#include <QDateTime>
#include <QFile>
#include <QIODevice>
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
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

    QString csvContent;
    QString openMessage;

    if (!readCsvTextFile(fileName, csvContent, openMessage)) {
        result.fileOpened = false;
        result.errorMessage = openMessage;
        return result;
    }

    result.fileOpened = true;

    int lineNumber = 0;
    CsvFormat csvFormat = CsvFormat::Unknown;
    const QStringList rows = splitCsvRows(csvContent);

    for (const QString& row : rows) {
        const QString line = row.trimmed();
        ++lineNumber;

        if (line.isEmpty()) {
            continue;
        }

        const QStringList fields = splitCsvLine(line);

        /*
         * 第一行优先尝试识别表头：
         * - 如果识别到通用帖子 CSV、Bilibili 趋势 CSV 或抖音作品列表 CSV，则跳过表头；
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
         * 平台导出的文件中可能有“累计 / 合计”汇总行。
         * 这类行不是单条作品数据，不能作为帖子导入。
         */
        const QString firstField = cleanCsvField(fields.value(0));

        if ((csvFormat == CsvFormat::BilibiliTrend
             || csvFormat == CsvFormat::DouyinWorkList)
            && (firstField == QStringLiteral("累计")
                || firstField == QStringLiteral("合计"))) {
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

bool CsvImportService::readCsvTextFile(const QString& fileName,
                                       QString& content,
                                       QString& message) const
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        message = QStringLiteral("Cannot open selected CSV file.");
        return false;
    }

    const QByteArray bytes = file.readAll();

    if (bytes.isEmpty()) {
        message = QStringLiteral("Selected CSV file is empty.");
        return false;
    }

    /*
     * 抖音导出的文件有时扩展名是 .CSV，
     * 但文件内容实际是 Excel 2007+ 的 xlsx 压缩包。
     *
     * xlsx 不能按文本 CSV 读取，否则会读到 PK... 文件头，
     * 后续表头识别一定失败。
     */
    if (looksLikeExcelWorkbook(bytes)) {
        message = QStringLiteral(
            "Selected file is an Excel workbook, not a real CSV text file.\n"
            "Please open it with Excel or WPS, choose Save As, select CSV UTF-8, "
            "then import the saved .csv file again."
            );
        return false;
    }

    content = decodeCsvBytes(bytes);

    if (content.trimmed().isEmpty()) {
        message = QStringLiteral("Selected CSV file does not contain valid text content.");
        return false;
    }

    /*
     * 如果 CSV 里已经出现大量问号，说明中文大概率在转换时已经丢失。
     * 这种情况程序无法恢复，只能提示用户回到原始 Excel 文件重新另存为 UTF-8 CSV。
     */
    if (content.left(300).count(QChar('?')) >= 6
        && !content.left(300).contains(QStringLiteral("作品名称"))
        && !content.left(300).contains(QStringLiteral("发布时间"))) {
        message = QStringLiteral(
            "Selected CSV seems to contain broken Chinese text like ???? .\n"
            "The Chinese characters were probably lost during CSV conversion.\n"
            "Please convert again from the original Excel file and choose CSV UTF-8."
            );
        return false;
    }

    return true;
}

bool CsvImportService::looksLikeExcelWorkbook(const QByteArray& bytes) const
{
    if (bytes.size() < 4) {
        return false;
    }

    /*
     * xlsx 本质是 zip 包，文件头通常是 PK。
     * 再结合 [Content_Types].xml / xl/workbook.xml 判断，
     * 避免把普通文本 CSV 误判为 Excel 文件。
     */
    const bool hasZipHeader =
        bytes.at(0) == 'P'
        && bytes.at(1) == 'K';

    if (!hasZipHeader) {
        return false;
    }

    return bytes.contains("[Content_Types].xml")
           || bytes.contains("xl/workbook.xml")
           || bytes.contains("xl/worksheets/");
}

QString CsvImportService::decodeCsvBytes(const QByteArray& bytes) const
{
    /*
     * UTF-8 BOM。
     */
    if (bytes.startsWith("\xEF\xBB\xBF")) {
        return QString::fromUtf8(bytes.mid(3));
    }

    /*
     * UTF-16 LE BOM。
     */
    if (bytes.startsWith("\xFF\xFE")) {
        QString text;

        for (int i = 2; i + 1 < bytes.size(); i += 2) {
            const ushort value =
                static_cast<unsigned char>(bytes.at(i))
                | (static_cast<unsigned char>(bytes.at(i + 1)) << 8);
            text.append(QChar(value));
        }

        return text;
    }

    /*
     * UTF-16 BE BOM。
     */
    if (bytes.startsWith("\xFE\xFF")) {
        QString text;

        for (int i = 2; i + 1 < bytes.size(); i += 2) {
            const ushort value =
                (static_cast<unsigned char>(bytes.at(i)) << 8)
                | static_cast<unsigned char>(bytes.at(i + 1));
            text.append(QChar(value));
        }

        return text;
    }

    const QString utf8Text = QString::fromUtf8(bytes);

    /*
     * 如果 UTF-8 解码没有出现替换字符，优先认为它就是 UTF-8。
     */
    if (!utf8Text.contains(QChar(0xFFFD))) {
        return utf8Text;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    /*
     * Qt5 下优先尝试 GB18030。
     * 国内 Excel / WPS 的普通 CSV 经常是 GBK / GB18030。
     */
    QTextCodec* gbCodec = QTextCodec::codecForName("GB18030");

    if (gbCodec) {
        const QString gbText = gbCodec->toUnicode(bytes);

        if (!gbText.contains(QChar(0xFFFD))) {
            return gbText;
        }
    }
#endif

    /*
     * Qt6 下没有 QTextCodec。
     * 在中文 Windows 上，fromLocal8Bit 通常可以正确读取 GBK / ANSI CSV。
     */
    const QString localText = QString::fromLocal8Bit(bytes);

    if (!localText.contains(QChar(0xFFFD))) {
        return localText;
    }

    return utf8Text;
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

QStringList CsvImportService::splitCsvRows(const QString& content) const
{
    QStringList rows;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < content.size(); ++i) {
        const QChar ch = content.at(i);

        if (ch == QChar('"')) {
            current.append(ch);

            if (inQuotes
                && i + 1 < content.size()
                && content.at(i + 1) == QChar('"')) {
                current.append(content.at(i + 1));
                ++i;
            } else {
                inQuotes = !inQuotes;
            }

            continue;
        }

        if (!inQuotes && (ch == QChar('\n') || ch == QChar('\r'))) {
            rows.append(current);
            current.clear();

            if (ch == QChar('\r')
                && i + 1 < content.size()
                && content.at(i + 1) == QChar('\n')) {
                ++i;
            }

            continue;
        }

        current.append(ch);
    }

    if (!current.isEmpty()) {
        rows.append(current);
    }

    return rows;
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
    dateText.replace(QStringLiteral("T"), QStringLiteral(" "));

    const QStringList dateTimeFormats = {
        QStringLiteral("yyyy-MM-dd HH:mm:ss"),
        QStringLiteral("yyyy-MM-dd HH:mm"),
        QStringLiteral("yyyy-M-d H:m:s"),
        QStringLiteral("yyyy-M-d H:m"),
        QStringLiteral("yyyy/MM/dd HH:mm:ss"),
        QStringLiteral("yyyy/MM/dd HH:mm"),
        QStringLiteral("yyyy/M/d H:m:s"),
        QStringLiteral("yyyy/M/d H:m"),
        QStringLiteral("yyyy年M月d日 H:m:s"),
        QStringLiteral("yyyy年M月d日 H:m")
    };

    for (const QString& format : dateTimeFormats) {
        const QDateTime dateTime = QDateTime::fromString(dateText, format);

        if (dateTime.isValid()) {
            return dateTime.date();
        }
    }

    // 有些 CSV 日期会带时间，例如 2026-05-25 12:00:00，这里只保留日期部分。
    if (dateText.contains(QChar(' '))) {
        dateText = dateText.section(QChar(' '), 0, 0);
    }

    const QStringList dateFormats = {
        QStringLiteral("yyyy-MM-dd"),
        QStringLiteral("yyyy-M-d"),
        QStringLiteral("yyyy/MM/dd"),
        QStringLiteral("yyyy/M/d"),
        QStringLiteral("yyyy年M月d日")
    };

    for (const QString& format : dateFormats) {
        const QDate date = QDate::fromString(dateText, format);

        if (date.isValid()) {
            return date;
        }
    }

    /*
     * WPS / Excel 有时会把日期保存成序列号。
     * Excel 日期序列号以 1899-12-30 为基准。
     */
    bool ok = false;
    const double serialNumber = dateText.toDouble(&ok);

    if (ok && serialNumber > 20000 && serialNumber < 80000) {
        return QDate(1899, 12, 30).addDays(static_cast<int>(serialNumber));
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

    /*
     * 抖音作品列表 CSV。
     *
     * 注意：
     * 必须放在 Bilibili 判断前面。
     * 因为“发布时间”包含“时间”，“点赞量”包含“点赞”，
     * 如果先判断 Bilibili，会把抖音 CSV 误识别成 Bilibili 趋势文件。
     */
    if (headers.contains(QStringLiteral("作品名称"))
        && headers.contains(QStringLiteral("发布时间"))
        && headers.contains(QStringLiteral("播放量"))
        && headers.contains(QStringLiteral("点赞量"))
        && headers.contains(QStringLiteral("分享量"))
        && headers.contains(QStringLiteral("评论量"))) {
        return CsvFormat::DouyinWorkList;
    }

    // 通用帖子 CSV：platform, account, content, date, likes, comments, shares, views
    if (joinedHeader.contains(QStringLiteral("platform"))
        && joinedHeader.contains(QStringLiteral("account"))
        && joinedHeader.contains(QStringLiteral("views"))) {
        return CsvFormat::StandardPost;
    }

    /*
     * Bilibili 趋势 CSV。
     *
     * 这里不要再只用 contains("时间") 做宽松判断，
     * 否则“发布时间”也会被当成 Bilibili 的“时间”字段。
     */
    if (!headers.isEmpty()
        && headers.first() == QStringLiteral("时间")
        && headers.contains(QStringLiteral("播放量"))
        && (headers.contains(QStringLiteral("点赞"))
            || headers.contains(QStringLiteral("点赞量")))) {
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

    if (format == CsvFormat::DouyinWorkList) {
        if (fields.size() < 13) {
            message = QStringLiteral("Douyin work list CSV row must contain at least 13 fields.");
            return false;
        }

        /*
         * 抖音作品列表字段顺序：
         * 0 作品名称
         * 1 发布时间
         * 2 体裁
         * 3 审核状态
         * 4 播放量
         * 10 点赞量
         * 11 分享量
         * 12 评论量
         *
         * 当前 Post 模型没有完播率、收藏量、粉丝增量等字段，
         * 所以这里只导入项目已有的通用统计字段。
         */
        post.platform = QStringLiteral("Douyin");
        post.accountName = QStringLiteral("Douyin Account");
        post.content = cleanCsvField(fields.value(0));
        post.publishDate = parseCsvDate(fields.value(1));
        post.views = csvNumber(fields.value(4));
        post.likes = csvNumber(fields.value(10));
        post.shares = csvNumber(fields.value(11));
        post.comments = csvNumber(fields.value(12));

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
    numberText.remove(QChar('%'));

    if (numberText.isEmpty() || numberText == QStringLiteral("-")) {
        return 0;
    }

    double multiplier = 1.0;

    if (numberText.endsWith(QStringLiteral("万"))) {
        multiplier = 10000.0;
        numberText.chop(1);
    } else if (numberText.endsWith(QStringLiteral("w"), Qt::CaseInsensitive)) {
        multiplier = 10000.0;
        numberText.chop(1);
    }

    bool ok = false;
    const double number = numberText.toDouble(&ok);

    if (!ok) {
        return 0;
    }

    return static_cast<int>(number * multiplier + 0.5);
}
