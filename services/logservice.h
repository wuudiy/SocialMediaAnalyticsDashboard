#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include "../models/operationlog.h"

#include <QDateTime>
#include <QList>
#include <QString>

class QSqlQuery;

/*
 * 系统日志服务。
 *
 * 负责：
 * - 写入用户关键操作日志；
 * - 查询操作日志；
 * - 给管理员日志页面提供数据。
 */
class LogService
{
public:
    LogService();

    bool writeLog(int userId,
                  const QString& username,
                  const QString& action,
                  const QString& detail,
                  const QString& result = QStringLiteral("success"));

    bool writeSystemLog(const QString& action,
                        const QString& detail,
                        const QString& result = QStringLiteral("success"));

    /*
     * 查询操作日志。
     *
     * usernameKeyword 为空：不按用户名过滤。
     * action 为空：不按操作类型过滤。
     * startTime 无效：不限制开始时间。
     * endTime 无效：不限制结束时间。
     */
    QList<OperationLog> findLogs(const QString& usernameKeyword = QString(),
                                 const QString& action = QString(),
                                 const QDateTime& startTime = QDateTime(),
                                 const QDateTime& endTime = QDateTime(),
                                 int limit = 200);

private:
    OperationLog buildLogFromQuery(const QSqlQuery& query) const;
};

#endif // LOGSERVICE_H
