#ifndef POSTMANAGEMENTPAGE_H
#define POSTMANAGEMENTPAGE_H

#include "../models/post.h"
#include "../models/user.h"
#include "../services/logservice.h"
#include "../services/postrepository.h"

#include <QWidget>
#include <QStringList>

class QComboBox;
class QDateEdit;
class QFrame;
class QGridLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

/*
 * 帖子数据管理页面。
 *
 * 负责：
 * - 新增社交媒体帖子数据；
 * - 按平台和关键词查询；
 * - 表格展示帖子列表；
 * - 删除选中的帖子；
 * - 从 CSV 文件批量导入数据；
 * - 记录帖子新增、删除、CSV 导入日志。
 *
 * 不负责：
 * - 统计分析计算；
 * - Dashboard 展示；
 * - 用户登录验证。
 */
class PostManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit PostManagementPage(QWidget *parent = nullptr);

    // 主窗口在登录成功后调用，用于记录当前操作人。
    void setCurrentUser(const User& user);

public slots:
    // 主窗口切换到本页面时调用，保证表格显示最新数据。
    void refreshPosts();

private slots:
    void onAddPostClicked();
    void onDeletePostClicked();
    void onSearchClicked();
    void onResetSearchClicked();
    void onImportCsvClicked();

private:
    enum class CsvFormat
    {
        Unknown,
        StandardPost,
        BilibiliTrend
    };

    // 创建页面整体布局。
    void buildUi();

    // 当前页面统一样式。
    void applyStyleSheet();

    // 创建新增帖子表单卡片。
    QFrame* createFormCard();

    // 创建查询和表格区域。
    QFrame* createTableCard();

    // 创建字段名，统一样式。
    QLabel* createFieldLabel(const QString& text);

    // 添加一行表单项。
    void addFormRow(QGridLayout *layout,
                    int row,
                    const QString& labelText,
                    QWidget *field);

    // 初始化表格列。
    void setupTable();

    // 把查询结果填充到表格。
    void fillTable(const QList<Post>& posts);

    // 从表单读取帖子数据。
    Post readPostFromForm() const;

    // 表单输入校验。
    bool validatePostInput(const Post& post,
                           QString& message) const;

    // 清空新增表单。
    void resetForm();

    // 获取当前选中的帖子 ID。
    int selectedPostId() const;

    // 页面内提示信息统一从这里设置。
    void setMessage(const QString& message,
                    bool error = false);

    // 获取当前操作人 ID。未登录或未传入时返回 -1。
    int currentOperatorId() const;

    // 获取当前操作人用户名。未登录或未传入时返回 unknown。
    QString currentOperatorName() const;

    // 统一写入帖子模块日志，避免每个槽函数重复写 userId 和 username。
    void writePostOperationLog(const QString& action,
                               const QString& detail,
                               const QString& result = QStringLiteral("success"));

    // 清理 CSV 字段，比如去掉 BOM、引号和多余空格。
    QString cleanCsvField(const QString& value) const;

    // 解析一行 CSV，支持简单的引号字段。
    QStringList splitCsvLine(const QString& line) const;

    // 解析 CSV 里的日期，同时支持 yyyy-MM-dd 和 yyyy/MM/dd。
    QDate parseCsvDate(const QString& value) const;

    // 判断 CSV 是项目标准格式，还是 Bilibili 导出的趋势数据。
    CsvFormat detectCsvFormat(const QStringList& fields) const;

    // 根据 CSV 字段创建 Post。
    bool buildPostFromCsvFields(const QStringList& fields,
                                CsvFormat format,
                                Post& post,
                                QString& message) const;

private:
    User currentUser;

    PostRepository postRepository;
    LogService logService;

    QLabel *messageLabel;

    QComboBox *platformComboBox;
    QLineEdit *accountLineEdit;
    QLineEdit *contentLineEdit;
    QDateEdit *publishDateEdit;
    QSpinBox *likesSpinBox;
    QSpinBox *commentsSpinBox;
    QSpinBox *sharesSpinBox;
    QSpinBox *viewsSpinBox;
    QPushButton *addButton;
    QPushButton *clearButton;
    QPushButton *importCsvButton;

    QComboBox *searchPlatformComboBox;
    QLineEdit *keywordLineEdit;
    QPushButton *searchButton;
    QPushButton *resetSearchButton;
    QPushButton *deleteButton;
    QPushButton *refreshButton;
    QTableWidget *postTable;
};

#endif // POSTMANAGEMENTPAGE_H
