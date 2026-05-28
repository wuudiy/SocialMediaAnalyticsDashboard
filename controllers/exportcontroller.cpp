#include "exportcontroller.h"

#include "../views/exportpage.h"

#include <QFileInfo>

ExportController::ExportController(ExportPage *view,
                                   QObject *parent)
    : QObject(parent),
    view(view)
{
    /*
     * Controller 接管导出请求。
     *
     * View 只发出“用户要导出 CSV / TXT”，
     * 其余业务全部由 Controller + Service 完成。
     */
    connect(view, &ExportPage::exportReportRequested,
            this, &ExportController::handleExportReport);
}

void ExportController::setCurrentUser(const User& user)
{
    currentUser = user;
}

void ExportController::handleExportReport(const ExportRequest& request)
{
    /*
     * 数据隔离关键点：
     * View 传来的 request 只有平台和日期筛选；
     * 当前用户权限必须由 Controller 补充，不能让 View 判断。
     */
    ExportRequest permissionRequest = request;
    permissionRequest.filter.includeAllUsers = currentUser.isValid() && currentUser.isAdmin();
    permissionRequest.filter.ownerUserId = currentUser.isValid() ? currentUser.userId : -1;

    QString message;

    if (!exportService.validateRequest(permissionRequest, message)) {
        view->showStatus(message, false);
        view->showWarningMessage(QStringLiteral("Export Failed"), message);

        writeExportLog(permissionRequest,
                       QString(),
                       QStringLiteral("failed"),
                       message);
        return;
    }

    const QString content = exportService.generateReport(permissionRequest);

    /*
     * 预览先显示出来，即使用户后面取消保存，也能看到当前报表内容。
     */
    view->showPreview(content);

    const QString extension = ExportService::extensionForFormat(permissionRequest.format);
    const QString suggestedPath = settingsService.defaultExportFilePath(
        permissionRequest.baseFileName,
        extension
        );

    const QString filePath = view->selectExportFilePath(
        suggestedPath,
        extension
        );

    if (filePath.trimmed().isEmpty()) {
        message = QStringLiteral("Export cancelled.");
        view->showStatus(message, false);

        writeExportLog(permissionRequest,
                       QString(),
                       QStringLiteral("failed"),
                       message);
        return;
    }

    const ExportSaveResult saveResult = exportService.saveReportToFile(
        content,
        filePath
        );

    if (!saveResult.success) {
        view->showStatus(saveResult.message, false);
        view->showWarningMessage(QStringLiteral("Export Failed"), saveResult.message);

        writeExportLog(permissionRequest,
                       filePath,
                       QStringLiteral("failed"),
                       saveResult.message);
        return;
    }

    view->showStatus(
        QStringLiteral("%1 report exported successfully!")
            .arg(ExportService::displayNameForFormat(permissionRequest.format)),
        true
        );

    writeExportLog(permissionRequest,
                   saveResult.filePath,
                   QStringLiteral("success"),
                   saveResult.message);
}

void ExportController::writeExportLog(const ExportRequest& request,
                                      const QString& filePath,
                                      const QString& result,
                                      const QString& message)
{
    const QString formatName = ExportService::displayNameForFormat(request.format);

    const QString platform = request.filter.platform.trimmed().isEmpty()
                                 ? QStringLiteral("All")
                                 : request.filter.platform.trimmed();

    const QString scope = request.filter.includeAllUsers
                              ? QStringLiteral("All Users")
                              : QStringLiteral("Current User Only");

    const QString detail = QStringLiteral("Format: %1, Platform: %2, Scope: %3, Date Range: %4 to %5, File: %6, Message: %7")
                               .arg(formatName,
                                    platform,
                                    scope,
                                    request.filter.startDate.toString(QStringLiteral("yyyy-MM-dd")),
                                    request.filter.endDate.toString(QStringLiteral("yyyy-MM-dd")),
                                    QFileInfo(filePath).fileName(),
                                    message);

    if (currentUser.isValid()) {
        logService.writeLog(currentUser.userId,
                            currentOperatorName(),
                            QStringLiteral("Export Report"),
                            detail,
                            result);
        return;
    }

    logService.writeSystemLog(QStringLiteral("Export Report"),
                              detail,
                              result);
}

QString ExportController::currentOperatorName() const
{
    if (currentUser.isValid() && !currentUser.username.trimmed().isEmpty()) {
        return currentUser.username.trimmed();
    }

    return QStringLiteral("unknown");
}
