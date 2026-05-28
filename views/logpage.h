#ifndef LOGPAGE_H
#define LOGPAGE_H

#include "../models/operationlog.h"

#include <QDateTime>
#include <QList>
#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class LogPage;
}
QT_END_NAMESPACE

class LogController;
class QTableWidgetItem;

/*
 * 管理员操作日志页面。
 *
 * MVC 重构后，本类只负责 View 层：
 * - 初始化筛选控件；
 * - 收集用户名、操作类型、日期范围；
 * - 显示 operation_logs 表格；
 * - 显示页面提示和警告弹窗；
 * - 发出日志查询信号。
 *
 * 不再负责：
 * - 直接调用 LogService；
 * - 直接查询数据库；
 * - 直接处理日志查询业务。
 */
class LogPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogPage(QWidget *parent = nullptr);
    ~LogPage();

public slots:
    // 主窗口切换到日志页时可调用，保证显示最新日志。
    void refreshLogs();

    // Controller 调用：显示日志查询结果。
    void showLogs(const QList<OperationLog>& logs);

    // Controller 调用：显示页面底部提示信息。
    void showMessage(const QString& message,
                     bool error = false);

    // Controller 调用：统一显示警告弹窗。
    void showWarningMessage(const QString& title,
                            const QString& message);

signals:
    void searchLogsRequested(const OperationLogFilter& filter);

private slots:
    void onSearchClicked();
    void onResetClicked();

private:
    // 初始化 .ui 中控件的运行时属性。
    void prepareUiObjects();

    // 连接 .ui 中控件的信号槽。
    void connectSignals();

    // 应用统一页面样式。
    void applyStyleSheet();

    // 初始化日志表格列名、选择模式和列宽。
    void setupTable();

    // 把 Controller 返回的日志结果填充到表格。
    void fillTable(const QList<OperationLog>& logs);

    // 读取当前筛选条件。
    OperationLogFilter readFilterFromUi() const;

    // 重置筛选控件。
    void resetFilterControls();

    // 获取当前选择的操作类型。
    QString selectedAction() const;

    // 获取查询开始时间；未勾选日期范围时返回无效时间。
    QDateTime startDateTime() const;

    // 获取查询结束时间；未勾选日期范围时返回无效时间。
    QDateTime endDateTime() const;

    // 创建表格单元格，减少重复代码。
    QTableWidgetItem *createTableItem(const QString& text,
                                      Qt::Alignment alignment = Qt::AlignCenter) const;

private:
    Ui::LogPage *ui;

    LogController *logController;
};

#endif // LOGPAGE_H
