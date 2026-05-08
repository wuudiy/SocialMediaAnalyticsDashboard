#include "appstyle.h"

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
        "    background-color: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 12px;"
        "}"
        );
}

QString AppStyle::inputStyle()
{
    return QStringLiteral(
        "QLineEdit, QComboBox {"
        "    min-height: 36px;"
        "    padding: 6px 10px;"
        "    border: 1px solid #D1D5DB;"
        "    border-radius: 6px;"
        "    background-color: #FFFFFF;"
        "}"
        "QLineEdit:focus, QComboBox:focus {"
        "    border: 1px solid #2563EB;"
        "}"
        );
}

QString AppStyle::primaryButtonStyle()
{
    return QStringLiteral(
        "QPushButton {"
        "    min-height: 38px;"
        "    padding: 8px 16px;"
        "    border: none;"
        "    border-radius: 6px;"
        "    background-color: #2563EB;"
        "    color: #FFFFFF;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1D4ED8;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1E40AF;"
        "}"
        );
}

QString AppStyle::secondaryButtonStyle()
{
    return QStringLiteral(
        "QPushButton {"
        "    min-height: 36px;"
        "    padding: 8px 16px;"
        "    border: 1px solid #D1D5DB;"
        "    border-radius: 6px;"
        "    background-color: #FFFFFF;"
        "    color: #374151;"
        "}"
        "QPushButton:hover {"
        "    background-color: #F3F4F6;"
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
