#ifndef LOGPAGE_H
#define LOGPAGE_H

#include "../services/logservice.h"

#include <QDateTime>
#include <QList>
#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class LogPage;
}
QT_END_NAMESPACE

/*
 * 管理员操作日志页面。
 *
 * 当前重构目标：
 * - forms/logpage.ui 负责固定界面结构；
 * - views/logpage.cpp 负责日志查询、表格刷新、筛选逻辑和信号槽；
 * - services/LogService 继续负责数据库日志查询，不在本次修改。
 *
 * 页面功能：
 * - 展示 operation_logs 表中的日志；
 * - 按用户名关键字查询；
 * - 按操作类型查询；
 * - 按日期范围查询；
 * - 支持刷新和重置查询条件。
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

    // 把 LogService 查询结果填充到表格。
    void fillTable(const QList<OperationLog>& logs);

    // 获取当前选择的操作类型。
    QString selectedAction() const;

    // 获取查询开始时间；未勾选日期范围时返回无效时间。
    QDateTime startDateTime() const;

    // 获取查询结束时间；未勾选日期范围时返回无效时间。
    QDateTime endDateTime() const;

    // 页面底部提示信息。
    void setMessage(const QString& message,
                    bool error = false);

private:
    Ui::LogPage *ui;

    LogService logService;
};

#endif // LOGPAGE_H
