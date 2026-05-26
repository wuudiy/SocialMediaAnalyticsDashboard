#ifndef ANALYTICSPAGE_H
#define ANALYTICSPAGE_H

#include "../services/analyticsservice.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class AnalyticsPage;
}
QT_END_NAMESPACE

class AnalyticsPage : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyticsPage(QWidget *parent = nullptr);
    ~AnalyticsPage();

    void refreshData();

private:
    void setupComboBox();
    void setupTables();
    void loadSummaryData();
    void loadPlatformStatistics();
    void loadDateTrends();
    void loadTopPosts();

private slots:
    void onFilterButtonClicked();
    void onRefreshButtonClicked();

private:
    Ui::AnalyticsPage *ui;
    AnalyticsService *analyticsService;
    AnalyticsFilter currentFilter;
};

#endif // ANALYTICSPAGE_H