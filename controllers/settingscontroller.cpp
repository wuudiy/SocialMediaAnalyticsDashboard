#include "settingscontroller.h"

#include "../views/settingspage.h"

SettingsController::SettingsController(SettingsPage *view,
                                       QObject *parent)
    : QObject(parent),
    view(view)
{
    /*
     * Controller 接管 SettingsPage 的配置读写请求。
     *
     * View 只负责：
     * - 用户选择了哪个目录；
     * - 用户点击了保存按钮。
     *
     * 真正的 QSettings 读写交给 SettingsService。
     */
    connect(view, &SettingsPage::loadSettingsRequested,
            this, &SettingsController::handleLoadSettings);

    connect(view, &SettingsPage::saveExportDirectoryRequested,
            this, &SettingsController::handleSaveExportDirectory);
}

void SettingsController::handleLoadSettings()
{
    view->showExportDirectory(settingsService.exportDirectory());
    view->showProjectDescription(settingsService.projectDescription());
}

void SettingsController::handleSaveExportDirectory(const QString& directory)
{
    const SettingsOperationResult result = settingsService.saveExportDirectory(directory);

    if (!result.success) {
        view->showStatus(result.message, false);
        view->showWarningMessage(
            QStringLiteral("Save Settings Failed"),
            result.message
            );

        return;
    }

    view->showStatus(result.message, true);
}
