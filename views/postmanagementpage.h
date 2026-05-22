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
 * - 修改选中的帖子数据；
 * - 按平台和关键词查询；
 * - 表格展示帖子列表；
 * - 删除选中的帖子；
 * - 从 CSV 文件批量导入数据；
 * - 记录帖子新增、修改、删除、CSV 导入日志。
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

    void buildUi();
    void applyStyleSheet();

    QFrame* createFormCard();
    QFrame* createTableCard();

    QLabel* createFieldLabel(const QString& text);

    void addFormRow(QGridLayout *layout,
                    int row,
                    const QString& labelText,
                    QWidget *field);

    void setupTable();
    void fillTable(const QList<Post>& posts);

    Post readPostFromForm() const;
    bool validatePostInput(const Post& post,
                           QString& message) const;

    void resetForm();

    // 将一条数据库帖子加载到表单，用于修改。
    void loadPostToForm(const Post& post);

    // 退出编辑状态。
    void clearEditingState();

    int selectedPostId() const;

    void setMessage(const QString& message,
                    bool error = false);

    int currentOperatorId() const;
    QString currentOperatorName() const;

    void writePostOperationLog(const QString& action,
                               const QString& detail,
                               const QString& result = QStringLiteral("success"));

    QString cleanCsvField(const QString& value) const;
    QStringList splitCsvLine(const QString& line) const;
    QDate parseCsvDate(const QString& value) const;

    CsvFormat detectCsvFormat(const QStringList& fields) const;

    bool buildPostFromCsvFields(const QStringList& fields,
                                CsvFormat format,
                                Post& post,
                                QString& message) const;

private:
    User currentUser;

    PostRepository postRepository;
    LogService logService;

    int editingPostId;

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
    QPushButton *updateButton;
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
