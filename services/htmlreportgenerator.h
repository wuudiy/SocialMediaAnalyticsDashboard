#ifndef HTMLREPORTGENERATOR_H
#define HTMLREPORTGENERATOR_H

#include "reportgenerator.h"

/*
 * HTML 字符转义工具。
 *
 * HtmlReportGenerator 使用 protected 继承复用它，
 * 避免 htmlEscape() 被外部当作公共接口调用。
 */
class HtmlEscaper
{
protected:
    HtmlEscaper() = default;
    ~HtmlEscaper() = default;

    QString htmlEscape(const QString& value) const;
};

/*
 * HTML 报表生成器。
 *
 * 继承关系：
 * HtmlReportGenerator -> AnalyticsReportGenerator -> ReportGenerator。
 *
 * 负责：
 * - 生成 HTML 页面结构；
 * - 使用与参考 HTML 类似的卡片式样式；
 * - 调用 ChartImageBuilder 生成图表 PNG 并以内嵌 base64 形式放入 HTML。
 *
 * 不负责：
 * - 查询数据库；
 * - 保存文件；
 * - 绘制 PNG 图表的底层细节。
 */
class HtmlReportGenerator : public AnalyticsReportGenerator,
                            protected HtmlEscaper
{
public:
    explicit HtmlReportGenerator(AnalyticsService *analyticsService);

    ExportFormat format() const override;
    GeneratedReport generate(const ExportRequest& request) override;

private:
    QString buildStyleSheet() const;

    QString buildHeaderSection(const ExportRequest& request,
                               const QString& generatedAt) const;

    QString buildSummarySection(const DashboardSummary& summary) const;

    QString buildChartsSection(const ReportData& data) const;

    QString buildPlatformTableSection(const QList<PlatformStatistics>& platformStats) const;

    QString buildTopPostsTableSection(const QList<Post>& topPosts) const;
};

#endif // HTMLREPORTGENERATOR_H