#include "settingspage.h"
#include "ui_SettingsPage.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SettingsPage),
      exportDirectory(QString())
{
    ui->setupUi(this);
    loadSettings();
    setupProjectDescription();

    connect(ui->browseDirButton, &QPushButton::clicked, this, &SettingsPage::onBrowseDirClicked);
    connect(ui->saveExportSettingsButton, &QPushButton::clicked, this, &SettingsPage::onSaveExportSettingsClicked);
}

SettingsPage::~SettingsPage()
{
    delete ui;
}

void SettingsPage::loadSettings()
{
    QSettings settings(QStringLiteral("SocialMediaAnalytics"), QStringLiteral("Settings"));
    exportDirectory = settings.value(QStringLiteral("exportDirectory"), QString()).toString();

    if (!exportDirectory.isEmpty()) {
        ui->exportDirEdit->setText(exportDirectory);
    }
}

void SettingsPage::setupProjectDescription()
{
    QString description = QStringLiteral("Social Media Analytics Dashboard\n\n") +
                          QStringLiteral("This application is a comprehensive social media analytics platform that allows users to:\n\n") +
                          QStringLiteral("• Manage social media post data across multiple platforms (Weibo, Douyin, Bilibili, Xiaohongshu, WeChat)\n") +
                          QStringLiteral("• View real-time analytics and statistics\n") +
                          QStringLiteral("• Generate detailed reports and export data to CSV or TXT format\n") +
                          QStringLiteral("• Track engagement rates and trending content\n") +
                          QStringLiteral("• User management with role-based access control\n\n") +
                          QStringLiteral("Built with Qt 5/6 and MySQL database.\n\n") +
                          QStringLiteral("Team Members:\n") +
                          QStringLiteral("• Team Leader: 吴裕勇 (Student ID: 8002124023)\n") +
                          QStringLiteral("• Member 1: 熊倡 (Student ID: 8002124024)\n") +
                          QStringLiteral("• Member 2: 王旭坤 (Student ID: 8002124022)\n") +
                          QStringLiteral("• Member 3: 刘子懿 (Student ID: 8002124019)");

    ui->descriptionTextEdit->setPlainText(description);
}

void SettingsPage::showStatus(const QString& message, bool success)
{
    ui->statusLabel->setText(message);
    ui->statusLabel->setStyleSheet(success ? "color: green;" : "color: red;");
}

void SettingsPage::onBrowseDirClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
                this,
                tr("Select Export Directory"),
                exportDirectory.isEmpty() ? QDir::homePath() : exportDirectory,
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                );

    if (!dir.isEmpty()) {
        exportDirectory = dir;
        ui->exportDirEdit->setText(dir);
        showStatus(tr("Directory selected: ") + dir, true);
    }
}

void SettingsPage::onSaveExportSettingsClicked()
{
    if (exportDirectory.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select an export directory first."));
        return;
    }

    QSettings settings(QStringLiteral("SocialMediaAnalytics"), QStringLiteral("Settings"));
    settings.setValue(QStringLiteral("exportDirectory"), exportDirectory);

    showStatus(tr("Export settings saved successfully!"), true);
}
