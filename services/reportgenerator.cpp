#include "reportgenerator.h"

#include <QDateTime>
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
     * 公共数据加载逻辑集中在父类。
     *
     * 这样 TXT / CSV 子类都不用重复写：
     * - loadDashboardSummary()
     * - getPlatformStatistics()
     * - getTopPosts()
     */
    data.summary = analyticsService->loadDashboardSummary(request.filter);
    data.platformStats = analyticsService->getPlatformStatistics(request.filter);
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

QString AnalyticsReportGenerator::percentText(double rate) const
{
    return QStringLiteral("%1%").arg(rate * 100.0, 0, 'f', 2);
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

QString TxtReportGenerator::generate(const ExportRequest& request)
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
    stream << "  Start Date: " << filter.startDate.toString(QStringLiteral("yyyy-MM-dd")) << "\n";
    stream << "  End Date:   " << filter.endDate.toString(QStringLiteral("yyyy-MM-dd")) << "\n";
    stream << "  Platform:   " << displayPlatform(filter.platform) << "\n";
    stream << "  Scope:      " << scopeText(filter) << "\n\n";

    stream << "========================================\n";
    stream << "Summary Statistics\n";
    stream << "========================================\n";
    stream << "  Total Posts:         " << data.summary.totalPosts << "\n";
    stream << "  Total Likes:         " << data.summary.totalLikes << "\n";
    stream << "  Total Comments:      " << data.summary.totalComments << "\n";
    stream << "  Total Shares:        " << data.summary.totalShares << "\n";
    stream << "  Total Interactions:  " << data.summary.totalInteractions << "\n";
    stream << "  Total Views:         " << data.summary.totalViews << "\n";
    stream << "  Avg Engagement Rate: " << percentText(data.summary.averageEngagementRate) << "\n\n";

    stream << "========================================\n";
    stream << "Platform Statistics\n";
    stream << "========================================\n";

    for (const PlatformStatistics& stats : data.platformStats) {
        stream << "\n";
        stream << "Platform: " << stats.platform << "\n";
        stream << "  Post Count:        " << stats.postCount << "\n";
        stream << "  Total Likes:       " << stats.totalLikes << "\n";
        stream << "  Total Comments:    " << stats.totalComments << "\n";
        stream << "  Total Shares:      " << stats.totalShares << "\n";
        stream << "  Total Views:       " << stats.totalViews << "\n";
        stream << "  Total Interaction: " << stats.totalInteractions << "\n";
        stream << "  Engagement Rate:   " << percentText(stats.averageEngagementRate) << "\n";
    }

    stream << "\n";
    stream << "========================================\n";
    stream << "Top Posts by Interactions\n";
    stream << "========================================\n";

    int rank = 1;

    for (const Post& post : data.topPosts) {
        stream << "\n";
        stream << rank << ". " << post.platform << " - " << post.accountName << "\n";
        stream << "   Content:      " << shortText(post.content, 60) << "\n";
        stream << "   Date:         " << post.publishDate.toString(QStringLiteral("yyyy-MM-dd")) << "\n";
        stream << "   Interactions: " << post.interactionCount() << "\n";
        stream << "   Views:        " << post.views << "\n";
        stream << "   Engagement:   " << percentText(post.engagementRate()) << "\n";

        ++rank;
    }

    stream << "\n";
    stream << "========================================\n";
    stream << "End of Report\n";
    stream << "========================================\n";

    return report;
}

CsvReportGenerator::CsvReportGenerator(AnalyticsService *analyticsService)
    : AnalyticsReportGenerator(analyticsService)
{
}

ExportFormat CsvReportGenerator::format() const
{
    return ExportFormat::Csv;
}

QString CsvReportGenerator::generate(const ExportRequest& request)
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
    stream << "Start Date," << csvEscape(filter.startDate.toString(QStringLiteral("yyyy-MM-dd"))) << "\n";
    stream << "End Date," << csvEscape(filter.endDate.toString(QStringLiteral("yyyy-MM-dd"))) << "\n";
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

    return csv;
}

std::unique_ptr<ReportGenerator> ReportGeneratorFactory::create(ExportFormat format,
                                                                AnalyticsService *analyticsService)
{
    if (format == ExportFormat::Csv) {
        return std::make_unique<CsvReportGenerator>(analyticsService);
    }

    return std::make_unique<TxtReportGenerator>(analyticsService);
}
