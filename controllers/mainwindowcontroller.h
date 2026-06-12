#ifndef MAINWINDOWCONTROLLER_H
#define MAINWINDOWCONTROLLER_H

#include "../models/mainwindowmodels.h"
#include "../models/user.h"

#include <QObject>
#include <QString>

class MainWindow;

/*
 * 主窗口控制器。
 *
 * 负责：
 * - 保存当前登录用户；
 * - 判断管理员权限；
 * - 控制哪些页面可以访问；
 * - 生成窗口标题和用户信息文本；
 * - 接收导航请求并通知 MainWindow 显示页面；
* - 处理注销并返回登录页流程。
 *
 * 不负责：
 * - 创建 UI 控件；
 * - 创建子页面；
 * - 设置按钮样式；
 * - 直接操作 QStackedWidget。
 */
class MainWindowController : public QObject
{
    Q_OBJECT

public:
    explicit MainWindowController(MainWindow *view,
                                  QObject *parent = nullptr);

private slots:
    void handleCurrentUserChanged(const User& user);

    void handleNavigationRequested(MainWindowPage page);

    void handleLogoutRequested();

private:
    bool isAdminUser() const;

    bool canOpenPage(MainWindowPage page) const;

    QString pageTitle(MainWindowPage page) const;

    QString accessDeniedMessage(MainWindowPage page) const;

    QString windowTitleForUser(const User& user) const;

    QString userInfoTextForUser(const User& user) const;

private:
    MainWindow *view;

    User currentUser;
};

#endif // MAINWINDOWCONTROLLER_H
