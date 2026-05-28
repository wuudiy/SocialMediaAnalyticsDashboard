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
 * 所以这里直接把常量定义成 QString，后面直接使用。
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

void SettingsService::saveExportDirectory(const QString& directory) const
{
    QSettings settings(SettingsOrganization, SettingsApplication);

    settings.setValue(ExportDirectoryKey, directory.trimmed());
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
