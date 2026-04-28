#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../models/User.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/*
 * View 层：MainWindow
 * -------------------
 * 作用：
 * 1. 登录成功后的系统主窗口；
 * 2. 接收当前登录用户；
 * 3. 根据用户角色决定显示管理员功能还是普通用户功能。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 登录成功后，由 main.cpp 调用，把当前用户传入主窗口
    void setCurrentUser(const User& user);

private:
    Ui::MainWindow *ui;
    User currentUser;   // 当前登录用户
};

#endif
 // MAINWINDOW_H
