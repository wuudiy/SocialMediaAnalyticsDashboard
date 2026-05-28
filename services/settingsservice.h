#ifndef SETTINGSSERVICE_H
#define SETTINGSSERVICE_H

#include <QString>

/*
 * 系统设置服务。
 *
 * 当前先服务导出模块：
 * - 读取默认导出目录；
 * - 拼接默认导出文件路径。
 *
 * 后续第 6 批重构 SettingsPage 时，
 * 会继续把 SettingsPage 中的 QSettings 逻辑迁到这里。
 */
class SettingsService
{
public:
    SettingsService();

    QString exportDirectory() const;

    void saveExportDirectory(const QString& directory) const;

    QString defaultExportFilePath(const QString& baseFileName,
                                  const QString& extension) const;

private:
    QString normalizedExtension(const QString& extension) const;
};

#endif // SETTINGSSERVICE_H
