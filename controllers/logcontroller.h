#ifndef LOGCONTROLLER_H
#define LOGCONTROLLER_H

#include "../models/operationlog.h"
#include "../services/logservice.h"

#include <QObject>
#include <QString>

class LogPage;

/*
 * 日志页面控制器。
 *
 * 负责：
 * - 接收 LogPage 的查询请求；
 * - 校验日期范围；
 * - 调用 LogService 查询日志；
 * - 通知 View 显示结果或错误提示。
 *
 * 不负责：
 * - 初始化 UI；
 * - 填充表格单元格；
 * - 直接写 SQL；
 * - 直接访问 DatabaseManager。
 */
class LogController : public QObject
{
    Q_OBJECT

public:
    explicit LogController(LogPage *view,
                           QObject *parent = nullptr);

private slots:
    void handleSearchLogs(const OperationLogFilter& filter);

private:
    bool validateFilter(const OperationLogFilter& filter,
                        QString& message) const;

private:
    LogPage *view;
    LogService logService;
};

#endif // LOGCONTROLLER_H
