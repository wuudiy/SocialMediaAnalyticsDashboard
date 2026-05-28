#ifndef ANALYTICSPAGE_H
#define ANALYTICSPAGE_H

#include "../models/analyticsmodels.h"
#include "../models/post.h"
#include "../models/user.h"

#include <QList>
#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class AnalyticsPage;
}
QT_END_NAMESPACE

class AnalyticsController;
class QTableWidgetItem;

/*
 * 数据分析页面。
 *
 * MVC 重构后，本类只负责 View 层：
 * - 初始化筛选控件；
 * - 读取平台和日期筛选条件；
 * - 显示汇总统计；
 * - 显示平台统计表；
 * - 显示日期趋势表；
 * - 显示热门帖子表；
 * - 发出刷新报表信号。
 *
 * 数据隔离由 AnalyticsController 根据当前用户自动补充，
 * View 不直接判断 admin / user。
 */
class AnalyticsPage : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyticsPage(QWidget *parent = nullptr);
    ~AnalyticsPage();

    // MainWindow 登录成功后调用，用于把当前用户交给 Controller。
    void setCurrentUser(const User& user);

public slots:
    void refreshData();

    void showReport(const AnalyticsReport& report);

    void showWarningMessage(const QString& title,
                            const QString& message);

signals:
    void reportRequested(const AnalyticsFilter& filter);

private slots:
    void onFilterButtonClicked();
    void onRefreshButtonClicked();

private:
    void prepareUiObjects();

    void setupComboBox();

    void setupTables();

    void connectSignals();

    AnalyticsFilter readFilterFromUi() const;

    void resetFilterControls();

    void showSummaryData(const DashboardSummary& summary);

    void showPlatformStatistics(const QList<PlatformStatistics>& statsList);

    void showDateTrends(const QList<DateTrend>& trends);

    void showTopPosts(const QList<Post>& posts);

    QTableWidgetItem *createTableItem(const QString& text,
                                      Qt::Alignment alignment = Qt::AlignCenter) const;

private:
    Ui::AnalyticsPage *ui;

    AnalyticsController *analyticsController;

    AnalyticsFilter currentFilter;
};

#endif // ANALYTICSPAGE_H
