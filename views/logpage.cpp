#include "logpage.h"
#include "ui_logpage.h"

#include "../services/appstyle.h"

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
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTime>

LogPage::LogPage(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::LogPage)
{
    /*
     * setupUi() 会读取 forms/logpage.ui，
     * 自动创建标题、筛选卡片、日期控件、按钮、表格和提示 Label。
     */
    ui->setupUi(this);

    prepareUiObjects();
    setupTable();
    connectSignals();
    applyStyleSheet();

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
     * LogService 查询时使用 action 字段，例如 login、create_user。
     */
    ui->actionComboBox->clear();
    ui->actionComboBox->addItem(QStringLiteral("All Actions"), QString());
    ui->actionComboBox->addItem(QStringLiteral("Login"), QStringLiteral("login"));
    ui->actionComboBox->addItem(QStringLiteral("Create User"), QStringLiteral("create_user"));
    ui->actionComboBox->addItem(QStringLiteral("Add Post"), QStringLiteral("add_post"));
    ui->actionComboBox->addItem(QStringLiteral("Update Post"), QStringLiteral("update_post"));
    ui->actionComboBox->addItem(QStringLiteral("Delete Post"), QStringLiteral("delete_post"));
    ui->actionComboBox->addItem(QStringLiteral("Import CSV"), QStringLiteral("import_csv"));
    ui->actionComboBox->addItem(QStringLiteral("Export Report"), QStringLiteral("export_report"));

    ui->dateRangeCheckBox->setChecked(false);

    ui->startDateEdit->setDate(QDate::currentDate().addDays(-7));
    ui->startDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    ui->startDateEdit->setCalendarPopup(true);
    ui->startDateEdit->setEnabled(false);

    ui->endDateEdit->setDate(QDate::currentDate());
    ui->endDateEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));
    ui->endDateEdit->setCalendarPopup(true);
    ui->endDateEdit->setEnabled(false);

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

void LogPage::refreshLogs()
{
    /*
     * 这里不直接拼 SQL。
     * 页面只负责收集筛选条件，真正查询交给 LogService。
     */
    const QList<OperationLog> logs = logService.findLogs(
        ui->usernameLineEdit->text().trimmed(),
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
    ui->usernameLineEdit->clear();
    ui->actionComboBox->setCurrentIndex(0);

    ui->dateRangeCheckBox->setChecked(false);
    ui->startDateEdit->setDate(QDate::currentDate().addDays(-7));
    ui->endDateEdit->setDate(QDate::currentDate());

    refreshLogs();
}

void LogPage::fillTable(const QList<OperationLog>& logs)
{
    ui->logTable->setRowCount(logs.size());

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

            // ID、结果、时间居中，详情列保持左对齐，阅读长文本更舒服。
            if (column == 0 || column == 1 || column == 5 || column == 6) {
                item->setTextAlignment(Qt::AlignCenter);
            }

            ui->logTable->setItem(row, column, item);
        }
    }

    setMessage(QStringLiteral("Loaded %1 operation log records.").arg(logs.size()));
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

void LogPage::setMessage(const QString& message,
                         bool error)
{
    ui->messageLabel->setText(message);
    ui->messageLabel->setStyleSheet(AppStyle::messageLabelStyle(error));
}
