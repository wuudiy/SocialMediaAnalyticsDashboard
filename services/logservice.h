#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include <QDateTime>
#include <QList>
#include <QString>
#include <QSqlQuery>

/*
 * 一条操作日志记录。
 *
 * 这里不用单独放到 models 目录，是因为日志只服务于 LogService 和后续日志页面，
 * 不像 User、Post 那样属于核心业务数据模型。
 */
struct OperationLog
{
    int logId = -1;
    int userId = -1;
    QString username;
    QString action;
    QString detail;
    QString result;
    QDateTime createdAt;
};

/*
 * 系统日志服务。
 *
 * 负责：
 * - 写入用户关键操作日志；
 * - 查询操作日志；
 * - 给后续管理员日志页面提供数据。
 *
 * 不负责：
 * - 弹窗提示；
 * - 页面控件创建；
 * - 判断用户是否为管理员。
 */
class LogService
{
public:
    LogService();

    // 写入一条日志。result 建议使用 success 或 failed。
    bool writeLog(int userId,
                  const QString& username,
                  const QString& action,
                  const QString& detail,
                  const QString& result = QStringLiteral("success"));

    // 写入系统级日志。没有具体用户时使用这个函数。
    bool writeSystemLog(const QString& action,
                        const QString& detail,
                        const QString& result = QStringLiteral("success"));

    // 查询日志。usernameKeyword 和 action 为空时表示不过滤。
    QList<OperationLog> findLogs(const QString& usernameKeyword = QString(),
                                 const QString& action = QString(),
                                 int limit = 200);

private:
    // 把 SQL 查询结果当前行转换成 OperationLog。
    OperationLog buildLogFromQuery(const QSqlQuery& query) const;
};

#endif // LOGSERVICE_H
