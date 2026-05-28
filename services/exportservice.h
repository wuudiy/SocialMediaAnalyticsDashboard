#ifndef EXPORTSERVICE_H
#define EXPORTSERVICE_H

#include "../models/exportmodels.h"
#include "analyticsservice.h"

#include <QString>

/*
 * 报表导出服务。
 *
 * 负责：
 * - 根据筛选条件生成 TXT 报表；
 * - 根据筛选条件生成 CSV 报表；
 * - 写入本地文件。
 *
 * 数据隔离说明：
 * ExportService 不直接判断 admin / user。
 * Controller 会把当前用户权限写入 ExportRequest::filter，
 * ExportService 只按 filter 生成报表。
 */
class ExportService
{
public:
    ExportService();

    bool validateRequest(const ExportRequest& request,
                         QString& message) const;

    QString generateReport(const ExportRequest& request);

    ExportSaveResult saveReportToFile(const QString& content,
                                      const QString& filePath) const;

    static QString extensionForFormat(ExportFormat format);

    static QString displayNameForFormat(ExportFormat format);

private:
    QString generateTxtReport(const ExportRequest& request);

    QString generateCsvReport(const ExportRequest& request);

    QString csvEscape(const QString& value) const;

    QString displayPlatform(const QString& platform) const;

private:
    AnalyticsService analyticsService;
};

#endif // EXPORTSERVICE_H
