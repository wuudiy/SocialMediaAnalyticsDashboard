#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include "../models/operationlog.h"
#include "../repositories/logrepository.h"

#include <QDateTime>
#include <QList>
#include <QString>

/*
 * 系统日志服务。
 *
 * 负责：
 * - 标准化日志字段；
 * - 写入用户关键操作日志；
 * - 写入系统日志；
 * - 对外提供日志查询接口。
 *
 * 注意：
 * SQL 已经迁移到 LogRepository，
 * LogService 不再直接访问 DatabaseManager / QSqlQuery。
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

    QList<OperationLog> findLogs(const OperationLogFilter& filter);

    /*
     * 兼容旧调用方式。
     *
     * 其他模块暂时如果还按旧参数调用 findLogs，
     * 不需要马上联动修改。
     */
    QList<OperationLog> findLogs(const QString& usernameKeyword = QString(),
                                 const QString& action = QString(),
                                 const QDateTime& startTime = QDateTime(),
                                 const QDateTime& endTime = QDateTime(),
                                 int limit = 200);

private:
    LogRepository logRepository;
};

#endif // LOGSERVICE_H
