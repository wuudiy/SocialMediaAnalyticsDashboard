#include "exportservice.h"

#include "reportgenerator.h"

#include <QFile>
#include <QTextStream>
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

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

QString ExportService::generateReport(const ExportRequest& request)
{
    /*
     * 使用工厂根据格式创建具体报表生成器。
     *
     * ExportService 只依赖 ReportGenerator 抽象接口，
     * 不再关心 CSV / TXT 的具体拼接细节。
     *
     * 这里体现了多态：
     * - CSV 请求会调用 CsvReportGenerator::generate()
     * - TXT 请求会调用 TxtReportGenerator::generate()
     */
    const std::unique_ptr<ReportGenerator> generator =
        ReportGeneratorFactory::create(request.format, &analyticsService);

    return generator->generate(request);
}

ExportSaveResult ExportService::saveReportToFile(const QString& content,
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

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.success = false;
        result.message = QStringLiteral("Failed to open file for writing.");
        return result;
    }

    QTextStream stream(&file);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    stream.setEncoding(QStringConverter::Utf8);
#else
    stream.setCodec("UTF-8");
#endif

    stream << content;
    file.close();

    result.success = true;
    result.message = QStringLiteral("Report exported successfully.");
    return result;
}

QString ExportService::extensionForFormat(ExportFormat format)
{
    return format == ExportFormat::Csv
               ? QStringLiteral("csv")
               : QStringLiteral("txt");
}

QString ExportService::displayNameForFormat(ExportFormat format)
{
    return format == ExportFormat::Csv
               ? QStringLiteral("CSV")
               : QStringLiteral("TXT");
}
