#include "logservice.h"
#include "databasemanager.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QtGlobal>

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

    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        QStringLiteral(
            "INSERT INTO operation_logs "
            "(user_id, username, action, detail, result) "
            "VALUES "
            "(:user_id, :username, :action, :detail, :result)"
            )
        );

    query.bindValue(QStringLiteral(":user_id"), userIdValue);
    query.bindValue(QStringLiteral(":username"), trimmedUsername);
    query.bindValue(QStringLiteral(":action"), trimmedAction);
    query.bindValue(QStringLiteral(":detail"), detail.trimmed());
    query.bindValue(QStringLiteral(":result"), trimmedResult);

    if (!query.exec()) {
        qDebug() << "Write operation log failed:" << query.lastError().text();
        return false;
    }

    return true;
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

QList<OperationLog> LogService::findLogs(const QString& usernameKeyword,
                                         const QString& action,
                                         const QDateTime& startTime,
                                         const QDateTime& endTime,
                                         int limit)
{
    QList<OperationLog> logs;

    const QString trimmedUsernameKeyword = usernameKeyword.trimmed();
    const QString trimmedAction = action.trimmed();

    QString sql =
        "SELECT log_id, user_id, username, action, detail, result, created_at "
        "FROM operation_logs "
        "WHERE 1 = 1 ";

    if (!trimmedUsernameKeyword.isEmpty()) {
        sql += "AND username LIKE :username ";
    }

    if (!trimmedAction.isEmpty()) {
        sql += "AND action = :action ";
    }

    if (startTime.isValid()) {
        sql += "AND created_at >= :start_time ";
    }

    if (endTime.isValid()) {
        sql += "AND created_at <= :end_time ";
    }

    sql += QStringLiteral("ORDER BY created_at DESC, log_id DESC LIMIT %1")
               .arg(qMax(1, limit));

    QSqlQuery query(DatabaseManager::database());
    query.prepare(sql);

    if (!trimmedUsernameKeyword.isEmpty()) {
        query.bindValue(
            QStringLiteral(":username"),
            QStringLiteral("%%1%").arg(trimmedUsernameKeyword)
            );
    }

    if (!trimmedAction.isEmpty()) {
        query.bindValue(QStringLiteral(":action"), trimmedAction);
    }

    if (startTime.isValid()) {
        query.bindValue(QStringLiteral(":start_time"), startTime);
    }

    if (endTime.isValid()) {
        query.bindValue(QStringLiteral(":end_time"), endTime);
    }

    if (!query.exec()) {
        qDebug() << "Find operation logs failed:" << query.lastError().text();
        return logs;
    }

    while (query.next()) {
        logs.append(buildLogFromQuery(query));
    }

    return logs;
}

OperationLog LogService::buildLogFromQuery(const QSqlQuery& query) const
{
    OperationLog log;

    log.logId = query.value(0).toInt();
    log.userId = query.value(1).isNull() ? -1 : query.value(1).toInt();
    log.username = query.value(2).toString();
    log.action = query.value(3).toString();
    log.detail = query.value(4).toString();
    log.result = query.value(5).toString();
    log.createdAt = query.value(6).toDateTime();

    return log;
}
