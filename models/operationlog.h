#ifndef OPERATIONLOG_H
#define OPERATIONLOG_H

#include <QDateTime>
#include <QString>

/*
 * 一条操作日志记录。
 *
 * 日志模块用于记录用户关键操作，例如：
 * - 登录成功 / 登录失败；
 * - 管理员创建用户；
 * - 新增、修改、删除帖子；
 * - CSV 导入；
 * - 报表导出。
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
 * 日志查询条件。
 *
 * usernameKeyword 为空：不按用户名过滤。
 * action 为空：不按操作类型过滤。
 * startTime 无效：不限制开始时间。
 * endTime 无效：不限制结束时间。
 * limit 小于等于 0：Repository 内部会修正为安全默认值。
 */
struct OperationLogFilter
{
    QString usernameKeyword;
    QString action;
    QDateTime startTime;
    QDateTime endTime;
    int limit = 300;
};

#endif // OPERATIONLOG_H
