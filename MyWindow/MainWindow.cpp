#include "MainWindow.h"
#include "AccountingWidget.h"
#include "AnalysisWidget.h"

#include <DataCenter/DataCenter.h>

#include <map>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QStackedWidget>
#include <QMessageBox>
#include <QSizePolicy>
#include <QLocale>

// Impl类
class MainWidgetPrivate
{
public:
    MainWidgetPrivate(MainWidget* q) : q_ptr(q) {}

    // 初始化整体布局
    void initLayout();
    // 初始化左侧导航栏
    void initLeftNav();
    // 初始化右侧内容区
    void initRightContent();
    // 创建收支汇总卡片
    QWidget* createSummaryCard(const QString& title,
        const QString& incomeText,
        const QString& expenseText,
        const QString& cardColor);

    // 控件声明
    // 左侧导航
    QWidget* leftNavWidget;
    QPushButton* homeBtn;
    QPushButton* accountingBtn;
    QPushButton* settingsBtn;
    QPushButton* analysisBtn;

    // 右侧内容
    QStackedWidget* stackedContent; // 内容切换容器
    QWidget* homeWidget;           // 首页（收支卡片）
    AccountingWidget* accountingWidget; // 记账管理界面
    QWidget* settingsWidget;       // 设置界面
    AnalysisWidget* analysisWidget; // 收支分析界面

    // 收支卡片标签
    std::map<QString, QLabel*> incomeLabelMap;
    std::map<QString, QLabel*> expenseLabelMap;
    std::map<QString, QLabel*> profitLabelMap;
    std::map<QString, QLabel*> titleLabelMap;

    MainWidget* q_ptr;

    QString toLocaleString(double amount);

public:
    DataCenter dataCenter;
};

// 初始化整体布局
void MainWidgetPrivate::initLayout()
{
    QHBoxLayout* mainLayout = new QHBoxLayout(q_ptr);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 左侧导航栏（固定宽度200）
    initLeftNav();
    leftNavWidget->setMinimumWidth(200);
    leftNavWidget->setMaximumWidth(200);
    leftNavWidget->setStyleSheet("background-color: #2c3e50;");
    mainLayout->addWidget(leftNavWidget);

    // 右侧内容区（占满剩余空间）
    initRightContent();
    mainLayout->addWidget(stackedContent, 1);
}

// 初始化左侧导航栏
void MainWidgetPrivate::initLeftNav()
{
    leftNavWidget = new QWidget(q_ptr);
    QVBoxLayout* navLayout = new QVBoxLayout(leftNavWidget);
    navLayout->setContentsMargins(10, 30, 10, 10);
    navLayout->setSpacing(15);

    // 导航标题
    QLabel* titleLabel = new QLabel("记账系统", leftNavWidget);
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    navLayout->addWidget(titleLabel);
    navLayout->addSpacing(20);
    // 新增：返回主界面按钮（默认选中）
    homeBtn = new QPushButton("主界面", leftNavWidget);
    homeBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 12px;
            font-size: 14px;
            text-align: left;
            padding-left: 20px;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
        QPushButton:pressed {
            background-color: #1f618d;
        }
    )");
    homeBtn->setMinimumHeight(40);
    navLayout->addWidget(homeBtn);

    // 记账管理按钮
    accountingBtn = new QPushButton("记账管理", leftNavWidget);
    accountingBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 12px;
            font-size: 14px;
            text-align: left;
            padding-left: 20px;
        }
        QPushButton:hover {
            background-color: #2980b9;
        }
        QPushButton:pressed {
            background-color: #1f618d;
        }
    )");
    accountingBtn->setMinimumHeight(40);
    navLayout->addWidget(accountingBtn);

    // 收支分析按钮
    analysisBtn = new QPushButton("收支分析", leftNavWidget);
    analysisBtn->setStyleSheet(accountingBtn->styleSheet()); // 用之前定义的默认样式
    analysisBtn->setMinimumHeight(40);
    navLayout->addWidget(analysisBtn);

    // 系统设置按钮
    settingsBtn = new QPushButton("系统设置", leftNavWidget);
    settingsBtn->setStyleSheet(accountingBtn->styleSheet());
    settingsBtn->setMinimumHeight(40);
    navLayout->addWidget(settingsBtn);

    // 底部拉伸
    navLayout->addStretch();

}

// 初始化右侧内容区
void MainWidgetPrivate::initRightContent()
{
    stackedContent = new QStackedWidget(q_ptr);
    stackedContent->setStyleSheet("background-color: #ecf0f1;");

    // 1. 首页（收支卡片）
    homeWidget = new QWidget(stackedContent);
    QVBoxLayout* homeLayout = new QVBoxLayout(homeWidget);
    homeLayout->setContentsMargins(20, 20, 20, 20);
    homeLayout->setSpacing(15);

    // 页面标题
    QLabel* pageTitle = new QLabel("收支汇总", homeWidget);
    pageTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #2c3e50;");
    homeLayout->addWidget(pageTitle);

    // 总收支卡片
    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->setContentsMargins(0, 0, 0, 0);
    QWidget* totalCard = createSummaryCard("总收支", "¥0.00", "¥0.00", "#3498db");
    leftLayout->addWidget(totalCard);

    // 年收支卡片
    QWidget* yearCard = createSummaryCard("年收支", "¥0.00", "¥0.00", "#9b59b6");
    leftLayout->addWidget(yearCard);

    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->setContentsMargins(0, 0, 0, 0);
    // 月收支卡片
    QWidget* monthCard = createSummaryCard("月收支", "¥0.00", "¥0.00", "#1abc9c");
    rightLayout->addWidget(monthCard);

    // 今日收支卡片
    QWidget* todayCard = createSummaryCard("今日收支", "¥0.00", "¥0.00", "#2ecc71");
    rightLayout->addWidget(todayCard);

    QHBoxLayout* cardLayout = new QHBoxLayout;
    cardLayout->addLayout(leftLayout);
    cardLayout->addLayout(rightLayout);
    cardLayout->setContentsMargins(0, 0, 0, 0);

    homeLayout->addLayout(cardLayout);

    // 底部拉伸
    homeLayout->addStretch();

    // 2. 记账管理界面
    accountingWidget = new AccountingWidget(stackedContent);

    // 3. 设置界面（示例）
    settingsWidget = new QWidget(stackedContent);
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsWidget);
    settingsLayout->setAlignment(Qt::AlignCenter);
    QLabel* settingsLabel = new QLabel("系统设置界面（待实现）", settingsWidget);
    settingsLabel->setStyleSheet("font-size: 18px; color: #7f8c8d;");
    settingsLayout->addWidget(settingsLabel);

    // 4. 收支分析界面
    analysisWidget = new AnalysisWidget(stackedContent);

    // 添加到堆叠窗口
    stackedContent->addWidget(homeWidget);
    stackedContent->addWidget(accountingWidget);
    stackedContent->addWidget(settingsWidget);
    stackedContent->addWidget(analysisWidget);

    // 默认显示首页
    stackedContent->setCurrentWidget(homeWidget);
}

// 创建收支汇总卡片
QWidget* MainWidgetPrivate::createSummaryCard(const QString& title,
    const QString& incomeText,
    const QString& expenseText,
    const QString& cardColor)
{
    QWidget* card = new QWidget(q_ptr);
    card->setStyleSheet(QString(R"(
        QWidget {
            background-color: %1;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
        }
        QLabel {
            color: white;
        }
    )").arg(cardColor));
    card->setMinimumHeight(150);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(10, 10, 10, 10);
    cardLayout->setSpacing(15);

    // 卡片标题
    QLabel* titleLabel = new QLabel(title, card);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: 500;");
    titleLabelMap[title] = titleLabel;
    cardLayout->addWidget(titleLabel);

    // 收支数值行
    QHBoxLayout* valueLayout = new QHBoxLayout();
    valueLayout->setSpacing(5);
    valueLayout->setContentsMargins(0, 0, 0, 0);

    // 收入
    QWidget* incomeWidget = new QWidget(card);
    QVBoxLayout* incomeLayout = new QVBoxLayout(incomeWidget);
    QLabel* incomeTitle = new QLabel("收入", incomeWidget);
    incomeTitle->setStyleSheet("font-size: 14px; opacity: 0.8;");
    QLabel* incomeValue = new QLabel(incomeText, incomeWidget);
    incomeValue->setStyleSheet("font-size: 24px; font-weight: bold;");
    incomeLabelMap[title] = incomeValue;
    incomeLayout->addWidget(incomeTitle);
    incomeLayout->addWidget(incomeValue);
    valueLayout->addWidget(incomeWidget);

    // 支出
    QWidget* expenseWidget = new QWidget(card);
    QVBoxLayout* expenseLayout = new QVBoxLayout(expenseWidget);
    QLabel* expenseTitle = new QLabel("支出", expenseWidget);
    expenseTitle->setStyleSheet("font-size: 14px; opacity: 0.8;");
    QLabel* expenseValue = new QLabel(expenseText, expenseWidget);
    expenseValue->setStyleSheet("font-size: 24px; font-weight: bold;");
    expenseLabelMap[title] = expenseValue;
    expenseLayout->addWidget(expenseTitle);
    expenseLayout->addWidget(expenseValue);
    valueLayout->addWidget(expenseWidget);

    // 盈余
    QWidget* profitWidget = new QWidget(card);
    QVBoxLayout* profitWidgetLayout = new QVBoxLayout(profitWidget);
    QLabel* profitTitle = new QLabel("盈余", profitWidget);
    profitTitle->setStyleSheet("font-size: 14px; opacity: 0.8;");
    QLabel* profitValue = new QLabel(profitWidget);
    profitValue->setStyleSheet("font-size: 24px; font-weight: bold;");
    profitLabelMap[title] = profitValue;
    profitWidgetLayout->addWidget(profitTitle);
    profitWidgetLayout->addWidget(profitValue);
    valueLayout->addWidget(profitWidget);


    valueLayout->addStretch();
    cardLayout->addLayout(valueLayout);

    return card;
}

QString MainWidgetPrivate::toLocaleString(double amount)
{
    QLocale locale(QLocale::Chinese);

    return locale.toCurrencyString(amount);
}

// 外部类实现
MainWidget::MainWidget(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new MainWidgetPrivate(this))
{
    Q_D(MainWidget);
    d->initLayout();

    // 窗口属性
    setWindowTitle("简易记账系统");
    setMinimumSize(800, 600);

    // 连接信号槽

    connect(d->homeBtn, &QPushButton::clicked, this, [=]() {
        onSwitchContent("home");
        });
    connect(d->accountingBtn, &QPushButton::clicked, this, [=]() {
        onSwitchContent("accounting");
        });
    connect(d->analysisBtn, &QPushButton::clicked, this, [=]() {
        onSwitchContent("analysis");
        });
    connect(d->settingsBtn, &QPushButton::clicked, this, [=]() {
        onSwitchContent("settings");
        });

    d_ptr->dataCenter.initTables();
    connect(d->accountingWidget, &AccountingWidget::addRecord, [this](TradeRecord record)
        {
            qDebug() << "添加记录";
            d_ptr->dataCenter.addRecord(record);        // 添加记录
            auto res = d_ptr->dataCenter.getNewRecord();// 查询结果
            d_ptr->accountingWidget->afterAddRecord(res);// 更新界面
        });

    connect(d->accountingWidget, &AccountingWidget::updateRecord, [this](TradeRecord record)
        {
            qDebug() << "修改记录";
            d_ptr->dataCenter.updateRecord(record);
        });

    auto list = d_ptr->dataCenter.getAllRecords();
    d->accountingWidget->fillTable(list);

    // 初始化卡片数据
    updateBalanceCards();
    update();
}

MainWidget::~MainWidget() = default;

// 切换右侧内容区
void MainWidget::onSwitchContent(const QString& type)
{
    Q_D(MainWidget);
    if (type == "accounting") {
        d->stackedContent->setCurrentWidget(d->accountingWidget);
    }
    else if (type == "settings") {
        d->stackedContent->setCurrentWidget(d->settingsWidget);
    }
    else if (type == "analysis")
    {
        d->stackedContent->setCurrentWidget(d->analysisWidget);
    }
    else {
        d->stackedContent->setCurrentWidget(d->homeWidget);
    }
    updateBalanceCards();// 更新统计
}

// 更新收支卡片数据
void MainWidget::updateBalanceCards()
{
    Q_D(MainWidget);
    // 模拟数据


    QDate today = QDate::currentDate();
    Statistics stats = d->dataCenter.getStatistics(
        TimeRange::Day,
        today.year(),
        today.month(),
        today.day()
    );

    double totalIncome = d->dataCenter.getIncome(TimeRange::Total);
	double totalExpense = d->dataCenter.getExpense(TimeRange::Total); 
    double totalProfit = d->dataCenter.getProfit(TimeRange::Total);
    double yearIncome = d->dataCenter.getIncome(TimeRange::Year, today.year());
    double yearExpense = d->dataCenter.getExpense(TimeRange::Year, today.year());
    double yearProfit = d->dataCenter.getProfit(TimeRange::Year, today.year());
    double monthIncome = d->dataCenter.getIncome(TimeRange::Month, today.year(), today.month());
    double monthExpense = d->dataCenter.getExpense(TimeRange::Month, today.year(), today.month());
    double monthProfit = d->dataCenter.getProfit(TimeRange::Month, today.year(), today.month());
    double dayIncome = d->dataCenter.getIncome(TimeRange::Day, today.year(), today.month(), today.day());
    double dayExpense = d->dataCenter.getExpense(TimeRange::Day, today.year(), today.month(), today.day());
    double dayProfit = d->dataCenter.getProfit(TimeRange::Day, today.year(), today.month(), today.day());
    
    d->incomeLabelMap["总收支"]->setText(d->toLocaleString(totalIncome));
    d->expenseLabelMap["总收支"]->setText(d->toLocaleString(totalExpense));
    d->profitLabelMap["总收支"]->setText(d->toLocaleString(totalProfit));

    d->incomeLabelMap["年收支"]->setText(d->toLocaleString(yearIncome));
    d->expenseLabelMap["年收支"]->setText(d->toLocaleString(yearExpense));
    d->profitLabelMap["年收支"]->setText(d->toLocaleString(yearProfit));

    d->incomeLabelMap["月收支"]->setText(d->toLocaleString(monthIncome));
    d->expenseLabelMap["月收支"]->setText(d->toLocaleString(monthExpense));
    d->profitLabelMap["月收支"]->setText(d->toLocaleString(monthProfit));
     
    d->incomeLabelMap["今日收支"]->setText(d->toLocaleString(stats.income));
    d->expenseLabelMap["今日收支"]->setText(d->toLocaleString(stats.expense));
    d->profitLabelMap["今日收支"]->setText(d->toLocaleString(stats.profit));

    d->titleLabelMap["年收支"]->setText(QString::number(today.year()) + "年收支");
    d->titleLabelMap["月收支"]->setText(QString::number(today.month()) + "月收支");
    d->titleLabelMap["今日收支"]->setText(QString::number(today.month()) + "月" + QString::number(today.day()) + "日收支");
}