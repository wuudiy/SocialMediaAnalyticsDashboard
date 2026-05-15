#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include "../models/user.h"
#include "../services/analyticsservice.h"

#include <QWidget>

class QLabel;
class QFrame;
class QGridLayout;
class QTableWidget;

/*
 * 登录后的首页页面。
 *
 * 负责：
 * - 展示欢迎信息；
 * - 展示帖子总量、互动量、互动率等核心指标；
 * - 展示热门帖子和最近帖子列表。
 *
 * 不负责：
 * - 新增或删除帖子；
 * - 直接写 SQL；
 * - 管理主窗口导航。
 */
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);

    // MainWindow 登录成功后传入当前用户，用来刷新欢迎文案。
    void setCurrentUser(const User& user);

public slots:
    // 数据变化或切回首页时调用，让首页显示最新统计。
    void refreshDashboard();

private:
    // 创建页面整体布局。
    void buildUi();

    // 当前页面的统一样式。
    void applyStyleSheet();

    // 创建顶部统计卡片区域。
    QGridLayout* createSummaryGrid();

    // 创建一个统计卡片，valueLabel 会保存下来方便刷新数据。
    QFrame* createSummaryCard(const QString& title,
                              QLabel **valueLabel,
                              const QString& description);

    // 创建热门帖子信息卡片。
    QFrame* createTopPostCard();

    // 创建最近帖子表格卡片。
    QFrame* createRecentPostsCard();

    // 初始化最近帖子表格。
    void setupRecentPostsTable();

    // 把最近帖子填入表格。
    void fillRecentPostsTable(const QList<Post>& posts);

    // 格式化百分比。
    QString formatPercent(double value) const;

private:
    AnalyticsService analyticsService;

    QLabel *welcomeLabel;
    QLabel *subtitleLabel;

    QLabel *totalPostsValueLabel;
    QLabel *totalInteractionsValueLabel;
    QLabel *totalViewsValueLabel;
    QLabel *engagementRateValueLabel;

    QLabel *topPostTitleLabel;
    QLabel *topPostDetailLabel;

    QTableWidget *recentPostsTable;
};

#endif // DASHBOARDPAGE_H
