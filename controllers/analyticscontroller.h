#ifndef ANALYTICSCONTROLLER_H
#define ANALYTICSCONTROLLER_H

#include "../models/analyticsmodels.h"
#include "../models/user.h"
#include "../services/analyticsservice.h"

#include <QObject>
#include <QString>

class AnalyticsPage;

/*
 * 数据分析控制器。
 *
 * 负责连接 AnalyticsPage 和 AnalyticsService：
 * - 保存当前登录用户；
 * - 接收页面发出的统计请求；
 * - 给筛选条件补充数据权限；
 * - 校验筛选条件；
 * - 调用 AnalyticsService 查询统计数据；
 * - 将完整 AnalyticsReport 返回给页面显示。
 */
class AnalyticsController : public QObject
{
    Q_OBJECT

public:
    explicit AnalyticsController(AnalyticsPage *view,
                                 QObject *parent = nullptr);

    void setCurrentUser(const User& user);

private slots:
    void handleReportRequested(const AnalyticsFilter& filter);

private:
    bool validateFilter(const AnalyticsFilter& filter,
                        QString& message) const;

    AnalyticsFilter attachPermissionFilter(const AnalyticsFilter& filter) const;

private:
    AnalyticsPage *view;
    User currentUser;

    AnalyticsService analyticsService;
};

#endif // ANALYTICSCONTROLLER_H
