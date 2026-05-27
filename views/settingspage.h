#ifndef SETTINGSPAGE_H
#define SETTINGSPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsPage;
}
QT_END_NAMESPACE

class SettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);
    ~SettingsPage();

private:
    void loadSettings();
    void setupProjectDescription();
    void showStatus(const QString& message, bool success);

private slots:
    void onBrowseDirClicked();
    void onSaveExportSettingsClicked();

private:
    Ui::SettingsPage *ui;
    QString exportDirectory;
};

#endif // SETTINGSPAGE_H