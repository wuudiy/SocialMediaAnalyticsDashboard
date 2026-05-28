#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H

#include "../services/settingsservice.h"

#include <QObject>
#include <QString>

class SettingsPage;

/*
 * 设置页面控制器。
 *
 * 负责：
 * - 接收 SettingsPage 的加载设置请求；
 * - 接收保存导出目录请求；
 * - 调用 SettingsService 读写配置；
 * - 通知 View 显示导出目录、项目说明和状态。
 *
 * 不负责：
 * - QFileDialog；
 * - QMessageBox；
 * - 直接操作 UI 控件；
 * - 直接读写 QSettings。
 */
class SettingsController : public QObject
{
    Q_OBJECT

public:
    explicit SettingsController(SettingsPage *view,
                                QObject *parent = nullptr);

private slots:
    void handleLoadSettings();

    void handleSaveExportDirectory(const QString& directory);

private:
    SettingsPage *view;
    SettingsService settingsService;
};

#endif // SETTINGSCONTROLLER_H
