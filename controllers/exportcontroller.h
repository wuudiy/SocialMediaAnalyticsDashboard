#ifndef EXPORTCONTROLLER_H
#define EXPORTCONTROLLER_H

#include "../models/exportmodels.h"
#include "../models/user.h"
#include "../services/exportservice.h"
#include "../services/logservice.h"
#include "../services/settingsservice.h"

#include <QObject>
#include <QString>

class ExportPage;

/*
 * 导出控制器。
 *
 * 负责：
 * - 接收 ExportPage 的导出请求；
 * - 调用 ExportService 生成报表；
 * - 调用 ExportService 保存文件；
 * - 写导出日志；
 * - 通知 View 显示预览和状态。
 *
 * 不负责：
 * - 直接拼接报表文本；
 * - 直接操作 QTextStream；
 * - 直接控制 UI 布局。
 */
class ExportController : public QObject
{
    Q_OBJECT

public:
    explicit ExportController(ExportPage *view,
                              QObject *parent = nullptr);

    void setCurrentUser(const User& user);

private slots:
    void handleExportReport(const ExportRequest& request);

private:
    void writeExportLog(const ExportRequest& request,
                        const QString& filePath,
                        const QString& result,
                        const QString& message);

    QString currentOperatorName() const;

private:
    ExportPage *view;
    User currentUser;

    ExportService exportService;
    SettingsService settingsService;
    LogService logService;
};

#endif // EXPORTCONTROLLER_H
