#ifndef EXPORTMODELS_H
#define EXPORTMODELS_H

#include "analyticsmodels.h"

#include <QString>

/*
 * 导出格式。
 *
 * 目前系统支持：
 * - CSV：适合 Excel / 数据分析；
 * - TXT：适合阅读和提交报告。
 */
enum class ExportFormat
{
    Csv,
    Txt
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
