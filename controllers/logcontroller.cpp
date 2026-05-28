#include "logcontroller.h"

#include "../views/logpage.h"

LogController::LogController(LogPage *view,
                             QObject *parent)
    : QObject(parent),
    view(view)
{
    /*
     * Controller 接管日志查询请求。
     *
     * View 只负责把筛选条件发出来；
     * 查询、日期校验和错误处理由 Controller 完成。
     */
    connect(view, &LogPage::searchLogsRequested,
            this, &LogController::handleSearchLogs);
}

void LogController::handleSearchLogs(const OperationLogFilter& filter)
{
    QString message;

    if (!validateFilter(filter, message)) {
        view->showMessage(message, true);
        view->showWarningMessage(QStringLiteral("Invalid Filter"), message);
        return;
    }

    const QList<OperationLog> logs = logService.findLogs(filter);

    view->showLogs(logs);
}

bool LogController::validateFilter(const OperationLogFilter& filter,
                                   QString& message) const
{
    if (filter.startTime.isValid()
        && filter.endTime.isValid()
        && filter.startTime > filter.endTime) {
        message = QStringLiteral("Start time cannot be later than end time.");
        return false;
    }

    return true;
}
