#include "postmanagementpage.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QIODevice>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
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

#include <QVBoxLayout>

PostManagementPage::PostManagementPage(QWidget *parent)
    : QWidget(parent),
    messageLabel(nullptr),
    platformComboBox(nullptr),
    accountLineEdit(nullptr),
    contentLineEdit(nullptr),
    publishDateEdit(nullptr),
    likesSpinBox(nullptr),
    commentsSpinBox(nullptr),
    sharesSpinBox(nullptr),
    viewsSpinBox(nullptr),
    addButton(nullptr),
    clearButton(nullptr),
    importCsvButton(nullptr),
    searchPlatformComboBox(nullptr),
    keywordLineEdit(nullptr),
    searchButton(nullptr),
    resetSearchButton(nullptr),
    deleteButton(nullptr),
    refreshButton(nullptr),
    postTable(nullptr)
{
    buildUi();
    refreshPosts();
}

// 主窗口登录成功后会调用这个函数，把当前登录用户传进帖子页面。
// 这样新增、删除、导入日志里就能记录是谁操作的。
void PostManagementPage::setCurrentUser(const User& user)
{
    currentUser = user;
}

// 创建页面：上方是新增表单，下方是查询和表格。
void PostManagementPage::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 22, 24, 24);
    rootLayout->setSpacing(16);

    auto *titleLabel = new QLabel(QStringLiteral("Post Data Management"));
    titleLabel->setObjectName(QStringLiteral("pageTitle"));

    auto *subtitleLabel = new QLabel(
        QStringLiteral("Add, search, delete or import local social media post data.")
        );
    subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));

    messageLabel = new QLabel();
    messageLabel->setObjectName(QStringLiteral("messageLabel"));
    messageLabel->setWordWrap(true);

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(subtitleLabel);
    rootLayout->addWidget(createFormCard());
    rootLayout->addWidget(createTableCard(), 1);
    rootLayout->addWidget(messageLabel);

    applyStyleSheet();

    connect(addButton, &QPushButton::clicked,
            this, &PostManagementPage::onAddPostClicked);

    connect(clearButton, &QPushButton::clicked,
            this, &PostManagementPage::resetForm);

    connect(importCsvButton, &QPushButton::clicked,
            this, &PostManagementPage::onImportCsvClicked);

    connect(searchButton, &QPushButton::clicked,
            this, &PostManagementPage::onSearchClicked);

    connect(resetSearchButton, &QPushButton::clicked,
            this, &PostManagementPage::onResetSearchClicked);

    connect(deleteButton, &QPushButton::clicked,
            this, &PostManagementPage::onDeletePostClicked);

    connect(refreshButton, &QPushButton::clicked,
            this, &PostManagementPage::refreshPosts);
}

void PostManagementPage::applyStyleSheet()
{
    setStyleSheet(
        "QLabel#pageTitle {"
        "    color: #111827;"
        "    font-size: 24px;"
        "    font-weight: 700;"
        "}"
        "QLabel#pageSubtitle {"
        "    color: #6B7280;"
        "    font-size: 13px;"
        "}"
        "QFrame#card {"
        "    background: #FFFFFF;"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 12px;"
        "}"
        "QLabel#fieldLabel {"
        "    color: #374151;"
        "    font-size: 13px;"
        "    font-weight: 600;"
        "}"
        "QLineEdit, QComboBox, QDateEdit, QSpinBox {"
        "    min-height: 32px;"
        "    border: 1px solid #D1D5DB;"
        "    border-radius: 8px;"
        "    padding-left: 8px;"
        "    background: #FFFFFF;"
        "}"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus {"
        "    border: 1px solid #2563EB;"
        "}"
        "QPushButton#primaryButton {"
        "    min-height: 34px;"
        "    background: #2563EB;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 0 16px;"
        "    font-weight: 600;"
        "}"
        "QPushButton#primaryButton:hover {"
        "    background: #1D4ED8;"
        "}"
        "QPushButton#secondaryButton {"
        "    min-height: 34px;"
        "    background: #F9FAFB;"
        "    color: #374151;"
        "    border: 1px solid #D1D5DB;"
        "    border-radius: 8px;"
        "    padding: 0 16px;"
        "}"
        "QPushButton#secondaryButton:hover {"
        "    background: #F3F4F6;"
        "}"
        "QPushButton#dangerButton {"
        "    min-height: 34px;"
        "    background: #DC2626;"
        "    color: #FFFFFF;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 0 16px;"
        "    font-weight: 600;"
        "}"
        "QPushButton#dangerButton:hover {"
        "    background: #B91C1C;"
        "}"
        "QTableWidget {"
        "    border: 1px solid #E5E7EB;"
        "    border-radius: 8px;"
        "    gridline-color: #E5E7EB;"
        "    background: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "    background: #F9FAFB;"
        "    color: #374151;"
        "    border: none;"
        "    border-bottom: 1px solid #E5E7EB;"
        "    padding: 8px;"
        "    font-weight: 600;"
        "}"
        "QLabel#messageLabel {"
        "    color: #374151;"
        "    font-size: 13px;"
        "}"
        );
}

QFrame* PostManagementPage::createFormCard()
{
    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));

    auto *layout = new QGridLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setHorizontalSpacing(14);
    layout->setVerticalSpacing(12);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(3, 1);

    platformComboBox = new QComboBox();
    platformComboBox->addItems({
        QStringLiteral("Weibo"),
        QStringLiteral("Douyin"),
        QStringLiteral("Bilibili"),
        QStringLiteral("Xiaohongshu"),
        QStringLiteral("Wechat")
    });

    accountLineEdit = new QLineEdit();
    accountLineEdit->setPlaceholderText(QStringLiteral("Account name"));

    contentLineEdit = new QLineEdit();
    contentLineEdit->setPlaceholderText(QStringLiteral("Post title or content summary"));

    publishDateEdit = new QDateEdit(QDate::currentDate());
    publishDateEdit->setCalendarPopup(true);
    publishDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

    likesSpinBox = new QSpinBox();
    commentsSpinBox = new QSpinBox();
    sharesSpinBox = new QSpinBox();
    viewsSpinBox = new QSpinBox();

    const int maxNumber = 100000000;
    likesSpinBox->setMaximum(maxNumber);
    commentsSpinBox->setMaximum(maxNumber);
    sharesSpinBox->setMaximum(maxNumber);
    viewsSpinBox->setMaximum(maxNumber);

    addButton = new QPushButton(QStringLiteral("Add Post"));
    addButton->setObjectName(QStringLiteral("primaryButton"));
    addButton->setCursor(Qt::PointingHandCursor);

    clearButton = new QPushButton(QStringLiteral("Clear"));
    clearButton->setObjectName(QStringLiteral("secondaryButton"));
    clearButton->setCursor(Qt::PointingHandCursor);

    importCsvButton = new QPushButton(QStringLiteral("Import CSV"));
    importCsvButton->setObjectName(QStringLiteral("secondaryButton"));
    importCsvButton->setCursor(Qt::PointingHandCursor);

    addFormRow(layout, 0, QStringLiteral("Platform"), platformComboBox);
    addFormRow(layout, 0, QStringLiteral("Account"), accountLineEdit);
    addFormRow(layout, 1, QStringLiteral("Content"), contentLineEdit);
    addFormRow(layout, 1, QStringLiteral("Date"), publishDateEdit);
    addFormRow(layout, 2, QStringLiteral("Likes"), likesSpinBox);
    addFormRow(layout, 2, QStringLiteral("Comments"), commentsSpinBox);
    addFormRow(layout, 3, QStringLiteral("Shares"), sharesSpinBox);
    addFormRow(layout, 3, QStringLiteral("Views"), viewsSpinBox);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(clearButton);
    buttonLayout->addWidget(importCsvButton);
    buttonLayout->addStretch();

    layout->addLayout(buttonLayout, 4, 1, 1, 3);

    return card;
}

QFrame* PostManagementPage::createTableCard()
{
    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(12);

    auto *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(10);

    searchPlatformComboBox = new QComboBox();
    searchPlatformComboBox->addItem(QStringLiteral("All Platforms"), QString());
    searchPlatformComboBox->addItem(QStringLiteral("Weibo"), QStringLiteral("Weibo"));
    searchPlatformComboBox->addItem(QStringLiteral("Douyin"), QStringLiteral("Douyin"));
    searchPlatformComboBox->addItem(QStringLiteral("Bilibili"), QStringLiteral("Bilibili"));
    searchPlatformComboBox->addItem(QStringLiteral("Xiaohongshu"), QStringLiteral("Xiaohongshu"));
    searchPlatformComboBox->addItem(QStringLiteral("Wechat"), QStringLiteral("Wechat"));

    keywordLineEdit = new QLineEdit();
    keywordLineEdit->setPlaceholderText(QStringLiteral("Search content or account"));

    searchButton = new QPushButton(QStringLiteral("Search"));
    searchButton->setObjectName(QStringLiteral("primaryButton"));
    searchButton->setCursor(Qt::PointingHandCursor);

    resetSearchButton = new QPushButton(QStringLiteral("Reset"));
    resetSearchButton->setObjectName(QStringLiteral("secondaryButton"));
    resetSearchButton->setCursor(Qt::PointingHandCursor);

    refreshButton = new QPushButton(QStringLiteral("Refresh"));
    refreshButton->setObjectName(QStringLiteral("secondaryButton"));
    refreshButton->setCursor(Qt::PointingHandCursor);

    deleteButton = new QPushButton(QStringLiteral("Delete Selected"));
    deleteButton->setObjectName(QStringLiteral("dangerButton"));
    deleteButton->setCursor(Qt::PointingHandCursor);

    searchLayout->addWidget(searchPlatformComboBox);
    searchLayout->addWidget(keywordLineEdit, 1);
    searchLayout->addWidget(searchButton);
    searchLayout->addWidget(resetSearchButton);
    searchLayout->addWidget(refreshButton);
    searchLayout->addWidget(deleteButton);

    postTable = new QTableWidget();
    setupTable();

    layout->addLayout(searchLayout);
    layout->addWidget(postTable, 1);

    return card;
}

QLabel* PostManagementPage::createFieldLabel(const QString& text)
{
    auto *label = new QLabel(text);
    label->setObjectName(QStringLiteral("fieldLabel"));

    return label;
}

void PostManagementPage::addFormRow(QGridLayout *layout,
                                    int row,
                                    const QString& labelText,
                                    QWidget *field)
{
    if (!layout || !field) {
        return;
    }

    // 同一行放两组字段：左边一组，右边一组。
    const int baseColumn = (layout->itemAtPosition(row, 0) == nullptr) ? 0 : 2;

    layout->addWidget(createFieldLabel(labelText), row, baseColumn);
    layout->addWidget(field, row, baseColumn + 1);
}

void PostManagementPage::setupTable()
{
    postTable->setColumnCount(10);

    postTable->setHorizontalHeaderLabels({
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

    postTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    postTable->setSelectionMode(QAbstractItemView::SingleSelection);
    postTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    postTable->verticalHeader()->setVisible(false);
    postTable->horizontalHeader()->setStretchLastSection(true);
    postTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
}

void PostManagementPage::refreshPosts()
{
    const QString platform = searchPlatformComboBox
                                 ? searchPlatformComboBox->currentData().toString()
                                 : QString();

    const QString keyword = keywordLineEdit
                                ? keywordLineEdit->text().trimmed()
                                : QString();

    fillTable(postRepository.findPosts(platform, keyword));
}

void PostManagementPage::fillTable(const QList<Post>& posts)
{
    postTable->setRowCount(posts.size());

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

            postTable->setItem(row, column, item);
        }
    }

    setMessage(QStringLiteral("Loaded %1 post records.").arg(posts.size()));
}

void PostManagementPage::onAddPostClicked()
{
    const Post post = readPostFromForm();

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

        QMessageBox::warning(this, QStringLiteral("Add Failed"), messageLabel->text());
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

        QMessageBox::warning(this, QStringLiteral("Delete Failed"), messageLabel->text());
        return;
    }

    writePostOperationLog(
        QStringLiteral("delete_post"),
        QStringLiteral("Delete post successful. Post ID: %1").arg(postId),
        QStringLiteral("success")
        );

    refreshPosts();
    setMessage(QStringLiteral("Post deleted successfully."));
}

void PostManagementPage::onSearchClicked()
{
    refreshPosts();
}

void PostManagementPage::onResetSearchClicked()
{
    searchPlatformComboBox->setCurrentIndex(0);
    keywordLineEdit->clear();
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

        // 第一行通常是表头。这里同时支持标准帖子 CSV 和 Bilibili 趋势 CSV。
        if (csvFormat == CsvFormat::Unknown) {
            csvFormat = detectCsvFormat(fields);

            if (csvFormat != CsvFormat::Unknown) {
                continue;
            }

            // 没有表头时，默认按我们自己的 8 列帖子格式处理。
            csvFormat = CsvFormat::StandardPost;
        }

        // Bilibili 导出的第二行是“累计”，它不是某一天的数据，不能当帖子记录导入。
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

Post PostManagementPage::readPostFromForm() const
{
    Post post;

    post.platform = platformComboBox->currentText();
    post.accountName = accountLineEdit->text();
    post.content = contentLineEdit->text();
    post.publishDate = publishDateEdit->date();
    post.likes = likesSpinBox->value();
    post.comments = commentsSpinBox->value();
    post.shares = sharesSpinBox->value();
    post.views = viewsSpinBox->value();

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
    platformComboBox->setCurrentIndex(0);
    accountLineEdit->clear();
    contentLineEdit->clear();
    publishDateEdit->setDate(QDate::currentDate());
    likesSpinBox->setValue(0);
    commentsSpinBox->setValue(0);
    sharesSpinBox->setValue(0);
    viewsSpinBox->setValue(0);
    accountLineEdit->setFocus();
}

int PostManagementPage::selectedPostId() const
{
    const QList<QTableWidgetItem*> selectedItems = postTable->selectedItems();

    if (selectedItems.isEmpty()) {
        return -1;
    }

    const int row = selectedItems.first()->row();
    const QTableWidgetItem *idItem = postTable->item(row, 0);

    if (!idItem) {
        return -1;
    }

    return idItem->text().toInt();
}

void PostManagementPage::setMessage(const QString& message,
                                    bool error)
{
    messageLabel->setText(message);

    messageLabel->setStyleSheet(
        error
            ? QStringLiteral("color: #DC2626;")
            : QStringLiteral("color: #374151;")
        );
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

    // 有些 CSV 第一列前面会带 UTF-8 BOM，不去掉会影响表头识别。
    if (!field.isEmpty() && field.at(0).unicode() == 0xFEFF) {
        field.remove(0, 1);
    }

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
    // 这里比普通 split(',') 稍微稳一点：可以处理带英文逗号的引号字段。
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

    // 如果以后 CSV 中出现“2026/05/01 12:00:00”，这里只取日期部分。
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

    if (joinedHeader.contains(QStringLiteral("platform"))
        && joinedHeader.contains(QStringLiteral("account"))
        && joinedHeader.contains(QStringLiteral("views"))) {
        return CsvFormat::StandardPost;
    }

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
        post.views = csvNumber(fields.at(1));       // 播放量
        post.likes = csvNumber(fields.at(4));       // 点赞
        post.comments = csvNumber(fields.at(5))     // 弹幕
                        + csvNumber(fields.at(6));  // 评论
        post.shares = csvNumber(fields.at(7));      // 分享

        return validatePostInput(post, message);
    }

    message = QStringLiteral("Unsupported CSV format.");
    return false;
}
