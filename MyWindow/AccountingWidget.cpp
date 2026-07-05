#include "AccountingWidget.h"
#include "PageController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QDoubleValidator>
#include <QMessageBox>
#include <QLabel>
#include <QScrollArea>


// Impl类
class AccountingWidgetPrivate
{
public:
    AccountingWidgetPrivate(AccountingWidget* q) 
        : q_ptr(q) 
    {}
    ~AccountingWidgetPrivate()
    {
    }

    // 初始化整体双栏布局
    void initLayout();
    // 初始化左侧添加/编辑栏
    void initLeftForm();
    // 初始化右侧查询/操作栏
    void initRightQuery();

    // 左侧表单控件
    QWidget* leftFormWidget;
    QRadioButton* incomeRadio;    // 收入单选
    QRadioButton* expenseRadio;   // 支出单选
    QComboBox* categoryCombo;     // 收支分类
    QLineEdit* amountEdit;        // 金额
    QDateEdit* dateEdit;          // 日期
    QTextEdit* noteEdit;          // 备注
    QPushButton* saveBtn;         // 保存按钮
    QLabel* formTitleLabel;       // 表单标题


    QComboBox* fromCombo;    // 来源账户
    QComboBox* toCombo;      // 去向账户

    // 右侧查询控件
    QLineEdit* searchEdit;        // 搜索框
    QDateEdit* startDateEdit;     // 筛选开始日期
    QDateEdit* endDateEdit;       // 筛选结束日期
    QPushButton* filterBtn;       // 筛选按钮
    QPushButton* searchBtn;       // 搜索按钮
    QTableWidget* recordTable;    // 记录表格
    QPushButton* deleteBtn;       // 删除按钮
    QPushButton* editBtn;         // 修改按钮
    PageController* pageCtrl;     // 分页控件

    // 标记是否为编辑模式
    bool isEditMode;
    int editRowIndex;             // 编辑的行索引
    QString id;                         // 选中流水编号

    AccountingWidget* q_ptr;

};

// 初始化整体双栏布局
void AccountingWidgetPrivate::initLayout()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(q_ptr);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    initLeftForm();
    QScrollArea* leftScrollArea = new QScrollArea(q_ptr);
    leftScrollArea->setWidgetResizable(true);
    leftScrollArea->setWidget(leftFormWidget); // 把表单根Widget放到滚动区
    leftScrollArea->setMinimumWidth(450);
    leftScrollArea->setMaximumWidth(450);
    mainLayout->addWidget(leftScrollArea);

    // 右侧查询栏（占满剩余空间）
    initRightQuery();
    QWidget* rightWidget = new QWidget(q_ptr);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setSpacing(15);

    // 筛选搜索行
    QWidget* searchBar = new QWidget(rightWidget);
    QHBoxLayout* searchLayout = new QHBoxLayout(searchBar);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->addWidget(new QLabel("搜索："));
    searchLayout->addWidget(searchEdit);
    searchLayout->addSpacing(10);
    searchLayout->addWidget(new QLabel("日期筛选："));
    searchLayout->addWidget(startDateEdit);
    searchLayout->addWidget(new QLabel("至"));
    searchLayout->addWidget(endDateEdit);
    searchLayout->addWidget(filterBtn);
    searchLayout->addWidget(searchBtn);
    searchLayout->addStretch();
    rightLayout->addWidget(searchBar);

    // 记录表格
    rightLayout->addWidget(recordTable, 1);

    // 操作按钮行
    QWidget* btnBar = new QWidget(rightWidget);
    QHBoxLayout* btnLayout = new QHBoxLayout(btnBar);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addWidget(editBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addStretch();
    rightLayout->addWidget(btnBar);

    // 分页控件
    pageCtrl = new PageController(rightWidget);
    rightLayout->addWidget(pageCtrl);

    mainLayout->addWidget(rightWidget, 1);
}

// 初始化左侧添加/编辑栏
void AccountingWidgetPrivate::initLeftForm()
{
    // 表单容器（包含标题+所有控件）
    leftFormWidget = new QWidget(q_ptr);
    QVBoxLayout* leftFormLayout = new QVBoxLayout(leftFormWidget);
    leftFormLayout->setContentsMargins(10, 10, 10, 10);
    leftFormLayout->setSpacing(15);

    // 表单标题
    formTitleLabel = new QLabel("添加记账记录", leftFormWidget);
    formTitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");
    leftFormLayout->addWidget(formTitleLabel);

    // ========== 修复：收支类型单选按钮（之前遗漏的部分） ==========
    QWidget* typeWidget = new QWidget(leftFormWidget);
    QHBoxLayout* typeLayout = new QHBoxLayout(typeWidget);
    typeLayout->setContentsMargins(0, 0, 0, 0);
    typeLayout->setSpacing(20);

    incomeRadio = new QRadioButton("收入", typeWidget);
    expenseRadio = new QRadioButton("支出", typeWidget);
    expenseRadio->setChecked(true); // 默认选中支出

    typeLayout->addWidget(incomeRadio);
    typeLayout->addWidget(expenseRadio);
    typeLayout->addStretch(); // 右侧留白

    leftFormLayout->addWidget(new QLabel("收支类型:", leftFormWidget));
    leftFormLayout->addWidget(typeWidget);
    // ==============================================================

    // 来源账户
    QWidget* fromWidget = new QWidget(leftFormWidget);
    QHBoxLayout* Layout = new QHBoxLayout();
    Layout->setContentsMargins(0, 0, 0, 0);
    QLabel* fromLabel = new QLabel("来源:");
    fromCombo = new QComboBox(fromWidget);
    fromCombo->addItems({ "工商银行", "建设银行", "南京银行", "江阴农商银行", "交通银行", "微信", "支付宝"});
    fromCombo->setEditable(true);
    Layout->addWidget(fromLabel);
    Layout->addWidget(fromCombo);

    // 去向账户（仅转账时显示）
    QLabel* toLabel = new QLabel("去向:");
    toCombo = new QComboBox(fromWidget);
    toCombo->addItems({ "美团", "淘宝", "京东", "租房中介", "闲鱼", "抖音", "政府",  "第三方" });
    toCombo->setEditable(true);
    Layout->addWidget(toLabel);
    Layout->addWidget(toCombo);
    fromWidget->setLayout(Layout);
    leftFormLayout->addWidget(fromWidget);


    // 收支分类
    categoryCombo = new QComboBox(leftFormWidget);
    QStringList incomeCats = { "工资", "奖金", "房租补贴", "理财", "兼职", "报销", "其他收入" };
    QStringList expenseCats = { "餐饮", "奶茶", "电子产品", "交通", "购物", "房租", "娱乐", "报销", "其他支出" };
    QStringList fromCats = { "工商银行", "建设银行", "南京银行", "江阴农商银行", "交通银行", "微信", "支付宝" };
    QStringList toCats = { "美团", "淘宝", "京东", "租房中介", "闲鱼", "抖音", "政府", "第三方" };
    categoryCombo->addItems(expenseCats);
    leftFormLayout->addWidget(new QLabel("收支分类:", leftFormWidget));
    leftFormLayout->addWidget(categoryCombo);

    // 金额输入框
    amountEdit = new QLineEdit(leftFormWidget);
    amountEdit->setPlaceholderText("请输入金额（正数）");
    amountEdit->setValidator(new QDoubleValidator(0.01, 9999999.99, 2, leftFormWidget));
    leftFormLayout->addWidget(new QLabel("金额(¥):", leftFormWidget));
    leftFormLayout->addWidget(amountEdit);

    // 日期选择
    dateEdit = new QDateEdit(QDate::currentDate(), leftFormWidget);
    dateEdit->setCalendarPopup(true);
    leftFormLayout->addWidget(new QLabel("记账日期:", leftFormWidget));
    leftFormLayout->addWidget(dateEdit);

    // 备注
    noteEdit = new QTextEdit(leftFormWidget);
    noteEdit->setPlaceholderText("请输入备注（可选）");
    noteEdit->setMaximumHeight(100);
    leftFormLayout->addWidget(new QLabel("备注信息:", leftFormWidget));
    leftFormLayout->addWidget(noteEdit);

    // 保存按钮
    saveBtn = new QPushButton("添加记录", leftFormWidget);
    saveBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #27ae60;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px;
            font-size: 14px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #219653;
        }
    )");
    leftFormLayout->addWidget(saveBtn);

    // 切换分类的信号
    QObject::connect(incomeRadio, &QRadioButton::toggled, q_ptr, [=](bool checked) {
        if (checked) {
            categoryCombo->clear();
            categoryCombo->addItems(incomeCats);
            fromCombo->clear();
            fromCombo->addItems(toCats);
			toCombo->clear();
			toCombo->addItems(fromCats);
        }
        });
    QObject::connect(expenseRadio, &QRadioButton::toggled, q_ptr, [=](bool checked) {
        if (checked) {
            categoryCombo->clear();
            categoryCombo->addItems(expenseCats);
            fromCombo->clear();
            fromCombo->addItems(fromCats);
            toCombo->clear();
            toCombo->addItems(toCats);

        }
        });

    isEditMode = false;
    editRowIndex = -1;
}

// 初始化右侧查询/操作栏
void AccountingWidgetPrivate::initRightQuery()
{
    // 搜索框
    searchEdit = new QLineEdit(q_ptr);
    searchEdit->setPlaceholderText("输入备注/金额搜索...");
    searchEdit->setMinimumWidth(200);

    // 筛选日期
    startDateEdit = new QDateEdit(QDate::currentDate().addMonths(-1), q_ptr);
    startDateEdit->setCalendarPopup(true);
    endDateEdit = new QDateEdit(QDate::currentDate(), q_ptr);
    endDateEdit->setCalendarPopup(true);

    // 筛选/搜索按钮
    filterBtn = new QPushButton("筛选", q_ptr);
    searchBtn = new QPushButton("搜索", q_ptr);

    // 记录表格
    recordTable = new QTableWidget(q_ptr);
    QStringList headers = { "类型", "分类", "金额(¥)", "日期", "备注", "来源", "去向", "流水编号"};
    recordTable->setColumnCount(headers.size());
    recordTable->setHorizontalHeaderLabels(headers);
    recordTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recordTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recordTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recordTable->setMinimumHeight(400);

    // 删除/修改按钮
    deleteBtn = new QPushButton("删除选中", q_ptr);
    deleteBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #e74c3c;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 8px 15px;
        }
        QPushButton:hover {
            background-color: #c0392b;
        }
        QPushButton:disabled {
            background-color: #95a5a6;
            color: #ecf0f1;
        }
    )");
    deleteBtn->setDisabled(true);

    editBtn = new QPushButton("修改选中", q_ptr);
    editBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #f39c12;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 8px 15px;
        }
        QPushButton:hover {
            background-color: #e67e22;
        }
        QPushButton:disabled {
            background-color: #95a5a6;
            color: #ecf0f1;
        }
    )");
    editBtn->setDisabled(true);
}

// 外部类实现
AccountingWidget::AccountingWidget(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new AccountingWidgetPrivate(this))
{
    Q_D(AccountingWidget);
    d->initLayout();

    // 窗口标题（作为子窗口时由主界面控制）
    setWindowTitle("记账管理");
    // 连接信号槽
    // 保存按钮（添加/修改）
    connect(d->saveBtn, &QPushButton::clicked, this, &AccountingWidget::onSaveRecord);
    // 删除/修改按钮
    connect(d->deleteBtn, &QPushButton::clicked, this, &AccountingWidget::onDeleteRecord);
    connect(d->editBtn, &QPushButton::clicked, this, &AccountingWidget::onEditRecord);
    // 筛选/搜索
    connect(d->filterBtn, &QPushButton::clicked, this, &AccountingWidget::onFilterRecords);
    connect(d->searchBtn, &QPushButton::clicked, this, &AccountingWidget::onSearchRecords);
    // 表格选中变化
    connect(d->recordTable, &QTableWidget::itemSelectionChanged, this, &AccountingWidget::onTableSelectionChanged);
    // 分页控件
    connect(d->pageCtrl, &PageController::pageChanged, this, [this](int page) { emit pageRequested(page); });
}

AccountingWidget::~AccountingWidget() = default;

PageController* AccountingWidget::getPageController() const
{
    return d_ptr->pageCtrl;
}

void AccountingWidget::fillTable(const QList<TradeRecord>& list)
{
    d_ptr->recordTable->clearContents();
    d_ptr->recordTable->setRowCount(list.size());

    for (int row = 0; row < list.size(); ++row)
    {
        const TradeRecord& record = list[row];

        d_ptr->recordTable->setItem(row, 0, new QTableWidgetItem(record.trade_type));
        d_ptr->recordTable->setItem(row, 1, new QTableWidgetItem(record.trade_category));
        d_ptr->recordTable->setItem(row, 2, new QTableWidgetItem(QString::number(record.amount, 'f', 2)));
        d_ptr->recordTable->setItem(row, 3, new QTableWidgetItem(record.trade_time));
        d_ptr->recordTable->setItem(row, 4, new QTableWidgetItem(record.remark));
        d_ptr->recordTable->setItem(row, 5, new QTableWidgetItem(record.from));
        d_ptr->recordTable->setItem(row, 6, new QTableWidgetItem(record.to));
        d_ptr->recordTable->setItem(row, 7, new QTableWidgetItem(record.id));
    }
    d_ptr->recordTable->viewport()->update();
    d_ptr->recordTable->update();
}

void AccountingWidget::afterAddRecord(TradeRecord record)
{
    d_ptr->recordTable->setItem(0, 0, new QTableWidgetItem(record.trade_type));
    d_ptr->recordTable->setItem(0, 1, new QTableWidgetItem(record.trade_category));
    d_ptr->recordTable->setItem(0, 2, new QTableWidgetItem(QString::number(record.amount, 'f', 2)));
    d_ptr->recordTable->setItem(0, 3, new QTableWidgetItem(record.trade_time));
    d_ptr->recordTable->setItem(0, 4, new QTableWidgetItem(record.remark));
    d_ptr->recordTable->setItem(0, 5, new QTableWidgetItem(record.from));
    d_ptr->recordTable->setItem(0, 6, new QTableWidgetItem(record.to));
    d_ptr->recordTable->setItem(0, 7, new QTableWidgetItem(record.id));
    d_ptr->recordTable->viewport()->update();
    d_ptr->recordTable->update();
}


// 保存记录（添加/修改）
void AccountingWidget::onSaveRecord()
{
    Q_D(AccountingWidget);

    // 获取表单数据
    QString type = d->incomeRadio->isChecked() ? "收入" : "支出";
    QString category = d->categoryCombo->currentText();
    QString amount = d->amountEdit->text().trimmed();
    QString date = d->dateEdit->date().toString("yyyy-MM-dd");
    QString note = d->noteEdit->toPlainText().trimmed();
    QString from = d->fromCombo->currentText().trimmed();
	QString to = d->toCombo->currentText().trimmed();
    QString id = d->id;

    // 验证
    if (amount.isEmpty() || amount.toDouble() <= 0) {
        QMessageBox::warning(this, "输入错误", "请输入有效的金额（大于0）！", QMessageBox::Ok);
        d->amountEdit->setFocus();
        return;
    }

    if (d->isEditMode) {
        // 编辑模式：更新选中行
        if (d->editRowIndex < 0 || d->editRowIndex >= d->recordTable->rowCount()) {
            QMessageBox::warning(this, "编辑错误", "无效的编辑行！", QMessageBox::Ok);
            return;
        }

        d->recordTable->setItem(d->editRowIndex, 0, new QTableWidgetItem(type));
        d->recordTable->setItem(d->editRowIndex, 1, new QTableWidgetItem(category));
        d->recordTable->setItem(d->editRowIndex, 2, new QTableWidgetItem(amount));
        d->recordTable->setItem(d->editRowIndex, 3, new QTableWidgetItem(date));
        d->recordTable->setItem(d->editRowIndex, 4, new QTableWidgetItem(note));
        d->recordTable->setItem(d->editRowIndex, 5, new QTableWidgetItem(from));
        d->recordTable->setItem(d->editRowIndex, 6, new QTableWidgetItem(to));
        emit updateRecord(TradeRecord{ date, category, type, amount.toDouble(), QString(), note, QString(), from, to, id});

        // 退出编辑模式
        d->isEditMode = false;
        d->editRowIndex = -1;
        d->formTitleLabel->setText("添加记账记录");
        d->saveBtn->setText("添加记录");
        QMessageBox::information(this, "成功", "记录修改成功！", QMessageBox::Ok);
    }
    else {
        // 添加模式：新增行
        int row = d->recordTable->rowCount();
        d->recordTable->insertRow(0);
        d->recordTable->setItem(0, 0, new QTableWidgetItem(type));
        d->recordTable->setItem(0, 1, new QTableWidgetItem(category));
        d->recordTable->setItem(0, 2, new QTableWidgetItem(amount));
        d->recordTable->setItem(0, 3, new QTableWidgetItem(date));
        d->recordTable->setItem(0, 4, new QTableWidgetItem(note));
        d->recordTable->setItem(0, 5, new QTableWidgetItem(from));
        d->recordTable->setItem(0, 6, new QTableWidgetItem(to));
        d->recordTable->scrollToTop();
        emit addRecord(TradeRecord{ date, category, type, amount.toDouble(), QString(), note, QString(), from, to, QString()});

        //QMessageBox::information(this, "成功", "记录添加成功！", QMessageBox::Ok);
    }

    // 清空表单
    d->amountEdit->clear();
    d->noteEdit->clear();
    d->amountEdit->setFocus();
}

// 删除选中记录
void AccountingWidget::onDeleteRecord()
{
    Q_D(AccountingWidget);
    int currentRow = d->recordTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::information(this, "提示", "请先选中要删除的记录！", QMessageBox::Ok);
        return;
    }

    if (QMessageBox::question(this, "确认删除", "是否删除选中的记录？",
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QString id = d->recordTable->item(currentRow, 7)->text();
        emit deleteRecord(id);

        // 如果是编辑模式，退出
        if (d->isEditMode) {
            d->isEditMode = false;
            d->editRowIndex = -1;
            d->formTitleLabel->setText("添加记账记录");
            d->saveBtn->setText("添加记录");
        }
    }
}

// 修改选中记录
void AccountingWidget::onEditRecord()
{
    Q_D(AccountingWidget);
    int currentRow = d->recordTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::information(this, "提示", "请先选中要修改的记录！", QMessageBox::Ok);
        return;
    }

    // 进入编辑模式
    d->isEditMode = true;
    d->editRowIndex = currentRow;

    // 加载选中行数据到表单
    QTableWidgetItem* typeItem = d->recordTable->item(currentRow, 0);
    QTableWidgetItem* categoryItem = d->recordTable->item(currentRow, 1);
    QTableWidgetItem* amountItem = d->recordTable->item(currentRow, 2);
    QTableWidgetItem* dateItem = d->recordTable->item(currentRow, 3);
    QTableWidgetItem* noteItem = d->recordTable->item(currentRow, 4);
    QTableWidgetItem* fromItem = d->recordTable->item(currentRow, 5);
    QTableWidgetItem* toItem = d->recordTable->item(currentRow, 6);
    QTableWidgetItem* idItem = d->recordTable->item(currentRow, 7);

    if (typeItem->text() == "收入") {
        d->incomeRadio->setChecked(true);
    }
    else {
        d->expenseRadio->setChecked(true);
    }

    // 切换分类后手动设置选中项
    QString category = categoryItem->text();
    int catIndex = d->categoryCombo->findText(category);
    if (catIndex >= 0) {
        d->categoryCombo->setCurrentIndex(catIndex);
    }

    d->amountEdit->setText(amountItem->text());
    d->dateEdit->setDate(QDate::fromString(dateItem->text(), "yyyy-MM-dd"));
    d->noteEdit->setText(noteItem->text());

    // 更新表单标题和按钮
    d->formTitleLabel->setText("修改记账记录");
    d->saveBtn->setText("保存修改");
    d->fromCombo->setCurrentText(fromItem->text());
    d->toCombo->setCurrentText(toItem->text());
    d->id = idItem->text();
}

// 筛选记录
void AccountingWidget::onFilterRecords()
{
    Q_D(AccountingWidget);
    QDate start = d->startDateEdit->date();
    QDate end = d->endDateEdit->date();

    if (start > end) {
        QMessageBox::warning(this, "筛选错误", "开始日期不能晚于结束日期！", QMessageBox::Ok);
        return;
    }

    emit filterRequested(start, end);
}

// 搜索记录
void AccountingWidget::onSearchRecords()
{
    Q_D(AccountingWidget);
    QString keyword = d->searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::warning(this, "搜索错误", "请输入搜索关键词！", QMessageBox::Ok);
        d->searchEdit->setFocus();
        return;
    }

    emit searchRequested(keyword);
}

// 表格选中行变化
void AccountingWidget::onTableSelectionChanged()
{
    Q_D(AccountingWidget);
    bool hasSelection = d->recordTable->currentRow() >= 0;
    d->deleteBtn->setEnabled(hasSelection);
    d->editBtn->setEnabled(hasSelection);
}