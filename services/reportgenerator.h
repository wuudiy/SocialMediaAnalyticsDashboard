#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include "../models/ExportModels.h"
#include "../models/post.h"
#include "analyticsservice.h"

#include <QList>
#include <QString>

#include <memory>

/*
 * 报表生成器抽象基类。
 *
 * 负责：
 * - 定义所有报表生成器的统一接口；
 * - 让 ExportService 只依赖抽象的 ReportGenerator；
 * - 通过多态支持 CSV、TXT、HTML 等不同导出格式。
 *
 * 不负责：
 * - 查询统计数据；
 * - 拼接某一种具体格式的报表内容；
 * - 写入本地文件。
 */
class ReportGenerator
{
public:
    virtual ~ReportGenerator() = default;

    // 返回当前生成器对应的导出格式。
    virtual ExportFormat format() const = 0;

    // 根据导出请求生成报表文件数据和预览内容。
    virtual GeneratedReport generate(const ExportRequest& request) = 0;
};

/*
 * 分析类报表生成器中间基类。
 *
 * 负责：
 * - 统一加载 DashboardSummary、PlatformStatistics、DateTrend、TopPosts；
 * - 提供平台、范围、百分比、数字和短文本等公共格式化方法；
 * - 提供文本和 HTML 两类 GeneratedReport 封装方法。
 *
 * 继承意义：
 * - TXT、CSV、HTML、 都是分析类报表；
 * - 它们共享数据加载逻辑，但各自负责不同格式的输出。
 */
class AnalyticsReportGenerator : public ReportGenerator
{
public:
    explicit AnalyticsReportGenerator(AnalyticsService *analyticsService);
    ~AnalyticsReportGenerator() override = default;

protected:
    /*
     * 报表生成所需的公共数据。
     *
     * 子类只需要调用 loadReportData()，不用重复访问 AnalyticsService。
     */
    struct ReportData
    {
        DashboardSummary summary;
        QList<PlatformStatistics> platformStats;
        QList<DateTrend> dateTrends;
        QList<Post> topPosts;
    };

    ReportData loadReportData(const ExportRequest& request,
                              int topPostLimit = 5) const;

    QString displayPlatform(const QString& platform) const;
    QString scopeText(const AnalyticsFilter& filter) const;
    QString dateText(const QDate& date) const;
    QString dateRangeText(const AnalyticsFilter& filter) const;
    QString percentText(double rate) const;
    QString numberText(qint64 value) const;
    QString shortText(const QString& text,
                      int maxLength) const;

    GeneratedReport makePlainTextReport(const QString& content) const;
    GeneratedReport makeHtmlReport(const QString& html) const;

protected:
    AnalyticsService *analyticsService;
};

/*
 * CSV 转义工具基类。
 *
 * CsvReportGenerator 使用 protected 继承复用该工具，
 * 这样 csvEscape() 不会暴露为对外公共接口。
 */
class CsvEscaper
{
protected:
    CsvEscaper() = default;
    ~CsvEscaper() = default;

    QString csvEscape(const QString& value) const;
};

/*
 * TXT 报表生成器。
 *
 * 继承关系：
 * TxtReportGenerator -> AnalyticsReportGenerator -> ReportGenerator。
 */
class TxtReportGenerator : public AnalyticsReportGenerator
{
public:
    explicit TxtReportGenerator(AnalyticsService *analyticsService);

    ExportFormat format() const override;
    GeneratedReport generate(const ExportRequest& request) override;
};

/*
 * CSV 报表生成器。
 *
 * 继承关系：
 * CsvReportGenerator -> AnalyticsReportGenerator -> ReportGenerator。
 *
 * 同时 protected 继承 CsvEscaper，用于复用 CSV 字段转义逻辑。
 */
class CsvReportGenerator : public AnalyticsReportGenerator,
                           protected CsvEscaper
{
public:
    explicit CsvReportGenerator(AnalyticsService *analyticsService);

    ExportFormat format() const override;
    GeneratedReport generate(const ExportRequest& request) override;
};

/*
 * 报表生成器工厂。
 *
 * ExportService 通过该工厂获得 ReportGenerator 抽象指针，
 * 不需要直接依赖具体生成器类。
 */
class ReportGeneratorFactory
{
public:
    static std::unique_ptr<ReportGenerator> create(ExportFormat format,
                                                   AnalyticsService *analyticsService);
};

#endif // REPORTGENERATOR_H