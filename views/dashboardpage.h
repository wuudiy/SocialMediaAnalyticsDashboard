#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include "../models/user.h"

#include <QWidget>

class QLabel;
class QFrame;
class QGridLayout;

/*
 * 登录后的首页页面。
 *
 * 负责：
 * - 展示欢迎信息；
 * - 展示当前预留的数据模块入口；
 * - 展示图表和最近内容的占位区域。
 *
 * 不负责：
 * - 查询真实统计数据；
 * - 操作数据库；
 * - 处理页面跳转。
 */
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);

    // MainWindow 登录成功后传入当前用户，用来刷新欢迎文案。
    void setCurrentUser(const User& user);

private:
    // 创建页面整体布局。
    void buildUi();

    // 当前页面的统一样式。
    void applyStyleSheet();

    // 创建顶部四个功能模块卡片区域。
    QGridLayout* createModuleGrid();

    // 创建中间两个图表占位区域。
    QGridLayout* createPanelGrid();

    // 创建一个功能模块卡片。
    QFrame* createModuleCard(const QString& title,
                             const QString& description);

    // 创建一个空状态面板，用于暂未接入数据的区域。
    QFrame* createEmptyPanel(const QString& title,
                             const QString& description,
                             int minimumHeight = 220);

private:
    QLabel *welcomeLabel;
    QLabel *subtitleLabel;
};

#endif // DASHBOARDPAGE_H
