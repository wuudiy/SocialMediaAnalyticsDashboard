#include "logrepository.h"

#include "../infrastructure/databasemanager.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QtGlobal>

LogRepository::LogRepository()
{
}

bool LogRepository::insertLog(const QVariant& userId,
                              const QString& username,
                              const QString& action,
                              const QString& detail,
                              const QString& result)
{
    QSqlQuery query(DatabaseManager::database());

    query.prepare(
        QStringLiteral(
            "INSERT INTO operation_logs "
            "(user_id, username, action, detail, result) "
            "VALUES "
            "(:user_id, :username, :action, :detail, :result)"
            )
        );

    query.bindValue(QStringLiteral(":user_id"), userId);
    query.bindValue(QStringLiteral(":username"), username);
    query.bindValue(QStringLiteral(":action"), action);
    query.bindValue(QStringLiteral(":detail"), detail);
    query.bindValue(QStringLiteral(":result"), result);

    if (!query.exec()) {
        qDebug() << "Insert operation log failed:" << query.lastError().text();
        return false;
    }

    return true;
}

QList<OperationLog> LogRepository::findLogs(const OperationLogFilter& filter)
{
    QList<OperationLog> logs;

    const QString trimmedUsernameKeyword = filter.usernameKeyword.trimmed();
    const QString trimmedAction = filter.action.trimmed();

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

    if (filter.startTime.isValid()) {
        sql += "AND created_at >= :start_time ";
    }

    if (filter.endTime.isValid()) {
        sql += "AND created_at <= :end_time ";
    }

    sql += QStringLiteral("ORDER BY created_at DESC, log_id DESC LIMIT %1")
               .arg(qMax(1, filter.limit));

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

    if (filter.startTime.isValid()) {
        query.bindValue(QStringLiteral(":start_time"), filter.startTime);
    }

    if (filter.endTime.isValid()) {
        query.bindValue(QStringLiteral(":end_time"), filter.endTime);
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

OperationLog LogRepository::buildLogFromQuery(const QSqlQuery& query) const
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
