#include "postcontroller.h"

#include "../views/postmanagementpage.h"

#include <QFileInfo>

PostController::PostController(PostManagementPage *view,
                               QObject *parent)
    : QObject(parent),
    view(view)
{
    /*
     * Controller 在这里接管 View 发出的业务请求。
     *
     * View 只负责告诉 Controller：
     * “用户点了新增 / 修改 / 删除 / 导入 / 刷新”
     *
     * 真正的业务处理、日志记录、数据库调用都不再写在 View 里。
     */
    connect(view, &PostManagementPage::refreshPostsRequested,
            this, &PostController::handleRefreshPosts);

    connect(view, &PostManagementPage::addPostRequested,
            this, &PostController::handleAddPost);

    connect(view, &PostManagementPage::updatePostRequested,
            this, &PostController::handleUpdatePost);

    connect(view, &PostManagementPage::deletePostRequested,
            this, &PostController::handleDeletePost);

    connect(view, &PostManagementPage::loadPostRequested,
            this, &PostController::handleLoadPost);

    connect(view, &PostManagementPage::importCsvRequested,
            this, &PostController::handleImportCsv);
}

void PostController::setCurrentUser(const User& user)
{
    currentUser = user;
}

void PostController::handleRefreshPosts(const QString& platform,
                                        const QString& keyword)
{
    /*
     * 数据隔离关键点：
     * 查询帖子时必须把 currentUser 传给 PostService。
     *
     * PostService 会判断：
     * - admin 查询全部；
     * - 普通 user 只查询自己的帖子。
     */
    const QList<Post> posts = postService.findPosts(currentUser, platform, keyword);
    view->showPosts(posts);
}

void PostController::handleAddPost(const Post& post)
{
    /*
     * 新增帖子时把 currentUser 传给 PostService。
     * PostService 会自动补 createdByUserId / createdByUsername。
     */
    const PostOperationResult result = postService.addPost(currentUser, post);

    if (!result.success) {
        view->showMessage(result.message, true);
        view->showWarningMessage(QStringLiteral("Invalid Input"), result.message);

        writePostOperationLog(
            QStringLiteral("add_post"),
            QStringLiteral("Add post failed: %1").arg(result.message),
            QStringLiteral("failed")
            );

        return;
    }

    writePostOperationLog(
        QStringLiteral("add_post"),
        QStringLiteral("Add post successful. Platform: %1, Account: %2, Date: %3, Likes: %4, Comments: %5, Shares: %6, Views: %7")
            .arg(post.platform.trimmed(),
                 post.accountName.trimmed(),
                 post.publishDate.toString(QStringLiteral("yyyy-MM-dd")))
            .arg(post.likes)
            .arg(post.comments)
            .arg(post.shares)
            .arg(post.views),
        QStringLiteral("success")
        );

    view->handleAddSuccess(result.message);
}

void PostController::handleUpdatePost(const Post& post)
{
    /*
     * 修改帖子时把 currentUser 传给 PostService。
     * 普通用户只能修改自己的帖子，管理员可以修改全部。
     */
    const PostOperationResult result = postService.updatePost(currentUser, post);

    if (!result.success) {
        view->showMessage(result.message, true);
        view->showWarningMessage(QStringLiteral("Update Failed"), result.message);

        writePostOperationLog(
            QStringLiteral("update_post"),
            QStringLiteral("Update post failed: %1 Post ID: %2")
                .arg(result.message)
                .arg(post.postId),
            QStringLiteral("failed")
            );

        return;
    }

    writePostOperationLog(
        QStringLiteral("update_post"),
        QStringLiteral("Update post successful. Post ID: %1, Platform: %2, Account: %3, Date: %4, Likes: %5, Comments: %6, Shares: %7, Views: %8")
            .arg(post.postId)
            .arg(post.platform.trimmed(),
                 post.accountName.trimmed(),
                 post.publishDate.toString(QStringLiteral("yyyy-MM-dd")))
            .arg(post.likes)
            .arg(post.comments)
            .arg(post.shares)
            .arg(post.views),
        QStringLiteral("success")
        );

    view->handleUpdateSuccess(result.message);
}

void PostController::handleDeletePost(int postId)
{
    /*
     * 删除帖子时把 currentUser 传给 PostService。
     * 普通用户只能删除自己的帖子，管理员可以删除全部。
     */
    const PostOperationResult result = postService.deletePost(currentUser, postId);

    if (!result.success) {
        view->showMessage(result.message, true);
        view->showWarningMessage(QStringLiteral("Delete Failed"), result.message);

        writePostOperationLog(
            QStringLiteral("delete_post"),
            QStringLiteral("Delete post failed. Post ID: %1. Reason: %2")
                .arg(postId)
                .arg(result.message),
            QStringLiteral("failed")
            );

        return;
    }

    writePostOperationLog(
        QStringLiteral("delete_post"),
        QStringLiteral("Delete post successful. Post ID: %1").arg(postId),
        QStringLiteral("success")
        );

    view->handleDeleteSuccess(postId, result.message);
}

void PostController::handleLoadPost(int postId)
{
    const Post post = postService.findPostById(postId);

    if (!post.isValid()) {
        view->showWarningMessage(
            QStringLiteral("Load Failed"),
            QStringLiteral("The selected post record does not exist.")
            );

        view->refreshPosts();
        return;
    }

    view->showPostForEditing(post);
}

void PostController::handleImportCsv(const QString& fileName)
{
    /*
     * CSV 导入时也必须传 currentUser。
     * 导入的每条帖子都会归属于当前登录用户。
     */
    const CsvImportResult result = csvImportService.importFromFile(currentUser, fileName);
    const QString displayMessage = result.toDisplayMessage();

    if (!result.fileOpened) {
        writePostOperationLog(
            QStringLiteral("import_csv"),
            QStringLiteral("CSV import failed. File: %1, Reason: %2")
                .arg(QFileInfo(fileName).fileName(),
                     displayMessage),
            QStringLiteral("failed")
            );

        view->showWarningMessage(QStringLiteral("Import Failed"), displayMessage);
        view->showMessage(displayMessage, true);
        return;
    }

    const bool hasFailedRows = result.hasFailedRows();

    writePostOperationLog(
        QStringLiteral("import_csv"),
        QStringLiteral("CSV import finished. File: %1, Success: %2, Failed: %3%4")
            .arg(QFileInfo(fileName).fileName())
            .arg(result.successCount)
            .arg(result.failedCount)
            .arg(result.errorDetails.isEmpty()
                     ? QString()
                     : QStringLiteral(", Errors: %1").arg(result.errorDetails.join(QStringLiteral(" | ")))),
        hasFailedRows ? QStringLiteral("failed") : QStringLiteral("success")
        );

    view->handleImportFinished(displayMessage, hasFailedRows);
}

int PostController::currentOperatorId() const
{
    return currentUser.isValid() ? currentUser.userId : -1;
}

QString PostController::currentOperatorName() const
{
    if (currentUser.isValid() && !currentUser.username.trimmed().isEmpty()) {
        return currentUser.username.trimmed();
    }

    return QStringLiteral("unknown");
}

void PostController::writePostOperationLog(const QString& action,
                                           const QString& detail,
                                           const QString& result)
{
    logService.writeLog(
        currentOperatorId(),
        currentOperatorName(),
        action,
        detail,
        result
        );
}
