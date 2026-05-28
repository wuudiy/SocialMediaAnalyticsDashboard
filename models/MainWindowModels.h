#ifndef MAINWINDOWMODELS_H
#define MAINWINDOWMODELS_H

/*
 * 主窗口页面枚举。
 *
 * 用枚举代替散落的字符串和 QWidget 判断，
 * 让 Controller 可以用统一方式处理导航和权限。
 */
enum class MainWindowPage
{
    Dashboard,
    PostManagement,
    Analytics,
    Export,
    UserManagement,
    OperationLogs,
    Settings
};

#endif // MAINWINDOWMODELS_H
