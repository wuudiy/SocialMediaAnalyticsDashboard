#include "postmanagementpage.h"
#include "ui_postmanagementpage.h"

#include "../services/appstyle.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QIODevice>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextStream>
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

PostManagementPage::PostManagementPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::PostManagementPage),
    editingPostId(-1)
{
    /*
     * setupUi() 会读取 forms/postmanagementpage.ui，
     * 并创建页面标题、表单卡片、搜索栏、表格和消息提示等固定控件。
     */
    ui->setupUi(this);

    prepareUiObjects();
    setupTable();
    connectSignals();
    applyStyleSheet();

    refreshPosts();
}

PostManagementPage::~PostManagementPage()
{
    delete ui;
}

void PostManagementPage::setCurrentUser(const User& user)
{
    currentUser = user;
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
    ui->platformComboBox->addItems({
        QStringLiteral("Weibo"),
        QStringLiteral("Douyin"),
        QStringLiteral("Bilibili"),
        QStringLiteral("Xiaohongshu"),
        QStringLiteral("Wechat")
    });

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

    // 初始化搜索平台下拉框。currentData() 是实际查询条件，显示文本只是给用户看。
    ui->searchPlatformComboBox->clear();
    ui->searchPlatformComboBox->addItem(QStringLiteral("All Platforms"), QString());
    ui->searchPlatformComboBox->addItem(QStringLiteral("Weibo"), QStringLiteral("Weibo"));
    ui->searchPlatformComboBox->addItem(QStringLiteral("Douyin"), QStringLiteral("Douyin"));
    ui->searchPlatformComboBox->addItem(QStringLiteral("Bilibili"), QStringLiteral("Bilibili"));
    ui->searchPlatformComboBox->addItem(QStringLiteral("Xiaohongshu"), QStringLiteral("Xiaohongshu"));
    ui->searchPlatformComboBox->addItem(QStringLiteral("Wechat"), QStringLiteral("Wechat"));

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
    const QString platform = ui->searchPlatformComboBox
                                 ? ui->searchPlatformComboBox->currentData().toString()
                                 : QString();

    const QString keyword = ui->keywordLineEdit
                                ? ui->keywordLineEdit->text().trimmed()
                                : QString();

    fillTable(postRepository.findPosts(platform, keyword));
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

    setMessage(QStringLiteral("Loaded %1 post records. Double-click one row to edit it.").arg(posts.size()));
}

void PostManagementPage::onAddPostClicked()
{
    Post post = readPostFromForm();

    QString message;
    if (!validatePostInput(post, message)) {
        setMessage(message, true);

        writePostOperationLog(
            QStringLiteral("add_post"),
            QStringLiteral("Add post failed: %1").arg(message),
            QStringLiteral("failed")
            );

        QMessageBox::warning(this, QStringLiteral("Invalid Input"), message);
        return;
    }

    if (!postRepository.insertPost(post)) {
        const QString errorMessage = QStringLiteral("Failed to add post. Please check the database.");
        setMessage(errorMessage, true);

        writePostOperationLog(
            QStringLiteral("add_post"),
            QStringLiteral("Add post failed: database insert failed. Platform: %1, Account: %2, Date: %3")
                .arg(post.platform,
                     post.accountName,
                     post.publishDate.toString(QStringLiteral("yyyy-MM-dd"))),
            QStringLiteral("failed")
            );

        QMessageBox::warning(this, QStringLiteral("Add Failed"), ui->messageLabel->text());
        return;
    }

    writePostOperationLog(
        QStringLiteral("add_post"),
        QStringLiteral("Add post successful. Platform: %1, Account: %2, Date: %3, Likes: %4, Comments: %5, Shares: %6, Views: %7")
            .arg(post.platform,
                 post.accountName,
                 post.publishDate.toString(QStringLiteral("yyyy-MM-dd")))
            .arg(post.likes)
            .arg(post.comments)
            .arg(post.shares)
            .arg(post.views),
        QStringLiteral("success")
        );

    resetForm();
    refreshPosts();
    setMessage(QStringLiteral("Post added successfully."));
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

    QString message;
    if (!validatePostInput(post, message)) {
        setMessage(message, true);

        writePostOperationLog(
            QStringLiteral("update_post"),
            QStringLiteral("Update post failed: %1 Post ID: %2")
                .arg(message)
                .arg(editingPostId),
            QStringLiteral("failed")
            );

        QMessageBox::warning(this, QStringLiteral("Invalid Input"), message);
        return;
    }

    if (!postRepository.updatePost(post)) {
        const QString errorMessage = QStringLiteral("Failed to update post. The record may have been deleted.");
        setMessage(errorMessage, true);

        writePostOperationLog(
            QStringLiteral("update_post"),
            QStringLiteral("Update post failed. Post ID: %1").arg(editingPostId),
            QStringLiteral("failed")
            );

        QMessageBox::warning(this, QStringLiteral("Update Failed"), errorMessage);
        return;
    }

    writePostOperationLog(
        QStringLiteral("update_post"),
        QStringLiteral("Update post successful. Post ID: %1, Platform: %2, Account: %3, Date: %4, Likes: %5, Comments: %6, Shares: %7, Views: %8")
            .arg(post.postId)
            .arg(post.platform,
                 post.accountName,
                 post.publishDate.toString(QStringLiteral("yyyy-MM-dd")))
            .arg(post.likes)
            .arg(post.comments)
            .arg(post.shares)
            .arg(post.views),
        QStringLiteral("success")
        );

    resetForm();
    refreshPosts();
    setMessage(QStringLiteral("Post updated successfully."));
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

    if (!postRepository.deletePostById(postId)) {
        setMessage(QStringLiteral("Failed to delete post."), true);

        writePostOperationLog(
            QStringLiteral("delete_post"),
            QStringLiteral("Delete post failed. Post ID: %1").arg(postId),
            QStringLiteral("failed")
            );

        QMessageBox::warning(this, QStringLiteral("Delete Failed"), ui->messageLabel->text());
        return;
    }

    writePostOperationLog(
        QStringLiteral("delete_post"),
        QStringLiteral("Delete post successful. Post ID: %1").arg(postId),
        QStringLiteral("success")
        );

    if (editingPostId == postId) {
        clearEditingState();
    }

    refreshPosts();
    setMessage(QStringLiteral("Post deleted successfully."));
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
        QStringLiteral("CSV Files (*.csv);;All Files (*.*)")
        );

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        writePostOperationLog(
            QStringLiteral("import_csv"),
            QStringLiteral("CSV import failed: cannot open file. File: %1")
                .arg(QFileInfo(fileName).fileName()),
            QStringLiteral("failed")
            );

        QMessageBox::warning(
            this,
            QStringLiteral("Import Failed"),
            QStringLiteral("Cannot open selected CSV file.")
            );
        return;
    }

    QTextStream stream(&file);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    stream.setEncoding(QStringConverter::Utf8);
#else
    stream.setCodec("UTF-8");
#endif

    int lineNumber = 0;
    int successCount = 0;
    int failedCount = 0;
    CsvFormat csvFormat = CsvFormat::Unknown;
    QStringList errorDetails;

    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        ++lineNumber;

        if (line.isEmpty()) {
            continue;
        }

        const QStringList fields = splitCsvLine(line);

        if (csvFormat == CsvFormat::Unknown) {
            csvFormat = detectCsvFormat(fields);

            if (csvFormat != CsvFormat::Unknown) {
                continue;
            }

            csvFormat = CsvFormat::StandardPost;
        }

        if (csvFormat == CsvFormat::BilibiliTrend
            && cleanCsvField(fields.value(0)) == QStringLiteral("累计")) {
            continue;
        }

        Post post;
        QString message;

        if (!buildPostFromCsvFields(fields, csvFormat, post, message)
            || !postRepository.insertPost(post)) {
            ++failedCount;

            if (errorDetails.size() < 5) {
                errorDetails.append(
                    QStringLiteral("Line %1: %2")
                        .arg(lineNumber)
                        .arg(message.isEmpty()
                                 ? QStringLiteral("Insert database failed.")
                                 : message)
                    );
            }

            continue;
        }

        ++successCount;
    }

    refreshPosts();

    QString resultMessage = QStringLiteral("CSV import finished. Success: %1, Failed: %2.")
                                .arg(successCount)
                                .arg(failedCount);

    if (!errorDetails.isEmpty()) {
        resultMessage += QStringLiteral("\n") + errorDetails.join(QStringLiteral("\n"));
    }

    const bool hasFailedRows = failedCount > 0;

    writePostOperationLog(
        QStringLiteral("import_csv"),
        QStringLiteral("CSV import finished. File: %1, Success: %2, Failed: %3%4")
            .arg(QFileInfo(fileName).fileName())
            .arg(successCount)
            .arg(failedCount)
            .arg(errorDetails.isEmpty()
                     ? QString()
                     : QStringLiteral(", Errors: %1").arg(errorDetails.join(QStringLiteral(" | ")))),
        hasFailedRows ? QStringLiteral("failed") : QStringLiteral("success")
        );

    setMessage(resultMessage, hasFailedRows);
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

    const Post post = postRepository.findPostById(postId);

    if (!post.isValid()) {
        QMessageBox::warning(
            this,
            QStringLiteral("Load Failed"),
            QStringLiteral("The selected post record does not exist.")
            );

        refreshPosts();
        return;
    }

    loadPostToForm(post);

    setMessage(
        QStringLiteral("Editing post ID %1. Modify the form and click Update Selected.")
            .arg(post.postId)
        );
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

bool PostManagementPage::validatePostInput(const Post& post,
                                           QString& message) const
{
    if (post.platform.trimmed().isEmpty()) {
        message = QStringLiteral("Platform cannot be empty.");
        return false;
    }

    if (post.accountName.trimmed().isEmpty()) {
        message = QStringLiteral("Account name cannot be empty.");
        return false;
    }

    if (post.content.trimmed().isEmpty()) {
        message = QStringLiteral("Content cannot be empty.");
        return false;
    }

    if (!post.publishDate.isValid()) {
        message = QStringLiteral("Publish date is invalid.");
        return false;
    }

    return true;
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

void PostManagementPage::setMessage(const QString& message,
                                    bool error)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}

int PostManagementPage::currentOperatorId() const
{
    return currentUser.isValid() ? currentUser.userId : -1;
}

QString PostManagementPage::currentOperatorName() const
{
    if (currentUser.isValid() && !currentUser.username.trimmed().isEmpty()) {
        return currentUser.username.trimmed();
    }

    return QStringLiteral("unknown");
}

void PostManagementPage::writePostOperationLog(const QString& action,
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

QString PostManagementPage::cleanCsvField(const QString& value) const
{
    QString field = value.trimmed();

    // 去掉 UTF-8 BOM，避免 CSV 第一列头部出现不可见字符。
    if (!field.isEmpty() && field.at(0).unicode() == 0xFEFF) {
        field.remove(0, 1);
    }

    // 支持 "a,b" 这种带引号的 CSV 字段，并把 CSV 转义的 "" 还原为 "。
    if (field.size() >= 2
        && field.startsWith(QChar('"'))
        && field.endsWith(QChar('"'))) {
        field = field.mid(1, field.size() - 2);
        field.replace(QStringLiteral("\"\""), QStringLiteral("\""));
    }

    return field.trimmed();
}

QStringList PostManagementPage::splitCsvLine(const QString& line) const
{
    QStringList fields;
    QString current;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        const QChar ch = line.at(i);

        if (ch == QChar('"')) {
            if (inQuotes && i + 1 < line.size() && line.at(i + 1) == QChar('"')) {
                current.append(QChar('"'));
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == QChar(',') && !inQuotes) {
            fields.append(cleanCsvField(current));
            current.clear();
        } else {
            current.append(ch);
        }
    }

    fields.append(cleanCsvField(current));
    return fields;
}

QDate PostManagementPage::parseCsvDate(const QString& value) const
{
    QString dateText = cleanCsvField(value);

    // 有些 CSV 日期会带时间，例如 2026-05-25 12:00:00，这里只保留日期部分。
    if (dateText.contains(QChar(' '))) {
        dateText = dateText.section(QChar(' '), 0, 0);
    }

    const QStringList formats = {
        QStringLiteral("yyyy-MM-dd"),
        QStringLiteral("yyyy-M-d"),
        QStringLiteral("yyyy/MM/dd"),
        QStringLiteral("yyyy/M/d")
    };

    for (const QString& format : formats) {
        const QDate date = QDate::fromString(dateText, format);

        if (date.isValid()) {
            return date;
        }
    }

    return QDate();
}

PostManagementPage::CsvFormat PostManagementPage::detectCsvFormat(const QStringList& fields) const
{
    QStringList headers;

    for (const QString& field : fields) {
        headers.append(cleanCsvField(field).toLower());
    }

    const QString joinedHeader = headers.join(QStringLiteral(","));

    // 通用帖子 CSV：platform, account, content, date, likes, comments, shares, views
    if (joinedHeader.contains(QStringLiteral("platform"))
        && joinedHeader.contains(QStringLiteral("account"))
        && joinedHeader.contains(QStringLiteral("views"))) {
        return CsvFormat::StandardPost;
    }

    // Bilibili 趋势 CSV：按中文表头识别。
    if (joinedHeader.contains(QStringLiteral("时间"))
        && joinedHeader.contains(QStringLiteral("播放量"))
        && joinedHeader.contains(QStringLiteral("点赞"))) {
        return CsvFormat::BilibiliTrend;
    }

    return CsvFormat::Unknown;
}

bool PostManagementPage::buildPostFromCsvFields(const QStringList& fields,
                                                CsvFormat format,
                                                Post& post,
                                                QString& message) const
{
    auto csvNumber = [this](const QString& value) -> int {
        QString numberText = cleanCsvField(value);
        numberText.remove(QChar(','));
        return numberText.toInt();
    };

    if (format == CsvFormat::StandardPost) {
        if (fields.size() < 8) {
            message = QStringLiteral("Standard CSV row must contain 8 fields.");
            return false;
        }

        post.platform = cleanCsvField(fields.at(0));
        post.accountName = cleanCsvField(fields.at(1));
        post.content = cleanCsvField(fields.at(2));
        post.publishDate = parseCsvDate(fields.at(3));
        post.likes = csvNumber(fields.at(4));
        post.comments = csvNumber(fields.at(5));
        post.shares = csvNumber(fields.at(6));
        post.views = csvNumber(fields.at(7));

        return validatePostInput(post, message);
    }

    if (format == CsvFormat::BilibiliTrend) {
        if (fields.size() < 10) {
            message = QStringLiteral("Bilibili trend CSV row must contain 10 fields.");
            return false;
        }

        const QString dateText = cleanCsvField(fields.at(0));

        post.platform = QStringLiteral("Bilibili");
        post.accountName = QStringLiteral("Bilibili Video");
        post.content = QStringLiteral("Bilibili playback trend - %1").arg(dateText);
        post.publishDate = parseCsvDate(dateText);
        post.views = csvNumber(fields.at(1));
        post.likes = csvNumber(fields.at(4));
        post.comments = csvNumber(fields.at(5)) + csvNumber(fields.at(6));
        post.shares = csvNumber(fields.at(7));

        return validatePostInput(post, message);
    }

    message = QStringLiteral("Unsupported CSV format.");
    return false;
}
