#ifndef EXPORTMODELS_H
#define EXPORTMODELS_H

#include "analyticsmodels.h"

#include <QByteArray>
#include <QString>

/*
 * 导出格式。
 *
 * 目前系统支持：
 * - CSV：适合 Excel / 数据分析；
 * - TXT：适合阅读和提交报告；
 * - HTML：适合以网页形式查看图文报表。
 *
 * 预留说明：
 * 后续如果继续扩展图片类导出，可以在这里新增新的枚举值，
 * 并在 ReportGeneratorFactory 中注册对应生成器。
 */
enum class ExportFormat
{
    Csv,
    Txt,
    Html
};

/*
 * 报表预览类型。
 *
 * QTextEdit 预览时需要区分：
 * - PlainText：使用 setPlainText() 显示普通文本；
 * - Html：使用 setHtml() 显示 HTML 页面。
 */
enum class ExportPreviewType
{
    PlainText,
    Html
};

/*
 * 报表生成结果。
 *
 * 设计目的：
 * - TXT / CSV / HTML 都可以统一转换为 UTF-8 字节写入文件；
 * - previewContent 专门给页面预览使用，避免 View 直接关心文件保存细节；
 * - fileData 保留为 QByteArray，是为了给后续二进制格式扩展预留空间。
 */
struct GeneratedReport
{
    QByteArray fileData;
    QString previewContent;
    ExportPreviewType previewType = ExportPreviewType::PlainText;
};

/*
 * 导出请求。
 *
 * View 只负责收集这些条件；
 * Controller 收到后交给 ExportService 生成报表内容。
 */
struct ExportRequest
{
    ExportFormat format = ExportFormat::Csv;
    AnalyticsFilter filter;
    QString baseFileName = QStringLiteral("social_media_report");
};

/*
 * 保存文件结果。
 *
 * ExportService 写文件后返回给 Controller，
 * Controller 再决定怎么提示 View、怎么写日志。
 */
struct ExportSaveResult
{
    bool success = false;
    QString message;
    QString filePath;
};

#endif // EXPORTMODELS_H