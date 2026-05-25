#include "logpage.h"
#include "../services/appstyle.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

LogPage::LogPage(QWidget *parent)
    : QWidget(parent),
    messageLabel(nullptr),
    usernameLineEdit(nullptr),
    actionComboBox(nullptr),
    dateRangeCheckBox(nullptr),
    startDateEdit(nullptr),
    endDateEdit(nullptr),
    searchButton(nullptr),
    resetButton(nullptr),
    refreshButton(nullptr),
    logTable(nullptr)
{
    buildUi();
    refreshLogs();
}

void LogPage::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 22, 24, 24);
    rootLayout->setSpacing(16);

    auto *titleLabel = new QLabel(QStringLiteral("Operation Logs"));
    titleLabel->setObjectName(QStringLiteral("pageTitle"));

    auto *subtitleLabel = new QLabel(
        QStringLiteral("View user login, user management, post operations and CSV import records.")
        );
    subtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));

    messageLabel = new QLabel();
    messageLabel->setObjectName(QStringLiteral("messageLabel"));

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(subtitleLabel);
    rootLayout->addWidget(createFilterCard());
    rootLayout->addWidget(createTableCard(), 1);
    rootLayout->addWidget(messageLabel);

    applyStyleSheet();

    connect(searchButton, &QPushButton::clicked,
            this, &LogPage::onSearchClicked);

    connect(resetButton, &QPushButton::clicked,
            this, &LogPage::onResetClicked);

    connect(refreshButton, &QPushButton::clicked,
            this, &LogPage::refreshLogs);
}

void LogPage::applyStyleSheet()
{
    setStyleSheet(AppStyle::dataManagementPageStyle());
}

QFrame* LogPage::createFilterCard()
{
    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));

    auto *layout = new QHBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(10);

    auto *usernameLabel = new QLabel(QStringLiteral("Username"));
    usernameLabel->setObjectName(QStringLiteral("fieldLabel"));

    usernameLineEdit = new QLineEdit();
    usernameLineEdit->setPlaceholderText(QStringLiteral("Search username"));

    auto *actionLabel = new QLabel(QStringLiteral("Action"));
    actionLabel->setObjectName(QStringLiteral("fieldLabel"));

    actionComboBox = new QComboBox();
    actionComboBox->addItem(QStringLiteral("All Actions"), QString());
    actionComboBox->addItem(QStringLiteral("Login"), QStringLiteral("login"));
    actionComboBox->addItem(QStringLiteral("Create User"), QStringLiteral("create_user"));
    actionComboBox->addItem(QStringLiteral("Add Post"), QStringLiteral("add_post"));
    actionComboBox->addItem(QStringLiteral("Delete Post"), QStringLiteral("delete_post"));
    actionComboBox->addItem(QStringLiteral("Import CSV"), QStringLiteral("import_csv"));
    actionComboBox->addItem(QStringLiteral("Update Post"), QStringLiteral("update_post"));
    actionComboBox->addItem(QStringLiteral("Export Report"), QStringLiteral("export_report"));

    dateRangeCheckBox = new QCheckBox(QStringLiteral("Date Range"));

    startDateEdit = new QDateEdit(QDate::currentDate().addDays(-7));
    startDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    startDateEdit->setCalendarPopup(true);

    endDateEdit = new QDateEdit(QDate::currentDate());
    endDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    endDateEdit->setCalendarPopup(true);

    searchButton = new QPushButton(QStringLiteral("Search"));
    searchButton->setObjectName(QStringLiteral("primaryButton"));
    searchButton->setCursor(Qt::PointingHandCursor);

    resetButton = new QPushButton(QStringLiteral("Reset"));
    resetButton->setObjectName(QStringLiteral("secondaryButton"));
    resetButton->setCursor(Qt::PointingHandCursor);

    refreshButton = new QPushButton(QStringLiteral("Refresh"));
    refreshButton->setObjectName(QStringLiteral("secondaryButton"));
    refreshButton->setCursor(Qt::PointingHandCursor);

    layout->addWidget(usernameLabel);
    layout->addWidget(usernameLineEdit, 1);
    layout->addWidget(actionLabel);
    layout->addWidget(actionComboBox);
    layout->addWidget(dateRangeCheckBox);
    layout->addWidget(startDateEdit);
    layout->addWidget(endDateEdit);
    layout->addWidget(searchButton);
    layout->addWidget(resetButton);
    layout->addWidget(refreshButton);

    return card;
}

QFrame* LogPage::createTableCard()
{
    auto *card = new QFrame();
    card->setObjectName(QStringLiteral("card"));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 18, 20, 18);

    logTable = new QTableWidget();
    setupTable();

    layout->addWidget(logTable);

    return card;
}

void LogPage::setupTable()
{
    logTable->setColumnCount(7);

    logTable->setHorizontalHeaderLabels({
        QStringLiteral("Log ID"),
        QStringLiteral("User ID"),
        QStringLiteral("Username"),
        QStringLiteral("Action"),
        QStringLiteral("Detail"),
        QStringLiteral("Result"),
        QStringLiteral("Time")
    });

    logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    logTable->verticalHeader()->setVisible(false);

    logTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    logTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    logTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    logTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    logTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    logTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    logTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
}

void LogPage::refreshLogs()
{
    const QList<OperationLog> logs = logService.findLogs(
        usernameLineEdit ? usernameLineEdit->text().trimmed() : QString(),
        selectedAction(),
        startDateTime(),
        endDateTime(),
        300
        );

    fillTable(logs);
}

void LogPage::onSearchClicked()
{
    refreshLogs();
}

void LogPage::onResetClicked()
{
    usernameLineEdit->clear();
    actionComboBox->setCurrentIndex(0);
    dateRangeCheckBox->setChecked(false);
    startDateEdit->setDate(QDate::currentDate().addDays(-7));
    endDateEdit->setDate(QDate::currentDate());

    refreshLogs();
}

void LogPage::fillTable(const QList<OperationLog>& logs)
{
    logTable->setRowCount(logs.size());

    for (int row = 0; row < logs.size(); ++row) {
        const OperationLog log = logs.at(row);

        const QString userIdText = log.userId > 0
                                       ? QString::number(log.userId)
                                       : QStringLiteral("-");

        const QStringList values = {
            QString::number(log.logId),
            userIdText,
            log.username,
            log.action,
            log.detail,
            log.result,
            log.createdAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss"))
        };

        for (int column = 0; column < values.size(); ++column) {
            auto *item = new QTableWidgetItem(values.at(column));

            if (column == 0 || column == 1 || column == 5 || column == 6) {
                item->setTextAlignment(Qt::AlignCenter);
            }

            logTable->setItem(row, column, item);
        }
    }

    setMessage(QStringLiteral("Loaded %1 operation log records.").arg(logs.size()));
}

QString LogPage::selectedAction() const
{
    if (!actionComboBox) {
        return QString();
    }

    return actionComboBox->currentData().toString();
}

QDateTime LogPage::startDateTime() const
{
    if (!dateRangeCheckBox || !dateRangeCheckBox->isChecked()) {
        return QDateTime();
    }

    return QDateTime(startDateEdit->date(), QTime(0, 0, 0));
}

QDateTime LogPage::endDateTime() const
{
    if (!dateRangeCheckBox || !dateRangeCheckBox->isChecked()) {
        return QDateTime();
    }

    return QDateTime(endDateEdit->date(), QTime(23, 59, 59));
}

void LogPage::setMessage(const QString& message,
                         bool error)
{
    messageLabel->setText(message);
    messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}