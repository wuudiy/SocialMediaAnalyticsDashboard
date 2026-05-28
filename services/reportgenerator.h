#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include "../models/exportmodels.h"
#include "../models/post.h"
#include "analyticsservice.h"

#include <QList>
#include <QString>

#include <memory>

/*
 * 报表生成器抽象基类。
 *
 * 这是本项目自定义继承体系的顶层抽象类。
 *
 * 设计目的：
 * - 统一 CSV / TXT 等不同报表格式的生成入口；
 * - 让 ExportService 只依赖抽象类 ReportGenerator；
 * - 后续新增 PDF / HTML / Excel 报表时，只需要新增子类。
 *
 * 体现：
 * - 类型抽象；
 * - 运行时多态；
 * - public inheritance。
 */
class ReportGenerator
{
public:
    virtual ~ReportGenerator() = default;

    // 当前生成器支持的导出格式。
    virtual ExportFormat format() const = 0;

    // 根据导出请求生成报表内容。
    virtual QString generate(const ExportRequest& request) = 0;
};

/*
 * 分析报表生成器中间基类。
 *
 * 继承关系：
 * AnalyticsReportGenerator -> ReportGenerator
 *
 * 设计目的：
 * - 复用 AnalyticsService 数据加载逻辑；
 * - 复用平台显示、权限范围显示、百分比格式化等公共逻辑；
 * - 避免 CSV / TXT 报表重复查询 summary / platformStats / topPosts。
 *
 * 这是 multilevel inheritance 的中间层：
 * TxtReportGenerator -> AnalyticsReportGenerator -> ReportGenerator
 * CsvReportGenerator -> AnalyticsReportGenerator -> ReportGenerator
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
     * TXT 和 CSV 都需要这些数据，
     * 所以统一由父类加载，子类只负责格式化输出。
     */
    struct ReportData
    {
        DashboardSummary summary;
        QList<PlatformStatistics> platformStats;
        QList<Post> topPosts;
    };

    ReportData loadReportData(const ExportRequest& request,
                              int topPostLimit = 5) const;

    QString displayPlatform(const QString& platform) const;

    QString scopeText(const AnalyticsFilter& filter) const;

    QString percentText(double rate) const;

    QString shortText(const QString& text,
                      int maxLength) const;

protected:
    AnalyticsService *analyticsService;
};

/*
 * CSV 转义工具基类。
 *
 * 设计目的：
 * - CSV 字符串转义是可复用能力；
 * - 但它不是一种完整的报表生成器；
 * - 所以 CsvReportGenerator 使用 protected inheritance 复用它。
 *
 * 体现：
 * - multiple inheritance；
 * - protected inheritance mode；
 * - 实现复用但不暴露给外部。
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
 * TxtReportGenerator -> AnalyticsReportGenerator -> ReportGenerator
 *
 * 体现：
 * - single inheritance；
 * - multilevel inheritance；
 * - 重写虚函数 generate()；
 * - 多态生成 TXT 报表。
 */
class TxtReportGenerator : public AnalyticsReportGenerator
{
public:
    explicit TxtReportGenerator(AnalyticsService *analyticsService);

    ExportFormat format() const override;

    QString generate(const ExportRequest& request) override;
};

/*
 * CSV 报表生成器。
 *
 * 继承关系：
 * CsvReportGenerator -> AnalyticsReportGenerator -> ReportGenerator
 *
 * 同时：
 * CsvReportGenerator protected 继承 CsvEscaper。
 *
 * 体现：
 * - multiple inheritance；
 * - protected inheritance；
 * - 多态生成 CSV 报表；
 * - 复用 CsvEscaper::csvEscape()。
 */
class CsvReportGenerator : public AnalyticsReportGenerator,
                           protected CsvEscaper
{
public:
    explicit CsvReportGenerator(AnalyticsService *analyticsService);

    ExportFormat format() const override;

    QString generate(const ExportRequest& request) override;
};

/*
 * 报表生成器工厂。
 *
 * ExportService 不需要知道具体生成器怎么创建，
 * 只需要根据 ExportFormat 拿到 ReportGenerator 抽象指针。
 */
class ReportGeneratorFactory
{
public:
    static std::unique_ptr<ReportGenerator> create(ExportFormat format,
                                                   AnalyticsService *analyticsService);
};

#endif // REPORTGENERATOR_H
