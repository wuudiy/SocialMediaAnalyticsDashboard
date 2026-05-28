#ifndef EXPORTPAGE_H
#define EXPORTPAGE_H

#include "../models/exportmodels.h"
#include "../models/user.h"

#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class ExportPage;
}
QT_END_NAMESPACE

class ExportController;

/*
 * 报表导出页面。
 *
 * MVC 重构后，本类只负责 View 层：
 * - 初始化导出筛选控件；
 * - 收集平台、日期、导出格式；
 * - 显示报表预览；
 * - 调用 QFileDialog 选择保存位置；
 * - 显示导出状态。
 *
 * 不再负责：
 * - 生成 TXT / CSV 报表；
 * - 写本地文件；
 * - 读取 QSettings；
 * - 写操作日志。
 */
class ExportPage : public QWidget
{
    Q_OBJECT

public:
    explicit ExportPage(QWidget *parent = nullptr);
    ~ExportPage();

    void setCurrentUser(const User& user);

public slots:
    void showPreview(const QString& content);

    void showStatus(const QString& message,
                    bool success);

    void showWarningMessage(const QString& title,
                            const QString& message);

    QString selectExportFilePath(const QString& suggestedPath,
                                 const QString& extension);

signals:
    void exportReportRequested(const ExportRequest& request);

private slots:
    void onExportCsvClicked();
    void onExportTxtClicked();

private:
    void prepareUiObjects();

    void setupComboBox();

    void connectSignals();

    ExportRequest readExportRequest(ExportFormat format) const;

private:
    Ui::ExportPage *ui;

    ExportController *exportController;
};

#endif // EXPORTPAGE_H
