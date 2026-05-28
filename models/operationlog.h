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

#endif // OPERATIONLOG_H
