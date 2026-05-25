#include "appstyle.h"

namespace
{
/*
 * 按钮基础样式生成函数。
 *
 * 作用：
 * - 避免 primary / secondary / danger 按钮重复写相同的 QSS；
 * - 统一按钮高度、圆角、字体粗细；
 * - 只通过颜色参数区分不同按钮类型。
 */
QString buttonBaseStyle(const QString& objectName,
                        const QString& background,
                        const QString& hoverBackground,
                        const QString& textColor,
                        const QString& border = QStringLiteral("none"))
{
    return QStringLiteral(
               "QPushButton#%1 {"
               "    min-height: 34px;"
               "    background: %2;"
               "    color: %3;"
               "    border: %4;"
               "    border-radius: 8px;"
               "    padding: 0 16px;"
               "    font-weight: 600;"
               "}"
               "QPushButton#%1:hover {"
               "    background: %5;"
               "}"
               )
        .arg(objectName, background, textColor, border, hoverBackground);
}
}

QString AppStyle::windowStyle()
{
    return QStringLiteral(
        "QWidget {"
        "    background-color: #F5F7FB;"
        "    color: #1F2937;"
        "    font-family: \"Microsoft YaHei\";"
        "    font-size: 14px;"
        "}"
        );
}

/*
 * 主窗口样式。
 *
 * 包含：
 * - 左侧深色导航栏；
 * - 顶部白色信息栏；
 * - 当前页面标题；
 * - 当前登录用户信息；
 * - 导航按钮默认状态和选中状态；
 * - Logout 按钮；
 * - Analytics / Settings 等占位页面卡片。
 */
QString AppStyle::mainWindowStyle()
{
    return QStringLiteral(
        "QWidget#sideBar {"
        "    background: #111827;"
        "}"
        "QWidget#mainArea {"
        "    background: #F3F4F6;"
        "}"
        "QWidget#topBar {"
        "    background: #FFFFFF;"
        "    border-bottom: 1px solid #E5E7EB;"
        "}"

        "QLabel#appName {"
        "    color: #FFFFFF;"
        "    font-size: 18px;"
        "    font-weight: 700;"
        "}"
        "QLabel#appSubTitle {"
        "    color: #9CA3AF;"
        "    font-size: 12px;"
        "}"
        "QLabel#pageTitleLabel {"
        "    color: #111827;"
        "    font-size: 18px;"
        "    font-weight: 700;"
        "}"
        "QLabel#userInfoLabel {"
        "    color: #374151;"
        "    font-size: 13px;"
        "}"

        "QPushButton#navButton {"
        "    color: #D1D5DB;"
        "    background: transparent;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 14px;"
        "    text-align: left;"
        "    font-size: 14px;"
        "}"
        "QPushButton#navButton:hover {"
        "    background: #1F2937;"
        "    color: #FFFFFF;"
        "}"
        "QPushButton#activeNavButton {"
        "    color: #FFFFFF;"
        "    background: #2563EB;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 14px;"
        "    text-align: left;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "}"

        "QPushButton#logoutButton {"
        "    color: #374151;"
        "    background: #F3F4F6;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 8px;"
        "    padding: 8px 14px;"
        "}"
        "QPushButton#logoutButton:hover {"
        "    background: #E5E7EB;"
        "}"

        "QFrame#placeholderCard {"
        "    background: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 12px;"
        "}"
        "QLabel#placeholderTitle {"
        "    color: #111827;"
        "    font-size: 24px;"
        "    font-weight: 700;"
        "}"
        "QLabel#placeholderText {"
        "    color: #6B7280;"
        "    font-size: 14px;"
        "}"
        );
}

QString AppStyle::titleStyle()
{
    return QStringLiteral(
        "QLabel {"
        "    color: #111827;"
        "    font-size: 24px;"
        "    font-weight: bold;"
        "}"
        );
}

QString AppStyle::subtitleStyle()
{
    return QStringLiteral(
        "QLabel {"
        "    color: #6B7280;"
        "    font-size: 13px;"
        "}"
        );
}

QString AppStyle::cardStyle()
{
    return QStringLiteral(
        "QFrame {"
        "    background: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 12px;"
        "}"
        );
}

QString AppStyle::inputStyle()
{
    return QStringLiteral(
        "QLineEdit, QComboBox, QDateEdit, QSpinBox {"
        "    min-height: 32px;"
        "    border: 1px solid #D1D5DB;"
        "    border-radius: 8px;"
        "    padding-left: 8px;"
        "    background: #FFFFFF;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus {"
        "    border: 1px solid #2563EB;"
        "}"
        );
}

QString AppStyle::primaryButtonStyle()
{
    return buttonBaseStyle(
        QStringLiteral("primaryButton"),
        QStringLiteral("#2563EB"),
        QStringLiteral("#1D4ED8"),
        QStringLiteral("#FFFFFF")
        );
}

QString AppStyle::secondaryButtonStyle()
{
    return buttonBaseStyle(
        QStringLiteral("secondaryButton"),
        QStringLiteral("#F9FAFB"),
        QStringLiteral("#F3F4F6"),
        QStringLiteral("#374151"),
        QStringLiteral("1px solid #D1D5DB")
        );
}

QString AppStyle::dangerButtonStyle()
{
    return buttonBaseStyle(
        QStringLiteral("dangerButton"),
        QStringLiteral("#DC2626"),
        QStringLiteral("#B91C1C"),
        QStringLiteral("#FFFFFF")
        );
}

QString AppStyle::successButtonStyle()
{
    return QStringLiteral(
        "QPushButton#successButton {"
        "    min-height: 34px;"
        "    background: #16A34A;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 0 16px;"
        "    font-weight: 600;"
        "}"
        "QPushButton#successButton:hover {"
        "    background: #15803D;"
        "}"
        "QPushButton#successButton:disabled {"
        "    background: #9CA3AF;"
        "    color: #F9FAFB;"
        "}"
        );
}

QString AppStyle::tableStyle()
{
    return QStringLiteral(
        "QTableWidget {"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 8px;"
        "    gridline-color: #E5E7EB;"
        "    background: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "    background: #F9FAFB;"
        "    color: #374151;"
        "    border: none;"
        "    border-bottom: 1px solid #E5E7EB;"
        "    padding: 8px;"
        "    font-weight: 600;"
        "}"
        );
}

QString AppStyle::navigationButtonStyle(bool active)
{
    if (active) {
        return QStringLiteral(
            "QPushButton {"
            "    min-height: 42px;"
            "    padding: 10px 16px;"
            "    border: none;"
            "    border-radius: 8px;"
            "    background-color: #2563EB;"
            "    color: #FFFFFF;"
            "    text-align: left;"
            "    font-weight: bold;"
            "}"
            );
    }

    return QStringLiteral(
        "QPushButton {"
        "    min-height: 42px;"
        "    padding: 10px 16px;"
        "    border: none;"
        "    border-radius: 8px;"
        "    background-color: transparent;"
        "    color: #D1D5DB;"
        "    text-align: left;"
        "}"
        "QPushButton:hover {"
        "    background-color: #374151;"
        "    color: #FFFFFF;"
        "}"
        );
}

QString AppStyle::dataManagementPageStyle()
{
    return QStringLiteral(
               "QLabel#pageTitle {"
               "    color: #111827;"
               "    font-size: 24px;"
               "    font-weight: 700;"
               "}"
               "QLabel#pageSubtitle {"
               "    color: #6B7280;"
               "    font-size: 13px;"
               "}"
               "QFrame#card {"
               "    background: #FFFFFF;"
               "    border: 1px solid #E5E7EB;"
               "    border-radius: 12px;"
               "}"
               "QLabel#fieldLabel {"
               "    color: #374151;"
               "    font-size: 13px;"
               "    font-weight: 600;"
               "}"
               )
           + inputStyle()
           + primaryButtonStyle()
           + successButtonStyle()
           + secondaryButtonStyle()
           + dangerButtonStyle()
           + tableStyle()
           + QStringLiteral(
               "QLabel#messageLabel {"
               "    color: #374151;"
               "    font-size: 13px;"
               "}"
               );
}

QString AppStyle::dashboardPageStyle()
{
    return QStringLiteral(
               "QLabel#pageTitle {"
               "    color: #111827;"
               "    font-size: 24px;"
               "    font-weight: 700;"
               "}"
               "QLabel#pageSubtitle {"
               "    color: #6B7280;"
               "    font-size: 13px;"
               "}"
               "QFrame#card {"
               "    background: #FFFFFF;"
               "    border: 1px solid #E5E7EB;"
               "    border-radius: 12px;"
               "}"
               "QLabel#cardTitle {"
               "    color: #6B7280;"
               "    font-size: 13px;"
               "}"
               "QLabel#cardValue {"
               "    color: #111827;"
               "    font-size: 28px;"
               "    font-weight: 700;"
               "}"
               "QLabel#cardDescription {"
               "    color: #9CA3AF;"
               "    font-size: 12px;"
               "}"
               "QLabel#panelTitle {"
               "    color: #111827;"
               "    font-size: 16px;"
               "    font-weight: 700;"
               "}"
               "QLabel#topPostTitle {"
               "    color: #111827;"
               "    font-size: 14px;"
               "    font-weight: 600;"
               "}"
               "QLabel#topPostDetail {"
               "    color: #6B7280;"
               "    font-size: 13px;"
               "}"
               )
           + tableStyle();
}

QString AppStyle::messageLabelStyle(bool error)
{
    return error
               ? QStringLiteral("color: #DC2626; font-size: 13px;")
               : QStringLiteral("color: #374151; font-size: 13px;");
}
