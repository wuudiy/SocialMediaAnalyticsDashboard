#include "infrastructure/databasemanager.h"
#include "views/loginview.h"
#include "views/mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSqlDatabase>

/*
 * 程序入口。
 *
 * 负责：
 * 1. 创建 Qt 应用；
 * 2. 初始化 MySQL 数据库；
 * 3. 自动创建 users、posts、operation_logs 表；
 * 4. 自动创建默认管理员 admin / 123456；
 * 5. 显示登录窗口；
 * 6. 登录成功后打开主窗口；
 * 7. 程序退出前关闭数据库连接。
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

    /*
     * 必须先初始化数据库。
     *
     * 如果不调用 DatabaseManager::initialize()：
     * - 不会连接 MySQL；
     * - 不会创建 users 表；
     * - 不会创建默认管理员 admin / 123456；
     * - 登录时 UserRepository 查询不到用户；
     * - 页面就会提示 User does not exist.
     */
    if (!DatabaseManager::initialize()) {
        QMessageBox::critical(
            nullptr,
            QStringLiteral("Database Error"),
            QStringLiteral(
                "Failed to initialize database.\n\n"
                "Error detail:\n%1\n\n"
                "Please check:\n"
                "1. MySQL service is running;\n"
                "2. Database social_media_system exists;\n"
                "3. Username and password in databasemanager.cpp are correct;\n"
                "4. Qt QMYSQL driver is available."
                ).arg(DatabaseManager::lastError())
            );

        return -1;
    }

    LoginView loginView;

    QObject::connect(&loginView, &LoginView::loginSuccess,
                     [&loginView](const User &user) {
                         auto *mainWindow = new MainWindow();
                         mainWindow->setAttribute(Qt::WA_DeleteOnClose);
                         QObject::connect(mainWindow, &MainWindow::returnToLoginRequested,
                                          &loginView, [&loginView, mainWindow]() {
                                              loginView.prepareForNextLogin();
                                              mainWindow->close();
                                          });

                         mainWindow->setCurrentUser(user);
                         mainWindow->show();
                     });

    loginView.show();


    const int exitCode = app.exec();

    DatabaseManager::closeDatabase();

    return exitCode;
}
