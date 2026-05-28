#ifndef POSTMANAGEMENTPAGE_H
#define POSTMANAGEMENTPAGE_H

#include "../models/post.h"
#include "../models/user.h"

#include <QList>
#include <QString>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class PostManagementPage;
}
QT_END_NAMESPACE

class PostController;

/*
 * 帖子数据管理页面。
 *
 * MVC 重构后，本类只负责 View 层：
 * - 初始化 ui 控件；
 * - 读取表单输入；
 * - 显示帖子表格；
 * - 显示提示信息；
 * - 发出用户操作信号。
 *
 * 不再负责：
 * - 直接调用 PostRepository；
 * - 直接写操作日志；
 * - 直接解析 CSV；
 * - 直接处理新增、修改、删除业务。
 */
class PostManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit PostManagementPage(QWidget *parent = nullptr);
    ~PostManagementPage();

    // 主窗口在登录成功后调用，用于记录当前操作人。
    void setCurrentUser(const User& user);

public slots:
    // 主窗口切换到本页面时调用，保证表格显示最新数据。
    void refreshPosts();

    // Controller 调用：显示帖子列表。
    void showPosts(const QList<Post>& posts);

    // Controller 调用：把一条帖子加载到表单，进入编辑状态。
    void showPostForEditing(const Post& post);

    // Controller 调用：显示底部提示。
    void showMessage(const QString& message,
                     bool error = false);

    // Controller 调用：统一显示警告弹窗。
    void showWarningMessage(const QString& title,
                            const QString& message);

    // Controller 调用：新增成功后的页面处理。
    void handleAddSuccess(const QString& message);

    // Controller 调用：修改成功后的页面处理。
    void handleUpdateSuccess(const QString& message);

    // Controller 调用：删除成功后的页面处理。
    void handleDeleteSuccess(int deletedPostId,
                             const QString& message);

    // Controller 调用：CSV 导入完成后的页面处理。
    void handleImportFinished(const QString& message,
                              bool hasFailedRows);

signals:
    void refreshPostsRequested(const QString& platform,
                               const QString& keyword);

    void addPostRequested(const Post& post);

    void updatePostRequested(const Post& post);

    void deletePostRequested(int postId);

    void loadPostRequested(int postId);

    void importCsvRequested(const QString& fileName);

private slots:
    void onAddPostClicked();
    void onUpdatePostClicked();
    void onDeletePostClicked();
    void onSearchClicked();
    void onResetSearchClicked();
    void onImportCsvClicked();
    void onTableCellDoubleClicked(int row, int column);

private:
    // 初始化 .ui 中已有控件的运行时属性，例如下拉框数据、按钮样式名、日期默认值。
    void prepareUiObjects();

    // 连接 .ui 中控件的信号槽。
    void connectSignals();

    // 应用统一样式，具体 QSS 不写在本类中。
    void applyStyleSheet();

    // 初始化帖子表格的列名、选择模式和列宽策略。
    void setupTable();

    // 把 Controller 返回的数据填入表格。
    void fillTable(const QList<Post>& posts);

    // 从表单控件读取一条帖子数据。
    Post readPostFromForm() const;

    // 清空表单，并退出编辑状态。
    void resetForm();

    // 将一条数据库帖子加载到表单，用于修改。
    void loadPostToForm(const Post& post);

    // 退出编辑状态。
    void clearEditingState();

    // 获取当前表格选中的帖子 ID。
    int selectedPostId() const;

    QString currentSearchPlatform() const;
    QString currentSearchKeyword() const;

private:
    Ui::PostManagementPage *ui;

    User currentUser;
    PostController *postController;

    // -1 表示当前不是编辑状态；大于 0 表示正在编辑某条帖子。
    int editingPostId;
};

#endif // POSTMANAGEMENTPAGE_H
