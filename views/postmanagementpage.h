#ifndef POSTMANAGEMENTPAGE_H
#define POSTMANAGEMENTPAGE_H

#include "../models/post.h"
#include "../models/user.h"
#include "../services/logservice.h"
#include "../services/postrepository.h"

#include <QDate>
#include <QList>
#include <QStringList>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class PostManagementPage;
}
QT_END_NAMESPACE

/*
 * 帖子数据管理页面。
 *
 * 当前版本采用 “postmanagementpage.ui + postmanagementpage.cpp” 分工：
 * - forms/postmanagementpage.ui：只负责固定界面结构和控件摆放；
 * - views/postmanagementpage.cpp：只负责控件初始化、信号槽、数据库操作、CSV 导入和日志记录；
 * - services/AppStyle：统一管理页面 QSS 样式。
 *
 * 这样可以减少本 cpp 中大量 new 控件、new layout、addWidget 代码，
 * 后续继续调整 UI 时，也可以优先在 Qt Designer 中拖控件完成。
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

private slots:
    void onAddPostClicked();
    void onUpdatePostClicked();
    void onDeletePostClicked();
    void onSearchClicked();
    void onResetSearchClicked();
    void onImportCsvClicked();
    void onTableCellDoubleClicked(int row, int column);

private:
    enum class CsvFormat
    {
        Unknown,
        StandardPost,
        BilibiliTrend
    };

    // 初始化 .ui 中已有控件的运行时属性，例如下拉框数据、按钮样式名、日期默认值。
    void prepareUiObjects();

    // 连接 .ui 中控件的信号槽。
    void connectSignals();

    // 应用统一样式，具体 QSS 不写在本类中。
    void applyStyleSheet();

    // 初始化帖子表格的列名、选择模式和列宽策略。
    void setupTable();

    // 把数据库查询结果填入表格。
    void fillTable(const QList<Post>& posts);

    // 从表单控件读取一条帖子数据。
    Post readPostFromForm() const;

    // 校验帖子表单输入是否合法。
    bool validatePostInput(const Post& post,
                           QString& message) const;

    // 清空表单，并退出编辑状态。
    void resetForm();

    // 将一条数据库帖子加载到表单，用于修改。
    void loadPostToForm(const Post& post);

    // 退出编辑状态。
    void clearEditingState();

    // 获取当前表格选中的帖子 ID。
    int selectedPostId() const;

    // 设置页面底部提示；error=true 时显示红色错误样式。
    void setMessage(const QString& message,
                    bool error = false);

    // 获取当前操作者信息，用于写操作日志。
    int currentOperatorId() const;
    QString currentOperatorName() const;

    // 写帖子相关操作日志。
    void writePostOperationLog(const QString& action,
                               const QString& detail,
                               const QString& result = QStringLiteral("success"));

    // CSV 解析相关工具函数。
    QString cleanCsvField(const QString& value) const;
    QStringList splitCsvLine(const QString& line) const;
    QDate parseCsvDate(const QString& value) const;

    CsvFormat detectCsvFormat(const QStringList& fields) const;

    bool buildPostFromCsvFields(const QStringList& fields,
                                CsvFormat format,
                                Post& post,
                                QString& message) const;

private:
    Ui::PostManagementPage *ui;

    User currentUser;

    PostRepository postRepository;
    LogService logService;

    // -1 表示当前不是编辑状态；大于 0 表示正在编辑某条帖子。
    int editingPostId;
};

#endif // POSTMANAGEMENTPAGE_H
