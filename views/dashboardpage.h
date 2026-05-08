#ifndef DASHBOARDPAGE_H
#define DASHBOARDPAGE_H

#include <QWidget>

#include "../models/user.h"

class QLabel;
class QFrame;

/*
 * DashboardPage
 * -------------
 * 登录后的首页。
 */
class DashboardPage : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardPage(QWidget *parent = nullptr);

    // MainWindow 登录成功后会把当前用户传进来
    void setCurrentUser(const User& user);

private:
    void buildUi();

    QFrame* createModuleCard(const QString& title,
                             const QString& description);

    QFrame* createEmptyPanel(const QString& title,
                             const QString& description);

private:
    QLabel *welcomeLabel;
    QLabel *subtitleLabel;
};

#endif // DASHBOARDPAGE_H
