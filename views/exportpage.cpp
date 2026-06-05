#include "exportpage.h"
#include "ui_ExportPage.h"

#include "../controllers/exportcontroller.h"
#include "../styles/appstyle.h"
#include "../utils/platformconstants.h"

#include <QDate>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>

ExportPage::ExportPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::ExportPage),
    exportController(nullptr)
{
    ui->setupUi(this);

    prepareUiObjects();
    setupComboBox();
    connectSignals();

    /*
     * 当前批次为了减少 MainWindow 改动，Controller 先由页面内部创建。
     * 后续重构 MainWindowController 时，可以再改成统一注入。
     */
    exportController = new ExportController(this, this);
}

ExportPage::~ExportPage()
{
    delete ui;
}

void ExportPage::setCurrentUser(const User& user)
{
    if (exportController) {
        exportController->setCurrentUser(user);
    }
}

/*
 * 初始化 .ui 中控件的运行时属性。
 *
 * .ui 负责布局；
 * cpp 负责运行时状态、样式名和默认值。
 */
void ExportPage::prepareUiObjects()
{
    setObjectName(QStringLiteral("exportPage"));

    ui->exportCsvButton->setObjectName(QStringLiteral("primaryButton"));
    ui->exportTxtButton->setObjectName(QStringLiteral("secondaryButton"));
    ui->exportHtmlButton->setObjectName(QStringLiteral("secondaryButton"));

    ui->exportCsvButton->setCursor(Qt::PointingHandCursor);
    ui->exportTxtButton->setCursor(Qt::PointingHandCursor);
    ui->exportHtmlButton->setCursor(Qt::PointingHandCursor);

    ui->previewTextEdit->setReadOnly(true);
    ui->previewTextEdit->setPlaceholderText(
        QStringLiteral("The generated report preview will appear here.")
        );

    ui->statusLabel->setText(QString());
    ui->statusLabel->setWordWrap(true);

    /*
     * 复用现有数据管理页面样式。
     * ExportPage 的 .ui 里本身也有部分内联样式，本批次暂不大改 UI。
     */
    setStyleSheet(AppStyle::dataManagementPageStyle());
}

void ExportPage::setupComboBox()
{
    ui->platformComboBox->clear();

    /*
     * 下拉框显示值和实际查询值分开：
     * - 显示 “All Platforms”
     * - 实际 data 为空字符串，表示不按平台过滤。
     */
    ui->platformComboBox->addItem(QStringLiteral("All Platforms"), QString());

    for (const QString& platform : PlatformConstants::availablePlatforms()) {
        ui->platformComboBox->addItem(platform, platform);
    }

    ui->startDateEdit->setDate(QDate::currentDate().addDays(-30));
    ui->endDateEdit->setDate(QDate::currentDate());

    ui->startDateEdit->setCalendarPopup(true);
    ui->endDateEdit->setCalendarPopup(true);
}

void ExportPage::connectSignals()
{
    connect(ui->exportCsvButton, &QPushButton::clicked,
            this, &ExportPage::onExportCsvClicked);

    connect(ui->exportTxtButton, &QPushButton::clicked,
            this, &ExportPage::onExportTxtClicked);

    connect(ui->exportHtmlButton, &QPushButton::clicked,
            this, &ExportPage::onExportHtmlClicked);

}

void ExportPage::onExportCsvClicked()
{
    emit exportReportRequested(readExportRequest(ExportFormat::Csv));
}

void ExportPage::onExportTxtClicked()
{
    emit exportReportRequested(readExportRequest(ExportFormat::Txt));
}

void ExportPage::onExportHtmlClicked()
{
    emit exportReportRequested(readExportRequest(ExportFormat::Html));
}

ExportRequest ExportPage::readExportRequest(ExportFormat format) const
{
    ExportRequest request;

    request.format = format;
    request.baseFileName = QStringLiteral("social_media_report");
    request.filter.platform = ui->platformComboBox->currentData().toString();
    request.filter.startDate = ui->startDateEdit->date();
    request.filter.endDate = ui->endDateEdit->date();

    return request;
}

void ExportPage::showPreview(const QString& content,
                             ExportPreviewType previewType)
{
    /*
     * 预览类型由 Service 返回，View 不再用字符串内容猜测格式。
     *
     * - TXT / CSV：纯文本预览；
     * - HTML：网页预览；
     */
    if (previewType == ExportPreviewType::Html) {
        ui->previewTextEdit->setHtml(content);
        return;
    }

    ui->previewTextEdit->setPlainText(content);
}

void ExportPage::showStatus(const QString& message,
                            bool success)
{
    ui->statusLabel->setText(message);

    /*
     * AppStyle::messageLabelStyle(true) 表示错误样式，
     * 所以这里传入 !success。
     */
    ui->statusLabel->setStyleSheet(AppStyle::messageLabelStyle(!success));
}

void ExportPage::showWarningMessage(const QString& title,
                                    const QString& message)
{
    QMessageBox::warning(this, title, message);
}

QString ExportPage::selectExportFilePath(const QString& suggestedPath,
                                         const QString& extension)
{
    return QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Export Report"),
        suggestedPath,
        QStringLiteral("%1 Files (*.%2)")
            .arg(extension.toUpper())
            .arg(extension)
        );
}