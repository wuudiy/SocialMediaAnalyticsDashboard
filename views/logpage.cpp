#include "logpage.h"
#include "ui_logpage.h"

#include "../controllers/logcontroller.h"
#include "../styles/appstyle.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDateTime>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTime>

LogPage::LogPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::LogPage),
    logController(nullptr)
{
    /*
     * setupUi() 会读取 forms/LogPage.ui，
     * 自动创建标题、筛选卡片、日期控件、按钮、表格和提示 Label。
     */
    ui->setupUi(this);

    prepareUiObjects();
    setupTable();
    connectSignals();
    applyStyleSheet();

    /*
     * 当前批次为了减少 MainWindow 改动，
     * Controller 先由页面内部创建。
     *
     * 后续如果重构 MainWindowController，
     * 可以改成由上层统一创建并注入。
     */
    logController = new LogController(this, this);

    refreshLogs();
}

LogPage::~LogPage()
{
    delete ui;
}

/*
 * 初始化 .ui 中已有控件的运行时属性。
 *
 * .ui 只负责“界面长什么样”，
 * 这里负责“控件默认值、下拉框数据、按钮样式名、输入提示”。
 */
void LogPage::prepareUiObjects()
{
    setObjectName(QStringLiteral("logPage"));

    ui->pageTitleLabel->setObjectName(QStringLiteral("pageTitle"));
    ui->pageSubtitleLabel->setObjectName(QStringLiteral("pageSubtitle"));

    ui->filterCard->setObjectName(QStringLiteral("card"));
    ui->tableCard->setObjectName(QStringLiteral("card"));

    ui->usernameLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->actionLabel->setObjectName(QStringLiteral("fieldLabel"));
    ui->messageLabel->setObjectName(QStringLiteral("messageLabel"));
    ui->messageLabel->setWordWrap(true);

    // 让输入框列可以自动拉伸，避免窗口变宽后控件仍然挤在一起。
    ui->filterLayout->setColumnStretch(1, 1);
    ui->filterLayout->setColumnStretch(3, 1);

    ui->usernameLineEdit->setPlaceholderText(QStringLiteral("Search username"));

    /*
     * 下拉框显示文本给用户看，currentData() 存真实查询值。
     *
     * 注意：
     * 当前项目里旧日志和新日志 action 可能存在命名差异。
     * 后续最终清理时建议统一 action 常量。
     */
    ui->actionComboBox->clear();
    ui->actionComboBox->addItem(QStringLiteral("All Actions"), QString());
    ui->actionComboBox->addItem(QStringLiteral("Login"), QStringLiteral("login"));
    ui->actionComboBox->addItem(QStringLiteral("Create User"), QStringLiteral("create_user"));
    ui->actionComboBox->addItem(QStringLiteral("Enable User"), QStringLiteral("enable_user"));
    ui->actionComboBox->addItem(QStringLiteral("Disable User"), QStringLiteral("disable_user"));
    ui->actionComboBox->addItem(QStringLiteral("Reset Password"), QStringLiteral("reset_password"));
    ui->actionComboBox->addItem(QStringLiteral("Add Post"), QStringLiteral("add_post"));
    ui->actionComboBox->addItem(QStringLiteral("Update Post"), QStringLiteral("update_post"));
    ui->actionComboBox->addItem(QStringLiteral("Delete Post"), QStringLiteral("delete_post"));
    ui->actionComboBox->addItem(QStringLiteral("Import CSV"), QStringLiteral("import_csv"));
    ui->actionComboBox->addItem(QStringLiteral("Export Report"), QStringLiteral("Export Report"));

    ui->searchButton->setObjectName(QStringLiteral("primaryButton"));
    ui->resetButton->setObjectName(QStringLiteral("secondaryButton"));
    ui->refreshButton->setObjectName(QStringLiteral("secondaryButton"));

    const QList<QPushButton*> buttons = {
        ui->searchButton,
        ui->resetButton,
        ui->refreshButton
    };

    for (QPushButton *button : buttons) {
        if (button) {
            button->setCursor(Qt::PointingHandCursor);
        }
    }

    resetFilterControls();
}

/*
 * 集中连接信号槽。
 *
 * 好处：
 * - 页面有哪些交互一眼能看到；
 * - .ui 文件不承担业务逻辑；
 * - 后续改按钮位置不影响功能代码。
 */
void LogPage::connectSignals()
{
    connect(ui->searchButton, &QPushButton::clicked,
            this, &LogPage::onSearchClicked);

    connect(ui->resetButton, &QPushButton::clicked,
            this, &LogPage::onResetClicked);

    connect(ui->refreshButton, &QPushButton::clicked,
            this, &LogPage::refreshLogs);

    /*
     * 勾选 Date Range 时才允许编辑开始日期和结束日期。
     * 未勾选时，查询不限制日期。
     */
    connect(ui->dateRangeCheckBox, &QCheckBox::toggled,
            this, [this](bool checked) {
                ui->startDateEdit->setEnabled(checked);
                ui->endDateEdit->setEnabled(checked);
            });
}

void LogPage::applyStyleSheet()
{
    setStyleSheet(AppStyle::dataManagementPageStyle());
}

void LogPage::setupTable()
{
    ui->logTable->setColumnCount(7);

    ui->logTable->setHorizontalHeaderLabels({
        QStringLiteral("Log ID"),
        QStringLiteral("User ID"),
        QStringLiteral("Username"),
        QStringLiteral("Action"),
        QStringLiteral("Detail"),
        QStringLiteral("Result"),
        QStringLiteral("Time")
    });

    ui->logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->logTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->logTable->setAlternatingRowColors(true);
    ui->logTable->verticalHeader()->setVisible(false);

    ui->logTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->logTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->logTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->logTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->logTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    ui->logTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->logTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
}

/*
 * 刷新日志。
 *
 * View 不再直接调用 LogService，
 * 只负责把当前筛选条件发给 Controller。
 */
void LogPage::refreshLogs()
{
    emit searchLogsRequested(readFilterFromUi());
}

void LogPage::onSearchClicked()
{
    refreshLogs();
}

void LogPage::onResetClicked()
{
    resetFilterControls();
    refreshLogs();
}

void LogPage::showLogs(const QList<OperationLog>& logs)
{
    fillTable(logs);
    showMessage(QStringLiteral("Loaded %1 operation log records.").arg(logs.size()));
}

void LogPage::fillTable(const QList<OperationLog>& logs)
{
    ui->logTable->clearContents();
    ui->logTable->setRowCount(logs.size());

    for (int row = 0; row < logs.size(); ++row) {
        const OperationLog log = logs.at(row);

        const QString userIdText = log.userId > 0
                                       ? QString::number(log.userId)
                                       : QStringLiteral("-");

        ui->logTable->setItem(row, 0, createTableItem(QString::number(log.logId)));
        ui->logTable->setItem(row, 1, createTableItem(userIdText));
        ui->logTable->setItem(row, 2, createTableItem(log.username, Qt::AlignLeft | Qt::AlignVCenter));
        ui->logTable->setItem(row, 3, createTableItem(log.action, Qt::AlignLeft | Qt::AlignVCenter));
        ui->logTable->setItem(row, 4, createTableItem(log.detail, Qt::AlignLeft | Qt::AlignVCenter));
        ui->logTable->setItem(row, 5, createTableItem(log.result));
        ui->logTable->setItem(
            row,
            6,
            createTableItem(log.createdAt.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
            );
    }
}

OperationLogFilter LogPage::readFilterFromUi() const
{
    OperationLogFilter filter;

    filter.usernameKeyword = ui->usernameLineEdit->text().trimmed();
    filter.action = selectedAction();
    filter.startTime = startDateTime();
    filter.endTime = endDateTime();
    filter.limit = 300;

    return filter;
}

void LogPage::resetFilterControls()
{
    ui->usernameLineEdit->clear();
    ui->actionComboBox->setCurrentIndex(0);

    ui->dateRangeCheckBox->setChecked(false);

    ui->startDateEdit->setDate(QDate::currentDate().addDays(-7));
    ui->startDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    ui->startDateEdit->setCalendarPopup(true);
    ui->startDateEdit->setEnabled(false);

    ui->endDateEdit->setDate(QDate::currentDate());
    ui->endDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    ui->endDateEdit->setCalendarPopup(true);
    ui->endDateEdit->setEnabled(false);
}

QString LogPage::selectedAction() const
{
    return ui->actionComboBox->currentData().toString();
}

QDateTime LogPage::startDateTime() const
{
    if (!ui->dateRangeCheckBox->isChecked()) {
        return QDateTime();
    }

    return QDateTime(ui->startDateEdit->date(), QTime(0, 0, 0));
}

QDateTime LogPage::endDateTime() const
{
    if (!ui->dateRangeCheckBox->isChecked()) {
        return QDateTime();
    }

    return QDateTime(ui->endDateEdit->date(), QTime(23, 59, 59));
}

QTableWidgetItem *LogPage::createTableItem(const QString& text,
                                           Qt::Alignment alignment) const
{
    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setTextAlignment(alignment);
    return item;
}

void LogPage::showMessage(const QString& message,
                          bool error)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}

void LogPage::showWarningMessage(const QString& title,
                                 const QString& message)
{
    QMessageBox::warning(this, title, message);
}
