#ifndef DASHBOARDCONTROLLER_H
#define DASHBOARDCONTROLLER_H

#include "../models/user.h"
#include "../services/dashboardservice.h"

#include <QObject>

class DashboardPage;

/*
 * Dashboard 控制器。
 *
 * 负责：
 * - 保存当前登录用户；
 * - 接收 DashboardPage 的刷新请求；
 * - 调用 DashboardService 获取页面数据；
 * - 将 DashboardViewModel 返回给页面显示。
 *
 * 不负责：
 * - 创建 UI 控件；
 * - 绘制图表；
 * - 直接写 SQL；
 * - 直接访问 DatabaseManager。
 */
class DashboardController : public QObject
{
    Q_OBJECT

public:
    explicit DashboardController(DashboardPage *view,
                                 QObject *parent = nullptr);

    void setCurrentUser(const User& user);

private slots:
    void handleRefreshDashboard();

private:
    DashboardPage *view;
    User currentUser;

    DashboardService dashboardService;
};

#endif // DASHBOARDCONTROLLER_H
