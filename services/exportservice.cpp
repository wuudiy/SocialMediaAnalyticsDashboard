#include "exportservice.h"

#include "../models/post.h"

#include <QDateTime>
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
    /*
     * 日期校验放在 Service 中，
     * 这样后续不管是按钮导出、定时导出还是命令行导出，都可以复用同一套规则。
     */
    if (request.filter.startDate.isValid()
        && request.filter.endDate.isValid()
        && request.filter.startDate > request.filter.endDate) {
        message = QStringLiteral("Start date cannot be later than end date.");
        return false;
    }

    return true;
}

QString ExportService::generateReport(const ExportRequest& request)
{
    if (request.format == ExportFormat::Csv) {
        return generateCsvReport(request);
    }

    return generateTxtReport(request);
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

QString ExportService::generateTxtReport(const ExportRequest& request)
{
    QString report;
    QTextStream stream(&report);

    const AnalyticsFilter filter = request.filter;

    const DashboardSummary summary = analyticsService.loadDashboardSummary(filter);
    const QList<PlatformStatistics> platformStats = analyticsService.getPlatformStatistics(filter);
    const QList<Post> topPosts = analyticsService.getTopPosts(5, filter);

    stream << "========================================\n";
    stream << "    Social Media Analytics Report\n";
    stream << "========================================\n\n";

    stream << "Report Generated: "
           << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
           << "\n\n";

    stream << "Data Range:\n";
    stream << "  Start Date: " << filter.startDate.toString(QStringLiteral("yyyy-MM-dd")) << "\n";
    stream << "  End Date:   " << filter.endDate.toString(QStringLiteral("yyyy-MM-dd")) << "\n";
    stream << "  Platform:   " << displayPlatform(filter.platform) << "\n\n";

    stream << "========================================\n";
    stream << "Summary Statistics\n";
    stream << "========================================\n";
    stream << "  Total Posts:         " << summary.totalPosts << "\n";
    stream << "  Total Likes:         " << summary.totalLikes << "\n";
    stream << "  Total Comments:      " << summary.totalComments << "\n";
    stream << "  Total Shares:        " << summary.totalShares << "\n";
    stream << "  Total Interactions:  " << summary.totalInteractions << "\n";
    stream << "  Total Views:         " << summary.totalViews << "\n";
    stream << "  Avg Engagement Rate: "
           << QStringLiteral("%1%").arg(summary.averageEngagementRate * 100.0, 0, 'f', 2)
           << "\n\n";

    stream << "========================================\n";
    stream << "Platform Statistics\n";
    stream << "========================================\n";

    for (const PlatformStatistics& stats : platformStats) {
        stream << "\n";
        stream << "Platform: " << stats.platform << "\n";
        stream << "  Post Count:       " << stats.postCount << "\n";
        stream << "  Total Likes:      " << stats.totalLikes << "\n";
        stream << "  Total Comments:   " << stats.totalComments << "\n";
        stream << "  Total Shares:     " << stats.totalShares << "\n";
        stream << "  Total Views:      " << stats.totalViews << "\n";
        stream << "  Total Interaction:" << stats.totalInteractions << "\n";
        stream << "  Engagement Rate:  "
               << QStringLiteral("%1%").arg(stats.averageEngagementRate * 100.0, 0, 'f', 2)
               << "\n";
    }

    stream << "\n";
    stream << "========================================\n";
    stream << "Top Posts by Interactions\n";
    stream << "========================================\n";

    int rank = 1;

    for (const Post& post : topPosts) {
        const QString shortContent = post.content.left(60)
        + (post.content.size() > 60 ? QStringLiteral("...") : QString());

        stream << "\n";
        stream << rank << ". " << post.platform << " - " << post.accountName << "\n";
        stream << "   Content:      " << shortContent << "\n";
        stream << "   Date:         " << post.publishDate.toString(QStringLiteral("yyyy-MM-dd")) << "\n";
        stream << "   Interactions: " << post.interactionCount() << "\n";
        stream << "   Views:        " << post.views << "\n";
        stream << "   Engagement:   "
               << QStringLiteral("%1%").arg(post.engagementRate() * 100.0, 0, 'f', 2)
               << "\n";

        ++rank;
    }

    stream << "\n";
    stream << "========================================\n";
    stream << "End of Report\n";
    stream << "========================================\n";

    return report;
}

QString ExportService::generateCsvReport(const ExportRequest& request)
{
    QString csv;
    QTextStream stream(&csv);

    const AnalyticsFilter filter = request.filter;

    const DashboardSummary summary = analyticsService.loadDashboardSummary(filter);
    const QList<PlatformStatistics> platformStats = analyticsService.getPlatformStatistics(filter);
    const QList<Post> topPosts = analyticsService.getTopPosts(5, filter);

    stream << "Social Media Analytics Report\n\n";

    stream << "Report Generated,"
           << csvEscape(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
           << "\n\n";

    stream << "Data Range\n";
    stream << "Start Date," << csvEscape(filter.startDate.toString(QStringLiteral("yyyy-MM-dd"))) << "\n";
    stream << "End Date," << csvEscape(filter.endDate.toString(QStringLiteral("yyyy-MM-dd"))) << "\n";
    stream << "Platform," << csvEscape(displayPlatform(filter.platform)) << "\n\n";

    stream << "Summary Statistics\n";
    stream << "Metric,Value\n";
    stream << "Total Posts," << summary.totalPosts << "\n";
    stream << "Total Likes," << summary.totalLikes << "\n";
    stream << "Total Comments," << summary.totalComments << "\n";
    stream << "Total Shares," << summary.totalShares << "\n";
    stream << "Total Interactions," << summary.totalInteractions << "\n";
    stream << "Total Views," << summary.totalViews << "\n";
    stream << "Avg Engagement Rate,"
           << csvEscape(QStringLiteral("%1%").arg(summary.averageEngagementRate * 100.0, 0, 'f', 2))
           << "\n\n";

    stream << "Platform Statistics\n";
    stream << "Platform,Post Count,Total Likes,Total Comments,Total Shares,Total Interactions,Total Views,Engagement Rate\n";

    for (const PlatformStatistics& stats : platformStats) {
        stream << csvEscape(stats.platform) << ","
               << stats.postCount << ","
               << stats.totalLikes << ","
               << stats.totalComments << ","
               << stats.totalShares << ","
               << stats.totalInteractions << ","
               << stats.totalViews << ","
               << csvEscape(QStringLiteral("%1%").arg(stats.averageEngagementRate * 100.0, 0, 'f', 2))
               << "\n";
    }

    stream << "\n";
    stream << "Top Posts by Interactions\n";
    stream << "Rank,Platform,Account,Content,Publish Date,Interactions,Views,Engagement Rate\n";

    int rank = 1;

    for (const Post& post : topPosts) {
        stream << rank << ","
               << csvEscape(post.platform) << ","
               << csvEscape(post.accountName) << ","
               << csvEscape(post.content.left(100)) << ","
               << csvEscape(post.publishDate.toString(QStringLiteral("yyyy-MM-dd"))) << ","
               << post.interactionCount() << ","
               << post.views << ","
               << csvEscape(QStringLiteral("%1%").arg(post.engagementRate() * 100.0, 0, 'f', 2))
               << "\n";

        ++rank;
    }

    return csv;
}

QString ExportService::csvEscape(const QString& value) const
{
    QString escaped = value;
    escaped.replace(QStringLiteral("\""), QStringLiteral("\"\""));

    if (escaped.contains(QChar(','))
        || escaped.contains(QChar('"'))
        || escaped.contains(QChar('\n'))
        || escaped.contains(QChar('\r'))) {
        escaped = QStringLiteral("\"%1\"").arg(escaped);
    }

    return escaped;
}

QString ExportService::displayPlatform(const QString& platform) const
{
    return platform.trimmed().isEmpty()
    ? QStringLiteral("All Platforms")
    : platform.trimmed();
}
