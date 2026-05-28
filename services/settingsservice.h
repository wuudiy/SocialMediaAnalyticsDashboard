#ifndef SETTINGSSERVICE_H
#define SETTINGSSERVICE_H

#include <QString>

/*
 * 设置保存结果。
 *
 * success：是否保存成功；
 * message：返回给 Controller / View 显示的提示信息。
 */
struct SettingsOperationResult
{
    bool success = false;
    QString message;
};

/*
 * 系统设置服务。
 *
 * 负责：
 * - 读取默认导出目录；
 * - 保存默认导出目录；
 * - 校验导出目录；
 * - 拼接默认导出文件路径；
 * - 提供项目说明文本。
 *
 * 不负责：
 * - QFileDialog；
 * - QMessageBox；
 * - 页面控件状态。
 */
class SettingsService
{
public:
    SettingsService();

    QString exportDirectory() const;

    SettingsOperationResult saveExportDirectory(const QString& directory) const;

    QString defaultExportFilePath(const QString& baseFileName,
                                  const QString& extension) const;

    QString projectDescription() const;

private:
    bool validateExportDirectory(const QString& directory,
                                 QString& message) const;

    QString normalizedExtension(const QString& extension) const;
};

#endif // SETTINGSSERVICE_H
