#include "settingspage.h"
#include "ui_SettingsPage.h"

#include "../controllers/settingscontroller.h"
#include "../styles/appstyle.h"

#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::SettingsPage),
    settingsController(nullptr)
{
    /*
     * setupUi() 读取 forms/SettingsPage.ui，
     * 自动创建导出设置区域、项目说明区域和状态 Label。
     */
    ui->setupUi(this);

    prepareUiObjects();
    connectSignals();
    applyStyleSheet();

    /*
     * 当前批次为了减少 MainWindow 改动，
     * Controller 先由页面内部创建。
     *
     * 后续如果继续做 MainWindowController，
     * 可以统一改成外部创建 Controller 并注入。
     */
    settingsController = new SettingsController(this, this);

    refreshSettings();
}

SettingsPage::~SettingsPage()
{
    delete ui;
}

/*
 * 初始化 .ui 中控件的运行时属性。
 *
 * .ui 保留布局；
 * cpp 只设置样式名、只读状态、按钮光标等运行时属性。
 */
void SettingsPage::prepareUiObjects()
{
    setObjectName(QStringLiteral("settingsPage"));

    ui->pageTitle->setObjectName(QStringLiteral("pageTitle"));

    ui->exportTitle->setObjectName(QStringLiteral("fieldLabel"));
    ui->exportDirLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->aboutTitle->setObjectName(QStringLiteral("fieldLabel"));
    ui->versionLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->buildDateLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->descriptionTitle->setObjectName(QStringLiteral("fieldLabel"));

    ui->statusLabel->setObjectName(QStringLiteral("messageLabel"));
    ui->statusLabel->setWordWrap(true);

    ui->exportDirEdit->setReadOnly(true);
    ui->exportDirEdit->setPlaceholderText(QStringLiteral("No export directory selected."));

    ui->descriptionTextEdit->setReadOnly(true);

    ui->browseDirButton->setObjectName(QStringLiteral("secondaryButton"));
    ui->saveExportSettingsButton->setObjectName(QStringLiteral("primaryButton"));

    ui->browseDirButton->setCursor(Qt::PointingHandCursor);
    ui->saveExportSettingsButton->setCursor(Qt::PointingHandCursor);
}

/*
 * 集中连接信号槽。
 */
void SettingsPage::connectSignals()
{
    connect(ui->browseDirButton, &QPushButton::clicked,
            this, &SettingsPage::onBrowseDirClicked);

    connect(ui->saveExportSettingsButton, &QPushButton::clicked,
            this, &SettingsPage::onSaveExportSettingsClicked);
}

void SettingsPage::applyStyleSheet()
{
    setStyleSheet(AppStyle::dataManagementPageStyle());
}

/*
 * 刷新设置。
 *
 * View 不再直接读取 QSettings，
 * 只发出加载请求，交给 Controller / Service 处理。
 */
void SettingsPage::refreshSettings()
{
    emit loadSettingsRequested();
}

void SettingsPage::showExportDirectory(const QString& directory)
{
    ui->exportDirEdit->setText(directory.trimmed());
}

void SettingsPage::showProjectDescription(const QString& description)
{
    ui->descriptionTextEdit->setPlainText(description);
}

void SettingsPage::onBrowseDirClicked()
{
    /*
     * QFileDialog 属于界面交互，保留在 View 层是合理的。
     * 选择到目录后只更新输入框，不直接保存配置。
     */
    const QString directory = selectExportDirectory(currentExportDirectory());

    if (directory.trimmed().isEmpty()) {
        return;
    }

    showExportDirectory(directory);

    showStatus(
        QStringLiteral("Directory selected: %1").arg(directory),
        true
        );
}

void SettingsPage::onSaveExportSettingsClicked()
{
    /*
     * View 只把当前目录发给 Controller。
     * 目录校验和 QSettings 保存不在 View 中做。
     */
    emit saveExportDirectoryRequested(currentExportDirectory());
}

QString SettingsPage::selectExportDirectory(const QString& initialDirectory)
{
    return QFileDialog::getExistingDirectory(
        this,
        QStringLiteral("Select Export Directory"),
        initialDirectory.trimmed().isEmpty()
            ? QDir::homePath()
            : initialDirectory.trimmed(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
}

QString SettingsPage::currentExportDirectory() const
{
    return ui->exportDirEdit->text().trimmed();
}

void SettingsPage::showStatus(const QString& message,
                              bool success)
{
    ui->statusLabel->setText(message);

    /*
     * AppStyle::messageLabelStyle(true) 表示错误样式，
     * 所以这里传入 !success。
     */
    ui->statusLabel->setStyleSheet(AppStyle::messageLabelStyle(!success));
}

void SettingsPage::showWarningMessage(const QString& title,
                                      const QString& message)
{
    QMessageBox::warning(this, title, message);
}
