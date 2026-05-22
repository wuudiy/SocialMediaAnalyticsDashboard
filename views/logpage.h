#ifndef LOGPAGE_H
#define LOGPAGE_H

#include "../services/logservice.h"

#include <QWidget>

class QCheckBox;
class QComboBox;
class QDateEdit;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

/*
 * 管理员操作日志页面。
 *
 * 负责：
 * - 展示 operation_logs 表中的日志；
 * - 按用户名关键字查询；
 * - 按操作类型查询；
 * - 按时间范围查询；
 * - 刷新和重置查询条件。
 *
 * 不负责：
 * - 写入日志；
 * - 判断当前用户是否管理员；
 * - 修改日志数据。
 */
class LogPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogPage(QWidget *parent = nullptr);

public slots:
    void refreshLogs();

private slots:
    void onSearchClicked();
    void onResetClicked();

private:
    void buildUi();
    void applyStyleSheet();

    QFrame* createFilterCard();
    QFrame* createTableCard();

    void setupTable();
    void fillTable(const QList<OperationLog>& logs);

    QString selectedAction() const;
    QDateTime startDateTime() const;
    QDateTime endDateTime() const;

    void setMessage(const QString& message,
                    bool error = false);

private:
    LogService logService;

    QLabel *messageLabel;

    QLineEdit *usernameLineEdit;
    QComboBox *actionComboBox;
    QCheckBox *dateRangeCheckBox;
    QDateEdit *startDateEdit;
    QDateEdit *endDateEdit;
    QPushButton *searchButton;
    QPushButton *resetButton;
    QPushButton *refreshButton;

    QTableWidget *logTable;
};

#endif // LOGPAGE_H
