#include "views/LoginView.h"
#include "views/mainwindow.h"
#include "services/DatabaseManager.h"

#include <QApplication>
#include <QMessageBox>
#include <QObject>

/*
 * 程序入口
 * ----------------
 * 第一阶段启动流程：
 * 1. 连接 MySQL；
 * 2. 创建 users 表；
 * 3. 插入初始管理员账号；
 * 4. 显示登录窗口；
 * 5. 登录成功后显示 MainWindow。
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 连接 MySQL 数据库
    if (!DatabaseManager::connectDatabase()) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            "Failed to connect MySQL database."
            );
        return -1;
    }

    // 创建 users 表
    if (!DatabaseManager::createTables()) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            "Failed to create database tables."
            );
        return -1;
    }

    // 插入初始管理员：admin / 123456
    if (!DatabaseManager::insertInitialAdmin()) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            "Failed to create initial admin account."
            );
        return -1;
    }

    LoginView loginView;
    MainWindow mainWindow;

    /*
     * 登录成功后：
     * 1. LoginView 发出 loginSuccess(User) 信号；
     * 2. main.cpp 接收该信号；
     * 3. 把 user 传给 MainWindow；
     * 4. 显示 MainWindow。
     */
    QObject::connect(
        &loginView,
        &LoginView::loginSuccess,
        [&mainWindow](const User& user) {
            mainWindow.setCurrentUser(user);
            mainWindow.show();
        }
        );

    loginView.show();

    return app.exec();
}
