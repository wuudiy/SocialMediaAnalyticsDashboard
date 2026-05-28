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
    QString message;

    if (!exportService.validateRequest(request, message)) {
        view->showStatus(message, false);
        view->showWarningMessage(QStringLiteral("Export Failed"), message);

        writeExportLog(request,
                       QString(),
                       QStringLiteral("failed"),
                       message);
        return;
    }

    const QString content = exportService.generateReport(request);

    /*
     * 预览先显示出来，即使用户后面取消保存，也能看到当前报表内容。
     */
    view->showPreview(content);

    const QString extension = ExportService::extensionForFormat(request.format);
    const QString suggestedPath = settingsService.defaultExportFilePath(
        request.baseFileName,
        extension
        );

    const QString filePath = view->selectExportFilePath(
        suggestedPath,
        extension
        );

    if (filePath.trimmed().isEmpty()) {
        message = QStringLiteral("Export cancelled.");
        view->showStatus(message, false);

        writeExportLog(request,
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

        writeExportLog(request,
                       filePath,
                       QStringLiteral("failed"),
                       saveResult.message);
        return;
    }

    view->showStatus(
        QStringLiteral("%1 report exported successfully!")
            .arg(ExportService::displayNameForFormat(request.format)),
        true
        );

    writeExportLog(request,
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

    const QString detail = QStringLiteral("Format: %1, Platform: %2, Date Range: %3 to %4, File: %5, Message: %6")
                               .arg(formatName,
                                    platform,
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
