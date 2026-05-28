#ifndef POSTCONTROLLER_H
#define POSTCONTROLLER_H

#include "../models/post.h"
#include "../models/user.h"
#include "../services/csvimportservice.h"
#include "../services/logservice.h"
#include "../services/postservice.h"

#include <QObject>
#include <QString>

class PostManagementPage;

/*
 * 帖子管理控制器。
 *
 * 负责连接 View 和 Service：
 * - 接收 PostManagementPage 发出的用户操作信号；
 * - 调用 PostService / CsvImportService 完成业务；
 * - 写操作日志；
 * - 通知 View 刷新表格、显示提示、加载编辑数据。
 *
 * 不负责：
 * - 创建界面控件；
 * - 直接写 SQL；
 * - CSV 行解析细节。
 */
class PostController : public QObject
{
    Q_OBJECT

public:
    explicit PostController(PostManagementPage *view,
                            QObject *parent = nullptr);

    void setCurrentUser(const User& user);

private slots:
    void handleRefreshPosts(const QString& platform,
                            const QString& keyword);

    void handleAddPost(const Post& post);

    void handleUpdatePost(const Post& post);

    void handleDeletePost(int postId);

    void handleLoadPost(int postId);

    void handleImportCsv(const QString& fileName);

private:
    int currentOperatorId() const;

    QString currentOperatorName() const;

    void writePostOperationLog(const QString& action,
                               const QString& detail,
                               const QString& result = QStringLiteral("success"));

private:
    PostManagementPage *view;
    User currentUser;

    PostService postService;
    CsvImportService csvImportService;
    LogService logService;
};

#endif // POSTCONTROLLER_H
