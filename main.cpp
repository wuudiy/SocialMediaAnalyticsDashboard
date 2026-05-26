#include "services/databasemanager.h"
#include "views/loginview.h"
#include "views/mainwindow.h"

#include <QApplication>
#include <QMessageBox>

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QDebug>


/*
 * 程序入口。
 *
 * 负责：
 * - 创建 Qt 应用；
 * - 初始化数据库；
 * - 显示登录窗口；
 * - 登录成功后打开主窗口；
 * - 程序退出前关闭数据库。
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 打印所有可用的数据库驱动
    qDebug() << "Available drivers:";
    QStringList drivers = QSqlDatabase::drivers();
    foreach (QString driver, drivers) {
        qDebug() << "\t" << driver;
    }

    if (!DatabaseManager::initialize()) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("Database Error"),
                              QStringLiteral("Database initialization failed:\n%1")
                                  .arg(DatabaseManager::lastError()));

        return -1;
    }

    LoginView loginView;

    QObject::connect(&loginView, &LoginView::loginSuccess, [](const User &user) {
        auto *mainWindow = new MainWindow();
        mainWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainWindow->setCurrentUser(user);
        mainWindow->show();
    });

    loginView.show();

    const int exitCode = app.exec();

    DatabaseManager::closeDatabase();

    return exitCode;
}



