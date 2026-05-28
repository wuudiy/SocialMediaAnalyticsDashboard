#include "analyticscontroller.h"

#include "../views/analyticspage.h"

AnalyticsController::AnalyticsController(AnalyticsPage *view,
                                         QObject *parent)
    : QObject(parent),
    view(view)
{
    /*
     * Controller 接管 AnalyticsPage 的数据请求。
     *
     * View 只负责发出：
     * “当前筛选条件是什么，需要刷新报表”
     *
     * 具体的数据查询、日期校验、数据权限都由 Controller / Service 处理。
     */
    connect(view, &AnalyticsPage::reportRequested,
            this, &AnalyticsController::handleReportRequested);
}

void AnalyticsController::setCurrentUser(const User& user)
{
    currentUser = user;
}

void AnalyticsController::handleReportRequested(const AnalyticsFilter& filter)
{
    /*
     * 如果页面在 MainWindow 完成 setCurrentUser() 之前触发刷新，
     * 这里不要弹窗。
     *
     * 这不是用户筛选条件错误，而是页面初始化阶段还没有注入当前用户。
     * 等登录成功后，MainWindow 会调用 AnalyticsPage::setCurrentUser()，
     * 到时候会自动重新刷新一次。
     */
    if (!currentUser.isValid()) {
        return;
    }

    const AnalyticsFilter permissionFilter = attachPermissionFilter(filter);

    QString message;

    if (!validateFilter(permissionFilter, message)) {
        view->showWarningMessage(
            QStringLiteral("Invalid Filter"),
            message
            );

        return;
    }

    const AnalyticsReport report = analyticsService.generateReport(permissionFilter);

    view->showReport(report);
}

bool AnalyticsController::validateFilter(const AnalyticsFilter& filter,
                                         QString& message) const
{
    if (filter.startDate.isValid()
        && filter.endDate.isValid()
        && filter.startDate > filter.endDate) {
        message = QStringLiteral("Start date cannot be later than end date.");
        return false;
    }

    if (!filter.includeAllUsers && filter.ownerUserId <= 0) {
        message = QStringLiteral("Current user is invalid. Please login again.");
        return false;
    }

    return true;
}

AnalyticsFilter AnalyticsController::attachPermissionFilter(const AnalyticsFilter& filter) const
{
    AnalyticsFilter cleanFilter = filter;

    /*
     * 数据隔离规则：
     * - admin：includeAllUsers = true，统计全部数据；
     * - 普通 user：includeAllUsers = false，只统计当前用户的数据；
     * - 未登录：按普通用户处理，ownerUserId 无效，validateFilter 会拦截。
     */
    cleanFilter.includeAllUsers = currentUser.isValid() && currentUser.isAdmin();
    cleanFilter.ownerUserId = currentUser.isValid() ? currentUser.userId : -1;

    return cleanFilter;
}
