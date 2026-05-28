#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsPage;
}
QT_END_NAMESPACE

class SettingsController;

/*
 * 系统设置页面。
 *
 * MVC 重构后，本类只负责 View 层：
 * - 初始化 UI 控件；
 * - 调用 QFileDialog 选择导出目录；
 * - 显示当前导出目录；
 * - 显示项目说明；
 * - 显示状态和警告弹窗；
 * - 发出加载 / 保存设置请求。
 *
 * 不再负责：
 * - 直接创建 QSettings；
 * - 直接读写 exportDirectory；
 * - 校验目录是否存在。
 */
class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage();

public slots:
    // MainWindow 或 Controller 可以调用，用于重新读取配置。
    void refreshSettings();

    // Controller 调用：显示当前导出目录。
    void showExportDirectory(const QString& directory);

    // Controller 调用：显示项目说明。
    void showProjectDescription(const QString& description);

    // Controller 调用：显示底部状态。
    void showStatus(const QString& message,
                    bool success);

    // Controller 调用：统一显示警告弹窗。
    void showWarningMessage(const QString& title,
                            const QString& message);

signals:
    void loadSettingsRequested();

    void saveExportDirectoryRequested(const QString& directory);

private slots:
    void onBrowseDirClicked();

    void onSaveExportSettingsClicked();

private:
    // 初始化 .ui 中控件的运行时属性。
    void prepareUiObjects();

    // 连接按钮事件。
    void connectSignals();

    // 应用统一样式。
    void applyStyleSheet();

    // 调用 QFileDialog 选择目录。
    QString selectExportDirectory(const QString& initialDirectory);

    // 读取当前输入框中的导出目录。
    QString currentExportDirectory() const;

private:
    Ui::SettingsPage *ui;

    SettingsController *settingsController;
};

#endif // SETTINGSPAGE_H
