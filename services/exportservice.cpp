#include "exportservice.h"

#include "reportgenerator.h"

#include <QFile>

ExportService::ExportService()
{
}

bool ExportService::validateRequest(const ExportRequest& request,
                                    QString& message) const
{
    if (request.filter.startDate.isValid()
        && request.filter.endDate.isValid()
        && request.filter.startDate > request.filter.endDate) {
        message = QStringLiteral("Start date cannot be later than end date.");
        return false;
    }

    if (!request.filter.includeAllUsers
        && request.filter.ownerUserId <= 0) {
        message = QStringLiteral("Current user is invalid. Please login again.");
        return false;
    }

    return true;
}

GeneratedReport ExportService::generateReport(const ExportRequest& request)
{
    /*
     * 使用工厂根据格式创建具体报表生成器。
     *
     * ExportService 只依赖 ReportGenerator 抽象接口，
     * 不关心 CSV / TXT / HTML 的具体生成细节。
     *
     * 这里体现了多态：
     * - CSV 请求会调用 CsvReportGenerator::generate()
     * - TXT 请求会调用 TxtReportGenerator::generate()
     * - HTML 请求会调用 HtmlReportGenerator::generate()
     */
    const std::unique_ptr<ReportGenerator> generator =
        ReportGeneratorFactory::create(request.format, &analyticsService);

    return generator->generate(request);
}

ExportSaveResult ExportService::saveReportToFile(const QByteArray& fileData,
                                                 const QString& filePath) const
{
    ExportSaveResult result;
    result.filePath = filePath;

    if (filePath.trimmed().isEmpty()) {
        result.success = false;
        result.message = QStringLiteral("Export cancelled.");
        return result;
    }

    QFile file(filePath);

    /*
     * 统一写入 QByteArray。
     *
     * TXT / CSV / HTML 都会先转成 UTF-8 字节，
     * 后续如果扩展二进制导出格式，也可以继续复用这里。
     */
    if (!file.open(QIODevice::WriteOnly)) {
        result.success = false;
        result.message = QStringLiteral("Failed to open file for writing.");
        return result;
    }

    if (file.write(fileData) != fileData.size()) {
        result.success = false;
        result.message = QStringLiteral("Failed to write complete report file.");
        file.close();
        return result;
    }

    file.close();

    result.success = true;
    result.message = QStringLiteral("Report exported successfully.");
    return result;
}

QString ExportService::extensionForFormat(ExportFormat format)
{
    switch (format) {
    case ExportFormat::Csv:
        return QStringLiteral("csv");
    case ExportFormat::Txt:
        return QStringLiteral("txt");
    case ExportFormat::Html:
        return QStringLiteral("html");
    }

    return QStringLiteral("txt");
}

QString ExportService::displayNameForFormat(ExportFormat format)
{
    switch (format) {
    case ExportFormat::Csv:
        return QStringLiteral("CSV");
    case ExportFormat::Txt:
        return QStringLiteral("TXT");
    case ExportFormat::Html:
        return QStringLiteral("HTML");
    }

    return QStringLiteral("TXT");
}