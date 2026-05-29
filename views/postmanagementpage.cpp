#include "postmanagementpage.h"
#include "ui_postmanagementpage.h"

#include "../controllers/postcontroller.h"
#include "../styles/appstyle.h"
#include "../utils/platformconstants.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>

PostManagementPage::PostManagementPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::PostManagementPage),
    postController(nullptr),
    editingPostId(-1)
{
    /*
     * setupUi() 会读取 forms/PostManagementPage.ui，
     * 并创建页面标题、表单卡片、搜索栏、表格和消息提示等固定控件。
     */
    ui->setupUi(this);

    prepareUiObjects();
    setupTable();
    connectSignals();
    applyStyleSheet();

    /*
     * 当前批次为了减少 MainWindow 改动，Controller 先由页面内部创建。
     * 后续重构 MainWindowController 时，可以再改成外部统一注入。
     */
    postController = new PostController(this, this);

    refreshPosts();
}

PostManagementPage::~PostManagementPage()
{
    delete ui;
}

void PostManagementPage::setCurrentUser(const User& user)
{
    currentUser = user;

    if (postController) {
        postController->setCurrentUser(user);
    }
}

/*
 * 初始化 .ui 中控件的运行时属性。
 *
 * 说明：
 * .ui 文件负责“控件在哪里”，这里负责“控件运行时怎么用”。
 * 这样做的好处是：后续你在 Qt Designer 里调整布局，不会影响业务逻辑。
 */
void PostManagementPage::prepareUiObjects()
{
    setObjectName(QStringLiteral("postManagementPage"));

    // 这些 objectName 对应 AppStyle::dataManagementPageStyle() 中的 QSS 选择器。
    ui->pageTitleLabel->setObjectName(QStringLiteral("pageTitle"));
    ui->pageSubtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));
    ui->formCard->setObjectName(QStringLiteral("card"));
    ui->tableCard->setObjectName(QStringLiteral("card"));
    ui->messageLabel->setObjectName(QStringLiteral("messageLabel"));
    ui->messageLabel->setWordWrap(true);

    // 字段名统一改成 fieldLabel，方便 AppStyle 统一控制字体颜色和粗细。
    const QList<QLabel*> fieldLabels = {
        ui->platformLabel,
        ui->accountLabel,
        ui->contentLabel,
        ui->dateLabel,
        ui->likesLabel,
        ui->commentsLabel,
        ui->sharesLabel,
        ui->viewsLabel
    };

    for (QLabel *label : fieldLabels) {
        if (label) {
            label->setObjectName(QStringLiteral("fieldLabel"));
        }
    }

    // 让表单的输入列自动占满剩余宽度，保持和原代码版本一致。
    ui->formLayout->setColumnStretch(1, 1);
    ui->formLayout->setColumnStretch(3, 1);

    // 初始化新增/编辑表单的平台下拉框。
    ui->platformComboBox->clear();
    ui->platformComboBox->addItems(PlatformConstants::availablePlatforms());

    ui->accountLineEdit->setPlaceholderText(QStringLiteral("Account name"));
    ui->contentLineEdit->setPlaceholderText(QStringLiteral("Post title or content summary"));

    ui->publishDateEdit->setDate(QDate::currentDate());
    ui->publishDateEdit->setCalendarPopup(true);
    ui->publishDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    // 课程作业中用 int 保存互动数据，所以这里最大值也保持 int 友好范围。
    const int maxNumber = 100000000;
    ui->likesSpinBox->setMaximum(maxNumber);
    ui->commentsSpinBox->setMaximum(maxNumber);
    ui->sharesSpinBox->setMaximum(maxNumber);
    ui->viewsSpinBox->setMaximum(maxNumber);

    /*
     * 初始化搜索平台下拉框。
     *
     * currentData() 是实际查询条件：
     * - 空字符串：查询全部平台；
     * - 平台名：按平台过滤。
     */
    ui->searchPlatformComboBox->clear();
    ui->searchPlatformComboBox->addItem(QStringLiteral("All Platforms"), QString());

    for (const QString& platform : PlatformConstants::availablePlatforms()) {
        ui->searchPlatformComboBox->addItem(platform, platform);
    }

    ui->keywordLineEdit->setPlaceholderText(QStringLiteral("Search content or account"));

    // 按钮样式通过 objectName 绑定 AppStyle，业务代码只负责设置状态和点击逻辑。
    ui->addButton->setObjectName(QStringLiteral("primaryButton"));
    ui->updateButton->setObjectName(QStringLiteral("successButton"));
    ui->clearButton->setObjectName(QStringLiteral("secondaryButton"));
    ui->importCsvButton->setObjectName(QStringLiteral("secondaryButton"));
    ui->searchButton->setObjectName(QStringLiteral("primaryButton"));
    ui->resetSearchButton->setObjectName(QStringLiteral("secondaryButton"));
    ui->refreshButton->setObjectName(QStringLiteral("secondaryButton"));
    ui->deleteButton->setObjectName(QStringLiteral("dangerButton"));

    const QList<QPushButton*> buttons = {
        ui->addButton,
        ui->updateButton,
        ui->clearButton,
        ui->importCsvButton,
        ui->searchButton,
        ui->resetSearchButton,
        ui->refreshButton,
        ui->deleteButton
    };

    for (QPushButton *button : buttons) {
        if (button) {
            button->setCursor(Qt::PointingHandCursor);
        }
    }

    ui->updateButton->setEnabled(false);
}

/*
 * 集中连接信号槽。
 *
 * .ui 文件中不要直接写业务逻辑连接，
 * 这样页面行为都能在 cpp 中统一查看和维护。
 */
void PostManagementPage::connectSignals()
{
    connect(ui->addButton, &QPushButton::clicked,
            this, &PostManagementPage::onAddPostClicked);

    connect(ui->updateButton, &QPushButton::clicked,
            this, &PostManagementPage::onUpdatePostClicked);

    connect(ui->clearButton, &QPushButton::clicked,
            this, &PostManagementPage::resetForm);

    connect(ui->importCsvButton, &QPushButton::clicked,
            this, &PostManagementPage::onImportCsvClicked);

    connect(ui->searchButton, &QPushButton::clicked,
            this, &PostManagementPage::onSearchClicked);

    connect(ui->resetSearchButton, &QPushButton::clicked,
            this, &PostManagementPage::onResetSearchClicked);

    connect(ui->deleteButton, &QPushButton::clicked,
            this, &PostManagementPage::onDeletePostClicked);

    connect(ui->refreshButton, &QPushButton::clicked,
            this, &PostManagementPage::refreshPosts);

    connect(ui->postTable, &QTableWidget::cellDoubleClicked,
            this, &PostManagementPage::onTableCellDoubleClicked);
}

void PostManagementPage::applyStyleSheet()
{
    setStyleSheet(AppStyle::dataManagementPageStyle());
}

void PostManagementPage::setupTable()
{
    ui->postTable->setColumnCount(10);

    ui->postTable->setHorizontalHeaderLabels({
        QStringLiteral("ID"),
        QStringLiteral("Platform"),
        QStringLiteral("Account"),
        QStringLiteral("Content"),
        QStringLiteral("Date"),
        QStringLiteral("Likes"),
        QStringLiteral("Comments"),
        QStringLiteral("Shares"),
        QStringLiteral("Views"),
        QStringLiteral("Engagement")
    });

    ui->postTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->postTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->postTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->postTable->setAlternatingRowColors(true);
    ui->postTable->verticalHeader()->setVisible(false);
    ui->postTable->horizontalHeader()->setStretchLastSection(true);
    ui->postTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
}

void PostManagementPage::refreshPosts()
{
    emit refreshPostsRequested(
        currentSearchPlatform(),
        currentSearchKeyword()
        );
}

void PostManagementPage::showPosts(const QList<Post>& posts)
{
    fillTable(posts);

    showMessage(
        QStringLiteral("Loaded %1 post records. Double-click one row to edit it.")
            .arg(posts.size())
        );
}

void PostManagementPage::fillTable(const QList<Post>& posts)
{
    ui->postTable->setRowCount(posts.size());

    for (int row = 0; row < posts.size(); ++row) {
        const Post post = posts.at(row);

        const QString engagementText = QStringLiteral("%1%")
                                           .arg(post.engagementRate() * 100.0, 0, 'f', 2);

        const QStringList values = {
            QString::number(post.postId),
            post.platform,
            post.accountName,
            post.content,
            post.publishDate.toString(QStringLiteral("yyyy-MM-dd")),
            QString::number(post.likes),
            QString::number(post.comments),
            QString::number(post.shares),
            QString::number(post.views),
            engagementText
        };

        for (int column = 0; column < values.size(); ++column) {
            auto *item = new QTableWidgetItem(values.at(column));

            if (column == 0 || column >= 5) {
                item->setTextAlignment(Qt::AlignCenter);
            }

            ui->postTable->setItem(row, column, item);
        }
    }
}

void PostManagementPage::onAddPostClicked()
{
    /*
     * View 只负责读取用户输入，然后把请求交给 Controller。
     * 校验、入库、日志都不在 View 中处理。
     */
    emit addPostRequested(readPostFromForm());
}

void PostManagementPage::onUpdatePostClicked()
{
    if (editingPostId <= 0) {
        QMessageBox::information(
            this,
            QStringLiteral("No Editing Post"),
            QStringLiteral("Please double-click one row in the table first.")
            );

        return;
    }

    Post post = readPostFromForm();
    post.postId = editingPostId;

    emit updatePostRequested(post);
}

void PostManagementPage::onDeletePostClicked()
{
    const int postId = selectedPostId();

    if (postId <= 0) {
        QMessageBox::information(
            this,
            QStringLiteral("No Selection"),
            QStringLiteral("Please select one post record first.")
            );
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this,
        QStringLiteral("Delete Post"),
        QStringLiteral("Are you sure you want to delete the selected post?")
        );

    if (answer != QMessageBox::Yes) {
        return;
    }

    emit deletePostRequested(postId);
}

void PostManagementPage::onSearchClicked()
{
    refreshPosts();
}

void PostManagementPage::onResetSearchClicked()
{
    ui->searchPlatformComboBox->setCurrentIndex(0);
    ui->keywordLineEdit->clear();

    refreshPosts();
}

void PostManagementPage::onImportCsvClicked()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        QStringLiteral("Import CSV"),
        QString(),
        QStringLiteral("CSV Text Files (*.csv *.CSV);;All Files (*.*)")
        );

    if (fileName.isEmpty()) {
        return;
    }

    emit importCsvRequested(fileName);
}

void PostManagementPage::onTableCellDoubleClicked(int row, int column)
{
    Q_UNUSED(column)

    if (row < 0) {
        return;
    }

    const QTableWidgetItem *idItem = ui->postTable->item(row, 0);

    if (!idItem) {
        return;
    }

    const int postId = idItem->text().toInt();

    if (postId <= 0) {
        return;
    }

    emit loadPostRequested(postId);
}

Post PostManagementPage::readPostFromForm() const
{
    Post post;

    post.platform = ui->platformComboBox->currentText();
    post.accountName = ui->accountLineEdit->text();
    post.content = ui->contentLineEdit->text();
    post.publishDate = ui->publishDateEdit->date();
    post.likes = ui->likesSpinBox->value();
    post.comments = ui->commentsSpinBox->value();
    post.shares = ui->sharesSpinBox->value();
    post.views = ui->viewsSpinBox->value();

    return post;
}

void PostManagementPage::resetForm()
{
    ui->platformComboBox->setCurrentIndex(0);
    ui->accountLineEdit->clear();
    ui->contentLineEdit->clear();
    ui->publishDateEdit->setDate(QDate::currentDate());
    ui->likesSpinBox->setValue(0);
    ui->commentsSpinBox->setValue(0);
    ui->sharesSpinBox->setValue(0);
    ui->viewsSpinBox->setValue(0);
    ui->accountLineEdit->setFocus();

    clearEditingState();
}

void PostManagementPage::showPostForEditing(const Post& post)
{
    loadPostToForm(post);

    showMessage(
        QStringLiteral("Editing post ID %1. Modify the form and click Update Selected.")
            .arg(post.postId)
        );
}

void PostManagementPage::loadPostToForm(const Post& post)
{
    editingPostId = post.postId;

    int platformIndex = ui->platformComboBox->findText(post.platform);

    if (platformIndex < 0) {
        ui->platformComboBox->addItem(post.platform);
        platformIndex = ui->platformComboBox->findText(post.platform);
    }

    ui->platformComboBox->setCurrentIndex(platformIndex);
    ui->accountLineEdit->setText(post.accountName);
    ui->contentLineEdit->setText(post.content);
    ui->publishDateEdit->setDate(post.publishDate);
    ui->likesSpinBox->setValue(post.likes);
    ui->commentsSpinBox->setValue(post.comments);
    ui->sharesSpinBox->setValue(post.shares);
    ui->viewsSpinBox->setValue(post.views);

    ui->updateButton->setEnabled(true);
    ui->addButton->setEnabled(false);
}

void PostManagementPage::clearEditingState()
{
    editingPostId = -1;

    if (ui->updateButton) {
        ui->updateButton->setEnabled(false);
    }

    if (ui->addButton) {
        ui->addButton->setEnabled(true);
    }
}

int PostManagementPage::selectedPostId() const
{
    const QList<QTableWidgetItem*> selectedItems = ui->postTable->selectedItems();

    if (selectedItems.isEmpty()) {
        return -1;
    }

    const int row = selectedItems.first()->row();
    const QTableWidgetItem *idItem = ui->postTable->item(row, 0);

    if (!idItem) {
        return -1;
    }

    return idItem->text().toInt();
}

QString PostManagementPage::currentSearchPlatform() const
{
    if (!ui->searchPlatformComboBox) {
        return QString();
    }

    return ui->searchPlatformComboBox->currentData().toString().trimmed();
}

QString PostManagementPage::currentSearchKeyword() const
{
    if (!ui->keywordLineEdit) {
        return QString();
    }

    return ui->keywordLineEdit->text().trimmed();
}

void PostManagementPage::showMessage(const QString& message,
                                     bool error)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}

void PostManagementPage::showWarningMessage(const QString& title,
                                            const QString& message)
{
    QMessageBox::warning(this, title, message);
}

void PostManagementPage::handleAddSuccess(const QString& message)
{
    resetForm();
    refreshPosts();
    showMessage(message);
}

void PostManagementPage::handleUpdateSuccess(const QString& message)
{
    resetForm();
    refreshPosts();
    showMessage(message);
}

void PostManagementPage::handleDeleteSuccess(int deletedPostId,
                                             const QString& message)
{
    if (editingPostId == deletedPostId) {
        clearEditingState();
    }

    refreshPosts();
    showMessage(message);
}

void PostManagementPage::handleImportFinished(const QString& message,
                                              bool hasFailedRows)
{
    refreshPosts();
    showMessage(message, hasFailedRows);
}
