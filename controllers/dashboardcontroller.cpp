#include "dashboardcontroller.h"

#include "../views/dashboardpage.h"

DashboardController::DashboardController(DashboardPage *view,
                                         QObject *parent)
    : QObject(parent),
    view(view)
{
    /*
     * Controller 接管 DashboardPage 的刷新请求。
     *
     * View 只需要发出 dashboardRefreshRequested()，
     * 具体加载哪些数据、按谁的数据权限加载，
     * 由 Controller / Service 处理。
     */
    connect(view, &DashboardPage::dashboardRefreshRequested,
            this, &DashboardController::handleRefreshDashboard);
}

void DashboardController::setCurrentUser(const User& user)
{
    currentUser = user;
}

void DashboardController::handleRefreshDashboard()
{
    const DashboardViewModel viewModel = dashboardService.loadDashboardData(currentUser);

    view->showDashboard(viewModel);

    view->showMessage(
        currentUser.isValid() && currentUser.isAdmin()
            ? QStringLiteral("Dashboard refreshed. Scope: All users.")
            : QStringLiteral("Dashboard refreshed. Scope: Current user only.")
        );
}
