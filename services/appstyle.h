#ifndef APPSTYLE_H
#define APPSTYLE_H

#include <QString>

/*
 * 公共样式类。
 *
 * 负责：
 * - 管理项目内可复用 QSS；
 * - 减少页面中的重复样式；
 * - 统一窗口、卡片、输入框、按钮、表格、导航按钮风格。
 *
 * 不负责：
 * - 创建控件；
 * - 管理页面布局；
 * - 处理业务逻辑；
 * - 连接信号槽。
 *
 * 设计原则：
 * AppStyle 只负责“长什么样”，页面类负责“有什么控件、怎么交互”。
 */
class AppStyle
{
public:
    // 基础窗口样式。
    static QString windowStyle();

    // 主窗口样式：左侧导航、顶部栏、退出按钮、占位页。
    static QString mainWindowStyle();

    // 独立控件样式。
    static QString titleStyle();
    static QString subtitleStyle();
    static QString cardStyle();
    static QString inputStyle();
    static QString primaryButtonStyle();
    static QString secondaryButtonStyle();
    static QString dangerButtonStyle();
    static QString successButtonStyle();
    static QString tableStyle();

    // 左侧导航按钮样式，保留给后续需要单独设置按钮样式的地方使用。
    static QString navigationButtonStyle(bool active);

    // 数据管理类页面样式，例如 PostManagementPage、UserManagementPage、LogPage。
    static QString dataManagementPageStyle();

    // Dashboard 首页样式。
    static QString dashboardPageStyle();

    // 消息提示样式。
    static QString messageLabelStyle(bool error);
};

#endif // APPSTYLE_H
