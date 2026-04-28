#include "mainwindow.h"
#include "ui_mainwindow.h"

/*
 * 主窗口构造函数
 * ----------------
 * 这里只初始化 mainwindow.ui。
 * 当前用户信息通过 setCurrentUser() 设置。
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * 设置当前登录用户
 * ----------------
 * 登录成功后，main.cpp 会调用这个函数。
 * 后续你可以在这里根据 currentUser.role 控制按钮显示。
 */
void MainWindow::setCurrentUser(const User& user)
{
    currentUser = user;

    // 设置主窗口标题，显示当前登录用户
    setWindowTitle("Social Media Analytics Dashboard - " + currentUser.username);

    if (currentUser.role == "admin") {
        /*
         * 管理员登录：
         * 以后可以在这里显示管理员功能按钮，
         * 例如：用户注册、账号管理、权限管理。
         */
    } else {
        /*
         * 普通用户登录：
         * 以后可以在这里隐藏管理员功能按钮。
         */
    }
}

