#include "settingsservice.h"

#include <QDate>
#include <QDir>
#include <QSettings>

namespace
{
/*
 * QSettings 的组织名、应用名和配置 key。
 *
 * 注意：
 * QStringLiteral() 只能用于真正的字符串字面量，
 * 不能写 QStringLiteral(SettingsOrganization)。
 */
const QString SettingsOrganization = QStringLiteral("SocialMediaAnalytics");
const QString SettingsApplication = QStringLiteral("Settings");
const QString ExportDirectoryKey = QStringLiteral("exportDirectory");
}

SettingsService::SettingsService()
{
}

QString SettingsService::exportDirectory() const
{
    QSettings settings(SettingsOrganization, SettingsApplication);

    return settings.value(ExportDirectoryKey, QString()).toString();
}

SettingsOperationResult SettingsService::saveExportDirectory(const QString& directory) const
{
    QString message;

    if (!validateExportDirectory(directory, message)) {
        return {
            false,
            message
        };
    }

    QSettings settings(SettingsOrganization, SettingsApplication);
    settings.setValue(ExportDirectoryKey, directory.trimmed());

    return {
        true,
        QStringLiteral("Export settings saved successfully.")
    };
}

QString SettingsService::defaultExportFilePath(const QString& baseFileName,
                                               const QString& extension) const
{
    const QString cleanExtension = normalizedExtension(extension);

    const QString cleanBaseFileName = baseFileName.trimmed().isEmpty()
                                          ? QStringLiteral("social_media_report")
                                          : baseFileName.trimmed();

    const QString todayText = QDate::currentDate().toString(QStringLiteral("yyyyMMdd"));

    const QString defaultFileName = QStringLiteral("%1_%2.%3")
                                        .arg(cleanBaseFileName,
                                             todayText,
                                             cleanExtension);

    const QString directory = exportDirectory();

    if (directory.trimmed().isEmpty()) {
        return defaultFileName;
    }

    return QDir(directory).filePath(defaultFileName);
}

/*
 * 项目说明文本集中放在 SettingsService。
 *
 * 这样 SettingsPage 只负责显示，
 * 后续如果要把说明内容改成从配置文件读取，也不需要改 View。
 */
QString SettingsService::projectDescription() const
{
    return QStringLiteral("Social Media Analytics Dashboard\n\n")
    + QStringLiteral("This application is a comprehensive social media analytics platform that allows users to:\n\n")
        + QStringLiteral("• Manage social media post data across multiple platforms (Weibo, Douyin, Bilibili, Xiaohongshu, Wechat)\n")
        + QStringLiteral("• View real-time analytics and statistics\n")
        + QStringLiteral("• Generate detailed reports and export data to CSV or TXT format\n")
        + QStringLiteral("• Track engagement rates and trending content\n")
        + QStringLiteral("• User management with role-based access control\n\n")
        + QStringLiteral("Built with Qt 5/6 and MySQL database.\n\n")
        + QStringLiteral("Team Members:\n")
        + QStringLiteral("• Team Leader: 吴裕勇 (Student ID: 8002124023)\n")
        + QStringLiteral("• Member 1: 熊倡 (Student ID: 8002124024)\n")
        + QStringLiteral("• Member 2: 王旭坤 (Student ID: 8002124022)\n")
        + QStringLiteral("• Member 3: 刘子懿 (Student ID: 8002124019)");
}

bool SettingsService::validateExportDirectory(const QString& directory,
                                              QString& message) const
{
    const QString cleanDirectory = directory.trimmed();

    if (cleanDirectory.isEmpty()) {
        message = QStringLiteral("Please select an export directory first.");
        return false;
    }

    const QDir dir(cleanDirectory);

    if (!dir.exists()) {
        message = QStringLiteral("The selected export directory does not exist.");
        return false;
    }

    return true;
}

QString SettingsService::normalizedExtension(const QString& extension) const
{
    QString cleanExtension = extension.trimmed().toLower();

    if (cleanExtension.startsWith(QChar('.'))) {
        cleanExtension.remove(0, 1);
    }

    return cleanExtension.isEmpty()
               ? QStringLiteral("txt")
               : cleanExtension;
}
