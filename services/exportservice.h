#ifndef EXPORTSERVICE_H
#define EXPORTSERVICE_H

#include "../models/exportmodels.h"
#include "../services/analyticsservice.h"

#include <QString>

/*
 * 报表导出服务。
 *
 * 负责：
 * - 根据筛选条件生成 TXT 报表；
 * - 根据筛选条件生成 CSV 报表；
 * - 写入本地文件。
 *
 * 不负责：
 * - QFileDialog；
 * - QMessageBox；
 * - 当前登录用户；
 * - 操作日志。
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
