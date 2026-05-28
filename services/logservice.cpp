#include "logservice.h"

#include <QDebug>
#include <QVariant>

LogService::LogService()
{
}

bool LogService::writeLog(int userId,
                          const QString& username,
                          const QString& action,
                          const QString& detail,
                          const QString& result)
{
    const QString trimmedUsername = username.trimmed().isEmpty()
    ? QStringLiteral("system")
    : username.trimmed();

    const QString trimmedAction = action.trimmed();

    const QString trimmedResult = result.trimmed().isEmpty()
                                      ? QStringLiteral("success")
                                      : result.trimmed();

    if (trimmedAction.isEmpty()) {
        qDebug() << "Write operation log failed: action is empty";
        return false;
    }

    QVariant userIdValue;

    if (userId > 0) {
        userIdValue = userId;
    } else {
        userIdValue = QVariant();
    }

    /*
     * 字段清洗和默认值处理放在 Service。
     * 真正的 INSERT SQL 交给 Repository。
     */
    return logRepository.insertLog(
        userIdValue,
        trimmedUsername,
        trimmedAction,
        detail.trimmed(),
        trimmedResult
        );
}

bool LogService::writeSystemLog(const QString& action,
                                const QString& detail,
                                const QString& result)
{
    return writeLog(
        -1,
        QStringLiteral("system"),
        action,
        detail,
        result
        );
}

QList<OperationLog> LogService::findLogs(const OperationLogFilter& filter)
{
    OperationLogFilter cleanFilter = filter;

    cleanFilter.usernameKeyword = filter.usernameKeyword.trimmed();
    cleanFilter.action = filter.action.trimmed();

    if (cleanFilter.limit <= 0) {
        cleanFilter.limit = 200;
    }

    return logRepository.findLogs(cleanFilter);
}

QList<OperationLog> LogService::findLogs(const QString& usernameKeyword,
                                         const QString& action,
                                         const QDateTime& startTime,
                                         const QDateTime& endTime,
                                         int limit)
{
    OperationLogFilter filter;

    filter.usernameKeyword = usernameKeyword;
    filter.action = action;
    filter.startTime = startTime;
    filter.endTime = endTime;
    filter.limit = limit;

    return findLogs(filter);
}
