#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include "../models/post.h"
#include "../models/user.h"
#include "../services/analyticsservice.h"

#include <QList>
#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class DashboardPage;
}
QT_END_NAMESPACE

/*
 * 登录后的 Dashboard 首页。
 *
 * 当前重构后的分工：
 *
 * 1. forms/dashboardpage.ui
 *    负责固定界面结构：
 *    - 欢迎标题；
 *    - 副标题；
 *    - 4 个统计卡片；
 *    - Top Post 卡片；
 *    - Recent Posts 表格卡片。
 *
 * 2. views/dashboardpage.cpp
 *    负责运行时逻辑：
 *    - 初始化控件 objectName；
 *    - 初始化表格列；
 *    - 从 AnalyticsService 读取统计数据；
 *    - 刷新统计卡片；
 *    - 刷新最近帖子表格。
 *
 * 3. services/AnalyticsService
 *    负责真正的数据统计和数据库查询。
 *
 * 这样 DashboardPage.cpp 不再手写大量 new QLabel / new QFrame /
 * new QGridLayout / addWidget 代码，后续可以直接用 Qt Designer 调整首页界面。
 */
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);
    ~DashboardPage();

    // MainWindow 登录成功后传入当前用户，用于刷新欢迎文案。
    void setCurrentUser(const User& user);

public slots:
    // 数据变化或切回首页时调用，保证首页统计是最新数据。
    void refreshDashboard();

private:
    // 初始化 .ui 中控件的运行时属性，例如 objectName、表格样式、文字换行。
    void prepareUiObjects();

    // 应用 Dashboard 页面统一样式。
    void applyStyleSheet();

    // 初始化最近帖子表格。
    void setupRecentPostsTable();

    // 把最近帖子数据填入表格。
    void fillRecentPostsTable(const QList<Post>& posts);

    // 百分比格式化工具，例如 0.1234 -> 12.34%。
    QString formatPercent(double value) const;

private:
    Ui::DashboardPage *ui;

    AnalyticsService analyticsService;
};

#endif // DASHBOARDPAGE_H
