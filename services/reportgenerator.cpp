#include "reportgenerator.h"

#include "htmlreportgenerator.h"

#include <QDateTime>
#include <QLocale>
#include <QTextStream>

AnalyticsReportGenerator::AnalyticsReportGenerator(AnalyticsService *analyticsService)
    : analyticsService(analyticsService)
{
}

AnalyticsReportGenerator::ReportData
AnalyticsReportGenerator::loadReportData(const ExportRequest& request,
                                         int topPostLimit) const
{
    ReportData data;

    if (!analyticsService) {
        return data;
    }

    /*
     * 所有导出格式使用同一份统计数据。
     *
     * 这样新增 HTML 时不用重复写查询逻辑，
     * 只需要在子类中处理展示格式。
     */
    data.summary = analyticsService->loadDashboardSummary(request.filter);
    data.platformStats = analyticsService->getPlatformStatistics(request.filter);
    data.dateTrends = analyticsService->getDateTrends(request.filter);
    data.topPosts = analyticsService->getTopPosts(topPostLimit, request.filter);

    return data;
}

QString AnalyticsReportGenerator::displayPlatform(const QString& platform) const
{
    return platform.trimmed().isEmpty()
    ? QStringLiteral("All Platforms")
    : platform.trimmed();
}

QString AnalyticsReportGenerator::scopeText(const AnalyticsFilter& filter) const
{
    return filter.includeAllUsers
               ? QStringLiteral("All Users")
               : QStringLiteral("Current User Only");
}

QString AnalyticsReportGenerator::dateText(const QDate& date) const
{
    return date.isValid()
    ? date.toString(QStringLiteral("yyyy-MM-dd"))
    : QStringLiteral("N/A");
}

QString AnalyticsReportGenerator::dateRangeText(const AnalyticsFilter& filter) const
{
    return QStringLiteral("%1 to %2")
    .arg(dateText(filter.startDate), dateText(filter.endDate));
}

QString AnalyticsReportGenerator::percentText(double rate) const
{
    return QStringLiteral("%1%").arg(rate * 100.0, 0, 'f', 2);
}

QString AnalyticsReportGenerator::numberText(qint64 value) const
{
    return QLocale(QLocale::English, QLocale::UnitedStates).toString(value);
}

QString AnalyticsReportGenerator::shortText(const QString& text,
                                            int maxLength) const
{
    const QString cleaned = text.simplified();

    if (cleaned.length() <= maxLength) {
        return cleaned;
    }

    return cleaned.left(maxLength - 3) + QStringLiteral("...");
}

GeneratedReport AnalyticsReportGenerator::makePlainTextReport(const QString& content) const
{
    GeneratedReport report;

    report.fileData = content.toUtf8();
    report.previewContent = content;
    report.previewType = ExportPreviewType::PlainText;

    return report;
}

GeneratedReport AnalyticsReportGenerator::makeHtmlReport(const QString& html) const
{
    GeneratedReport report;

    report.fileData = html.toUtf8();
    report.previewContent = html;
    report.previewType = ExportPreviewType::Html;

    return report;
}

QString CsvEscaper::csvEscape(const QString& value) const
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

TxtReportGenerator::TxtReportGenerator(AnalyticsService *analyticsService)
    : AnalyticsReportGenerator(analyticsService)
{
}

ExportFormat TxtReportGenerator::format() const
{
    return ExportFormat::Txt;
}

GeneratedReport TxtReportGenerator::generate(const ExportRequest& request)
{
    QString report;
    QTextStream stream(&report);

    const AnalyticsFilter filter = request.filter;
    const ReportData data = loadReportData(request, 5);

    stream << "========================================\n";
    stream << "    Social Media Analytics Report\n";
    stream << "========================================\n\n";

    stream << "Report Generated: "
           << QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
           << "\n\n";

    stream << "Data Range:\n";
    stream << "  Start Date: " << dateText(filter.startDate) << "\n";
    stream << "  End Date:   " << dateText(filter.endDate) << "\n";
    stream << "  Platform:   " << displayPlatform(filter.platform) << "\n";
    stream << "  Scope:      " << scopeText(filter) << "\n\n";

    stream << "========================================\n";
    stream << "Summary Statistics\n";
    stream << "========================================\n";
    stream << "  Total Posts:         " << numberText(data.summary.totalPosts) << "\n";
    stream << "  Total Likes:         " << numberText(data.summary.totalLikes) << "\n";
    stream << "  Total Comments:      " << numberText(data.summary.totalComments) << "\n";
    stream << "  Total Shares:        " << numberText(data.summary.totalShares) << "\n";
    stream << "  Total Interactions:  " << numberText(data.summary.totalInteractions) << "\n";
    stream << "  Total Views:         " << numberText(data.summary.totalViews) << "\n";
    stream << "  Avg Engagement Rate: " << percentText(data.summary.averageEngagementRate) << "\n\n";

    stream << "========================================\n";
    stream << "Platform Statistics\n";
    stream << "========================================\n";

    if (data.platformStats.isEmpty()) {
        stream << "No platform statistics available.\n";
    }

    for (const PlatformStatistics& stats : data.platformStats) {
        stream << "\n";
        stream << "Platform: " << stats.platform << "\n";
        stream << "  Post Count:        " << numberText(stats.postCount) << "\n";
        stream << "  Total Likes:       " << numberText(stats.totalLikes) << "\n";
        stream << "  Total Comments:    " << numberText(stats.totalComments) << "\n";
        stream << "  Total Shares:      " << numberText(stats.totalShares) << "\n";
        stream << "  Total Views:       " << numberText(stats.totalViews) << "\n";
        stream << "  Total Interaction: " << numberText(stats.totalInteractions) << "\n";
        stream << "  Engagement Rate:   " << percentText(stats.averageEngagementRate) << "\n";
    }

    stream << "\n";
    stream << "========================================\n";
    stream << "Top Posts by Interactions\n";
    stream << "========================================\n";

    if (data.topPosts.isEmpty()) {
        stream << "No post data available.\n";
    }

    int rank = 1;

    for (const Post& post : data.topPosts) {
        stream << "\n";
        stream << rank << ". " << post.platform << " - " << post.accountName << "\n";
        stream << "   Content:      " << shortText(post.content, 60) << "\n";
        stream << "   Date:         " << post.publishDate.toString(QStringLiteral("yyyy-MM-dd")) << "\n";
        stream << "   Interactions: " << numberText(post.interactionCount()) << "\n";
        stream << "   Views:        " << numberText(post.views) << "\n";
        stream << "   Engagement:   " << percentText(post.engagementRate()) << "\n";

        ++rank;
    }

    stream << "\n";
    stream << "========================================\n";
    stream << "End of Report\n";
    stream << "========================================\n";

    return makePlainTextReport(report);
}

CsvReportGenerator::CsvReportGenerator(AnalyticsService *analyticsService)
    : AnalyticsReportGenerator(analyticsService)
{
}

ExportFormat CsvReportGenerator::format() const
{
    return ExportFormat::Csv;
}

GeneratedReport CsvReportGenerator::generate(const ExportRequest& request)
{
    QString csv;
    QTextStream stream(&csv);

    const AnalyticsFilter filter = request.filter;
    const ReportData data = loadReportData(request, 5);

    stream << "Social Media Analytics Report\n\n";

    stream << "Report Generated,"
           << csvEscape(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
           << "\n\n";

    stream << "Data Range\n";
    stream << "Start Date," << csvEscape(dateText(filter.startDate)) << "\n";
    stream << "End Date," << csvEscape(dateText(filter.endDate)) << "\n";
    stream << "Platform," << csvEscape(displayPlatform(filter.platform)) << "\n";
    stream << "Scope," << csvEscape(scopeText(filter)) << "\n\n";

    stream << "Summary Statistics\n";
    stream << "Metric,Value\n";
    stream << "Total Posts," << data.summary.totalPosts << "\n";
    stream << "Total Likes," << data.summary.totalLikes << "\n";
    stream << "Total Comments," << data.summary.totalComments << "\n";
    stream << "Total Shares," << data.summary.totalShares << "\n";
    stream << "Total Interactions," << data.summary.totalInteractions << "\n";
    stream << "Total Views," << data.summary.totalViews << "\n";
    stream << "Avg Engagement Rate," << csvEscape(percentText(data.summary.averageEngagementRate)) << "\n\n";

    stream << "Platform Statistics\n";
    stream << "Platform,Post Count,Total Likes,Total Comments,Total Shares,Total Interactions,Total Views,Engagement Rate\n";

    for (const PlatformStatistics& stats : data.platformStats) {
        stream << csvEscape(stats.platform) << ","
               << stats.postCount << ","
               << stats.totalLikes << ","
               << stats.totalComments << ","
               << stats.totalShares << ","
               << stats.totalInteractions << ","
               << stats.totalViews << ","
               << csvEscape(percentText(stats.averageEngagementRate))
               << "\n";
    }

    stream << "\n";
    stream << "Top Posts by Interactions\n";
    stream << "Rank,Platform,Account,Content,Publish Date,Interactions,Views,Engagement Rate\n";

    int rank = 1;

    for (const Post& post : data.topPosts) {
        stream << rank << ","
               << csvEscape(post.platform) << ","
               << csvEscape(post.accountName) << ","
               << csvEscape(shortText(post.content, 100)) << ","
               << csvEscape(post.publishDate.toString(QStringLiteral("yyyy-MM-dd"))) << ","
               << post.interactionCount() << ","
               << post.views << ","
               << csvEscape(percentText(post.engagementRate()))
               << "\n";

        ++rank;
    }

    return makePlainTextReport(csv);
}

std::unique_ptr<ReportGenerator> ReportGeneratorFactory::create(
    ExportFormat format,
    AnalyticsService *analyticsService)
{
    switch (format) {
    case ExportFormat::Csv:
        return std::make_unique<CsvReportGenerator>(analyticsService);
    case ExportFormat::Txt:
        return std::make_unique<TxtReportGenerator>(analyticsService);
    case ExportFormat::Html:
        return std::make_unique<HtmlReportGenerator>(analyticsService);
    }

    return std::make_unique<TxtReportGenerator>(analyticsService);
}