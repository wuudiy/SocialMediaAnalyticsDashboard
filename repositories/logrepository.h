#ifndef LOGREPOSITORY_H
#define LOGREPOSITORY_H

#include "../models/operationlog.h"

#include <QList>
#include <QString>
#include <QVariant>

class QSqlQuery;

/*
 * 操作日志数据访问层。
 *
 * 只负责 operation_logs 表的 SQL：
 * - 插入日志；
 * - 按条件查询日志；
 * - 从 QSqlQuery 构造 OperationLog。
 *
 * 不负责：
 * - 当前登录用户；
 * - 页面提示；
 * - 日期范围是否合法；
 * - 业务动作含义。
 */
class LogRepository
{
public:
    LogRepository();

    bool insertLog(const QVariant& userId,
                   const QString& username,
                   const QString& action,
                   const QString& detail,
                   const QString& result);

    QList<OperationLog> findLogs(const OperationLogFilter& filter);

private:
    OperationLog buildLogFromQuery(const QSqlQuery& query) const;
};

#endif // LOGREPOSITORY_H
