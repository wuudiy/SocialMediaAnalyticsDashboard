#include "exportpage.h"
#include "ui_ExportPage.h"

#include "../services/analyticsservice.h"
#include "../services/logservice.h"

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>

ExportPage::ExportPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::ExportPage),
      analyticsService(new AnalyticsService()),
      logService(new LogService())
{
    ui->setupUi(this);
    setupComboBox();

    connect(ui->exportCsvButton, &QPushButton::clicked, this, &ExportPage::onExportCsvClicked);
    connect(ui->exportTxtButton, &QPushButton::clicked, this, &ExportPage::onExportTxtClicked);
}

ExportPage::~ExportPage()
{
    delete logService;
    delete analyticsService;
    delete ui;
}

void ExportPage::setupComboBox()
{
    ui->platformComboBox->addItem(tr("All Platforms"), QString());
    ui->platformComboBox->addItem(tr("Weibo"), QStringLiteral("Weibo"));
    ui->platformComboBox->addItem(tr("Douyin"), QStringLiteral("Douyin"));
    ui->platformComboBox->addItem(tr("Bilibili"), QStringLiteral("Bilibili"));
    ui->platformComboBox->addItem(tr("Xiaohongshu"), QStringLiteral("Xiaohongshu"));
    ui->platformComboBox->addItem(tr("WeChat"), QStringLiteral("WeChat"));

    ui->startDateEdit->setDate(QDate::currentDate().addDays(-30));
    ui->endDateEdit->setDate(QDate::currentDate());
}

QString ExportPage::generateTxtReport()
{
    QString platform = ui->platformComboBox->currentData().toString();
    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();

    QString report;
    QTextStream stream(&report);

    stream << "========================================\n";
    stream << "    Social Media Analytics Report\n";
    stream << "========================================\n";
    stream << "\n";

    stream << "Report Generated: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    stream << "\n";

    stream << "Data Range:\n";
    stream << "  Start Date: " << startDate.toString("yyyy-MM-dd") << "\n";
    stream << "  End Date:   " << endDate.toString("yyyy-MM-dd") << "\n";
    stream << "  Platform:   " << (platform.isEmpty() ? "All Platforms" : platform) << "\n";
    stream << "\n";

    stream << "========================================\n";
    stream << "Summary Statistics\n";
    stream << "========================================\n";

    DashboardSummary summary = analyticsService->loadDashboardSummary();
    stream << "  Total Posts:        " << summary.totalPosts << "\n";
    stream << "  Total Interactions: " << summary.totalInteractions << "\n";
    stream << "  Total Views:        " << summary.totalViews << "\n";
    stream << "  Avg Engagement Rate: " << QString("%1%").arg(summary.averageEngagementRate * 100, 0, 'f', 2) << "\n";
    stream << "\n";

    stream << "========================================\n";
    stream << "Platform Statistics\n";
    stream << "========================================\n";

    QList<PlatformStatistics> platformStats = analyticsService->getPlatformStatistics(platform);
    for (const PlatformStatistics& stats : platformStats) {
        stream << "\n";
        stream << "Platform: " << stats.platform << "\n";
        stream << "  Post Count:        " << stats.postCount << "\n";
        stream << "  Total Likes:       " << stats.totalLikes << "\n";
        stream << "  Total Comments:    " << stats.totalComments << "\n";
        stream << "  Total Shares:      " << stats.totalShares << "\n";
        stream << "  Total Views:       " << stats.totalViews << "\n";
        stream << "  Engagement Rate:   " << QString("%1%").arg(stats.averageEngagementRate * 100, 0, 'f', 2) << "\n";
    }
    stream << "\n";

    stream << "========================================\n";
    stream << "Top Posts by Interactions\n";
    stream << "========================================\n";

    QList<Post> topPosts = analyticsService->getTopPosts(5);
    int rank = 1;
    for (const Post& post : topPosts) {
        stream << "\n";
        stream << rank << ". " << post.platform << " - " << post.accountName << "\n";
        stream << "   Content:      " << post.content.left(60) << (post.content.size() > 60 ? "..." : "") << "\n";
        stream << "   Interactions: " << post.interactionCount() << "\n";
        stream << "   Views:        " << post.views << "\n";
        stream << "   Engagement:   " << QString("%1%").arg(post.engagementRate() * 100, 0, 'f', 2) << "\n";
        rank++;
    }
    stream << "\n";

    stream << "========================================\n";
    stream << "End of Report\n";
    stream << "========================================\n";

    return report;
}

QString ExportPage::generateCsvReport()
{
    QString platform = ui->platformComboBox->currentData().toString();
    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();

    QString csv;
    QTextStream stream(&csv);

    stream << "Social Media Analytics Report\n";
    stream << "\n";

    stream << "Report Generated," << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    stream << "\n";

    stream << "Data Range\n";
    stream << "Start Date," << startDate.toString("yyyy-MM-dd") << "\n";
    stream << "End Date," << endDate.toString("yyyy-MM-dd") << "\n";
    stream << "Platform," << (platform.isEmpty() ? "All Platforms" : platform) << "\n";
    stream << "\n";

    stream << "Summary Statistics\n";
    stream << "Metric,Value\n";

    DashboardSummary summary = analyticsService->loadDashboardSummary();
    stream << "Total Posts," << summary.totalPosts << "\n";
    stream << "Total Interactions," << summary.totalInteractions << "\n";
    stream << "Total Views," << summary.totalViews << "\n";
    stream << "Avg Engagement Rate," << QString("%1").arg(summary.averageEngagementRate * 100, 0, 'f', 2) << "%\n";
    stream << "\n";

    stream << "Platform Statistics\n";
    stream << "Platform,Post Count,Total Likes,Total Comments,Total Shares,Total Views,Engagement Rate\n";

    QList<PlatformStatistics> platformStats = analyticsService->getPlatformStatistics(platform);
    for (const PlatformStatistics& stats : platformStats) {
        stream << stats.platform << ","
               << stats.postCount << ","
               << stats.totalLikes << ","
               << stats.totalComments << ","
               << stats.totalShares << ","
               << stats.totalViews << ","
               << QString("%1").arg(stats.averageEngagementRate * 100, 0, 'f', 2) << "%\n";
    }
    stream << "\n";

    stream << "Top Posts by Interactions\n";
    stream << "Rank,Platform,Account,Content,Interactions,Views,Engagement Rate\n";

    QList<Post> topPosts = analyticsService->getTopPosts(5);
    int rank = 1;
    for (const Post& post : topPosts) {
        QString content = post.content;
        content.replace("\"", "\"\"");
        if (content.contains(",")) {
            content = "\"" + content + "\"";
        }

        stream << rank << ","
               << post.platform << ","
               << post.accountName << ","
               << content.left(100) << ","
               << post.interactionCount() << ","
               << post.views << ","
               << QString("%1").arg(post.engagementRate() * 100, 0, 'f', 2) << "%\n";
        rank++;
    }

    return csv;
}

bool ExportPage::exportToFile(const QString& content, const QString& fileName, const QString& extension, QString& outFilePath)
{
    QString defaultFileName = QString("%1_%2.%3")
            .arg(fileName)
            .arg(QDate::currentDate().toString("yyyyMMdd"))
            .arg(extension);

    QSettings settings(QStringLiteral("SocialMediaAnalytics"), QStringLiteral("Settings"));
    QString defaultDir = settings.value(QStringLiteral("exportDirectory"), QString()).toString();

    QString initialPath = defaultDir.isEmpty() ? defaultFileName : QDir(defaultDir).filePath(defaultFileName);

    QString filePath = QFileDialog::getSaveFileName(
                this,
                tr("Export Report"),
                initialPath,
                tr("%1 Files (*.%2)").arg(extension.toUpper()).arg(extension)
                );

    if (filePath.isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Export Failed"), tr("Failed to open file for writing"));
        return false;
    }

    QTextStream out(&file);
    out << content;
    file.close();

    outFilePath = filePath;
    return true;
}

void ExportPage::showStatus(const QString& message, bool success)
{
    ui->statusLabel->setText(message);
    ui->statusLabel->setStyleSheet(success ? "color: green;" : "color: red;");
}

void ExportPage::onExportCsvClicked()
{
    QString csvContent = generateCsvReport();
    QString filePath;

    ui->previewTextEdit->setText(csvContent);

    if (exportToFile(csvContent, "social_media_report", "csv", filePath)) {
        showStatus(tr("CSV report exported successfully!"), true);
        logExportAction("CSV", filePath, true);
    } else {
        showStatus(tr("Export cancelled or failed."), false);
        logExportAction("CSV", QString(), false);
    }
}

void ExportPage::onExportTxtClicked()
{
    QString txtContent = generateTxtReport();
    QString filePath;

    ui->previewTextEdit->setText(txtContent);

    if (exportToFile(txtContent, "social_media_report", "txt", filePath)) {
        showStatus(tr("TXT report exported successfully!"), true);
        logExportAction("TXT", filePath, true);
    } else {
        showStatus(tr("Export cancelled or failed."), false);
        logExportAction("TXT", QString(), false);
    }
}

void ExportPage::setCurrentUser(const User& user)
{
    currentUser = user;
}

void ExportPage::logExportAction(const QString& format, const QString& filePath, bool success)
{
    QString platform = ui->platformComboBox->currentData().toString();
    QDate startDate = ui->startDateEdit->date();
    QDate endDate = ui->endDateEdit->date();

    QString detail = QString("Format: %1, Platform: %2, Date Range: %3 to %4, File: %5")
            .arg(format)
            .arg(platform.isEmpty() ? "All" : platform)
            .arg(startDate.toString("yyyy-MM-dd"))
            .arg(endDate.toString("yyyy-MM-dd"))
            .arg(filePath);

    if (currentUser.isValid()) {
        logService->writeLog(currentUser.userId, currentUser.username, "Export Report", detail, success ? "success" : "failed");
    } else {
        logService->writeSystemLog("Export Report", detail, success ? "success" : "failed");
    }
}
