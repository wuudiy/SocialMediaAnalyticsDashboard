#ifndef EXPORTPAGE_H
#define EXPORTPAGE_H

#include "../models/user.h"
#include "../services/analyticsservice.h"
#include "../services/logservice.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class ExportPage;
}
QT_END_NAMESPACE

class ExportPage : public QWidget
{
    Q_OBJECT

public:
    explicit ExportPage(QWidget *parent = nullptr);
    ~ExportPage();

    void setCurrentUser(const User& user);

private:
    void setupComboBox();
    QString generateTxtReport();
    QString generateCsvReport();
    bool exportToFile(const QString& content, const QString& fileName, const QString& extension, QString& outFilePath);
    void showStatus(const QString& message, bool success);
    void logExportAction(const QString& format, const QString& filePath, bool success);

private slots:
    void onExportCsvClicked();
    void onExportTxtClicked();

private:
    Ui::ExportPage *ui;
    AnalyticsService *analyticsService;
    LogService *logService;
    User currentUser;
};

#endif // EXPORTPAGE_H