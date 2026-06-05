#ifndef EXPORTSERVICE_H
#define EXPORTSERVICE_H

#include "../models/exportmodels.h"
#include "analyticsservice.h"

#include <QByteArray>
#include <QString>

/*
 * 报表导出服务。
 *
 * 重构后，ExportService 不再直接拼接 CSV / TXT / HTML 内容。
 *
 * 职责变为：
 * - 校验导出请求；
 * - 根据 ExportFormat 选择对应 ReportGenerator；
 * - 调用多态接口生成报表；
 * - 将报表字节数据写入本地文件。
 *
 * 这样继承体系真正解决了代码复用和扩展问题：
 * - 公共统计数据加载逻辑放在 AnalyticsReportGenerator；
 * - TXT / CSV / HTML 的格式化逻辑放在各自子类；
 * - ExportService 只依赖 ReportGenerator 抽象接口。
 */
class ExportService
{
public:
    ExportService();

    bool validateRequest(const ExportRequest& request,
                         QString& message) const;

    GeneratedReport generateReport(const ExportRequest& request);

    ExportSaveResult saveReportToFile(const QByteArray& fileData,
                                      const QString& filePath) const;

    static QString extensionForFormat(ExportFormat format);

    static QString displayNameForFormat(ExportFormat format);

private:
    AnalyticsService analyticsService;
};

#endif // EXPORTSERVICE_H