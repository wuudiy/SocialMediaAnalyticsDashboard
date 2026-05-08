#ifndef APPSTYLE_H
#define APPSTYLE_H

#include <QString>

/*
 * 公共样式类。
 *
 * 负责：
 * - 管理项目内可复用 QSS；
 * - 减少页面中的重复样式；
 * - 统一窗口、卡片、输入框、按钮、导航按钮风格。
 *
 * 不负责：
 * - 创建控件；
 * - 管理布局；
 * - 处理业务逻辑。
 */
class AppStyle
{
public:
    static QString windowStyle();

    static QString titleStyle();
    static QString subtitleStyle();

    static QString cardStyle();

    static QString inputStyle();
    static QString primaryButtonStyle();
    static QString secondaryButtonStyle();

    static QString navigationButtonStyle(bool active);
};

#endif // APPSTYLE_H
