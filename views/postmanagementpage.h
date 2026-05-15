#ifndef POSTMANAGEMENTPAGE_H
#define POSTMANAGEMENTPAGE_H

#include "../models/post.h"
#include "../services/postrepository.h"

#include <QWidget>

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
 * - 从 CSV 文件批量导入数据。
 *
 * 不负责：
 * - 直接写 SQL；
 * - 计算统计图表；
 * - 管理主窗口导航。
 */
class PostManagementPage : public QWidget
{
    Q_OBJECT

public:
    explicit PostManagementPage(QWidget *parent = nullptr);

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

    // 解析一行简单 CSV。这里适合课程项目使用，不处理复杂引号嵌套。
    QStringList splitCsvLine(const QString& line) const;

    // 根据 CSV 字段创建 Post。
    bool buildPostFromCsvFields(const QStringList& fields,
                                Post& post,
                                QString& message) const;

private:
    PostRepository postRepository;

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
