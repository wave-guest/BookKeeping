#include "MainWindow.h"
#include "AccountingWidget.h"
#include "AnalysisWidget.h"
#include "PageController.h"

#include <DataCenter/DataCenter.h>

#include <map>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QLocale>

// Impl类
class MainWidgetPrivate
{
public:
    MainWidgetPrivate(const QString& dbPath, MainWidget* q) : q_ptr(q), dataCenter(dbPath) {}

    // 初始化整体布局
    void initLayout();
    // 初始化左侧导航栏
    void initLeftNav();
    // 初始化右侧内容区
    void initRightContent();
    // 创建收支汇总卡片
    QWidget* createSummaryCard(const QString& role,
        const QString& title,
        const QString& incomeText,
        const QString& expenseText);

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

    // 左侧导航栏（固定宽度180）
    initLeftNav();
    leftNavWidget->setObjectName("leftNavWidget");
    leftNavWidget->setMinimumWidth(180);
    leftNavWidget->setMaximumWidth(180);
    mainLayout->addWidget(leftNavWidget);

    // 右侧内容区（占满剩余空间）
    initRightContent();
    mainLayout->addWidget(stackedContent, 1);
}

// 初始化左侧导航栏
void MainWidgetPrivate::initLeftNav()
{
    leftNavWidget = new QWidget(q_ptr);
    leftNavWidget->setObjectName("leftNavWidget");
    QVBoxLayout* navLayout = new QVBoxLayout(leftNavWidget);
    navLayout->setContentsMargins(10, 30, 10, 10);
    navLayout->setSpacing(8);

    // 导航标题
    QLabel* titleLabel = new QLabel(QStringLiteral("\U0001f4b0 \u8bb0\u8d26\u7cfb\u7edf"), leftNavWidget);
    titleLabel->setObjectName("navTitle");
    titleLabel->setAlignment(Qt::AlignCenter);
    navLayout->addWidget(titleLabel);
    navLayout->addSpacing(10);

    // 导航按钮
    auto createNavBtn = [&](const QString& objName, const QString& text) -> QPushButton* {
        QPushButton* btn = new QPushButton(text, leftNavWidget);
        btn->setObjectName(objName);
        btn->setCheckable(true);
        btn->setMinimumHeight(40);
        navLayout->addWidget(btn);
        return btn;
    };

    homeBtn = createNavBtn("homeBtn", QStringLiteral("\U0001f3e0 \u4e3b\u754c\u9762"));
    accountingBtn = createNavBtn("accountingBtn", QStringLiteral("\U0001f4cb \u8bb0\u8d26\u7ba1\u7406"));
    analysisBtn = createNavBtn("analysisBtn", QStringLiteral("\U0001f4ca \u6536\u652f\u5206\u6790"));
    settingsBtn = createNavBtn("settingsBtn", QStringLiteral("\u2699\ufe0f \u7cfb\u7edf\u8bbe\u7f6e"));

    homeBtn->setChecked(true);

    // 底部版本号
    navLayout->addStretch();
    QLabel* versionLabel = new QLabel("v1.0", leftNavWidget);
    versionLabel->setObjectName("versionLabel");
    versionLabel->setAlignment(Qt::AlignCenter);
    navLayout->addWidget(versionLabel);
}

// 初始化右侧内容区
void MainWidgetPrivate::initRightContent()
{
    stackedContent = new QStackedWidget(q_ptr);

    // 1. 首页（收支卡片）
    homeWidget = new QWidget(stackedContent);
    homeWidget->setObjectName("homeWidget");
    QVBoxLayout* homeLayout = new QVBoxLayout(homeWidget);
    homeLayout->setContentsMargins(20, 20, 20, 20);
    homeLayout->setSpacing(20);

    // 页面标题
    QLabel* pageTitle = new QLabel(QStringLiteral("\U0001f4ca \u6536\u652f\u6c47\u603b"), homeWidget);
    pageTitle->setStyleSheet("font-size: 22px; font-weight: bold; color: #2d3436;");
    homeLayout->addWidget(pageTitle);

    // 4 张卡片横向排列
    QHBoxLayout* cardLayout = new QHBoxLayout;
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(15);

    QStringList cardRoles = { "total", "year", "month", "day" };
    QStringList cardTitles = {
        QStringLiteral("\U0001f310 \u603b\u6536\u652f"),
        QStringLiteral("\U0001f4c5 \u5e74\u6536\u652f"),
        QStringLiteral("\U0001f4c6 \u6708\u6536\u652f"),
        QStringLiteral("\U0001f31e \u4eca\u65e5\u6536\u652f")
    };

    for (int i = 0; i < cardRoles.size(); ++i)
    {
        QWidget* card = createSummaryCard(cardRoles[i], cardTitles[i], QStringLiteral("\u00a50.00"), QStringLiteral("\u00a50.00"));
        card->setObjectName("card" + QString(cardRoles[i][0].toUpper()) + cardRoles[i].mid(1));
        cardLayout->addWidget(card);
    }

    homeLayout->addLayout(cardLayout);
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
QWidget* MainWidgetPrivate::createSummaryCard(const QString& role,
    const QString& title,
    const QString& incomeText,
    const QString& expenseText)
{
    QWidget* card = new QWidget(q_ptr);
    card->setMinimumHeight(140);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(15, 12, 15, 12);
    cardLayout->setSpacing(8);

    // 卡片标题
    QLabel* titleLabel = new QLabel(title, card);
    titleLabel->setProperty("cardRole", "cardTitle");
    titleLabelMap[role] = titleLabel;
    cardLayout->addWidget(titleLabel);

    // 收入
    QLabel* incomeTitle = new QLabel(QStringLiteral("\u6536\u5165"), card);
    incomeTitle->setProperty("cardRole", "cardIncomeLabel");
    QLabel* incomeValue = new QLabel(incomeText, card);
    incomeValue->setProperty("cardRole", "cardIncomeValue");
    incomeLabelMap[role] = incomeValue;
    cardLayout->addWidget(incomeTitle);
    cardLayout->addWidget(incomeValue);

    // 支出
    QLabel* expenseTitle = new QLabel(QStringLiteral("\u652f\u51fa"), card);
    expenseTitle->setProperty("cardRole", "cardExpenseLabel");
    QLabel* expenseValue = new QLabel(expenseText, card);
    expenseValue->setProperty("cardRole", "cardExpenseValue");
    expenseLabelMap[role] = expenseValue;
    cardLayout->addWidget(expenseTitle);
    cardLayout->addWidget(expenseValue);

    // 盈余
    QLabel* profitTitle = new QLabel(QStringLiteral("\u76c8\u4f59"), card);
    profitTitle->setProperty("cardRole", "cardProfitLabel");
    QLabel* profitValue = new QLabel(QStringLiteral("\u00a50.00"), card);
    profitValue->setProperty("cardRole", "cardProfitValue");
    profitLabelMap[role] = profitValue;
    cardLayout->addWidget(profitTitle);
    cardLayout->addWidget(profitValue);

    cardLayout->addStretch();

    return card;
}

QString MainWidgetPrivate::toLocaleString(double amount)
{
    QLocale locale(QLocale::Chinese);

    return locale.toCurrencyString(amount);
}

// 外部类实现
MainWidget::MainWidget(const QString& dbPath, QWidget* parent)
    : QWidget(parent)
    , d_ptr(new MainWidgetPrivate(dbPath, this))
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

    // 从数据库加载分类/账户列表，替换 UI 层硬编码
    {
        auto incCats = d_ptr->dataCenter.getCategoryList(QStringLiteral("\u6536\u5165"));
        auto expCats = d_ptr->dataCenter.getCategoryList(QStringLiteral("\u652f\u51fa"));
        auto fromAccs = d_ptr->dataCenter.getAccountList("from");
        auto toAccs = d_ptr->dataCenter.getAccountList("to");
        d->accountingWidget->loadCategoryLists(incCats, expCats, fromAccs, toAccs);

        // 合并所有分类用于分析页
        QStringList allCats = incCats;
        for (const auto& c : expCats) {
            if (!allCats.contains(c)) allCats << c;
        }
        d->analysisWidget->loadCategoryList(allCats);
    }

    connect(d->accountingWidget, &AccountingWidget::addRecord, [this](TradeRecord record)
        {
            qDebug() << "添加记录";
            d_ptr->dataCenter.addRecord(record);
            auto res = d_ptr->dataCenter.getNewRecord();
            d_ptr->accountingWidget->afterAddRecord(res);
            int total = d_ptr->dataCenter.getRecordCount();
            d_ptr->accountingWidget->getPageController()->setTotalPage(total, 20);
        });

    connect(d->accountingWidget, &AccountingWidget::updateRecord, [this](TradeRecord record)
        {
            qDebug() << "修改记录";
            d_ptr->dataCenter.updateRecord(record);
            auto list = d_ptr->dataCenter.getAllRecords();
            d_ptr->accountingWidget->fillTable(list);
            updateBalanceCards();
            int total = d_ptr->dataCenter.getRecordCount();
            d_ptr->accountingWidget->getPageController()->setTotalPage(total, 20);
        });

    connect(d->accountingWidget, &AccountingWidget::deleteRecord, [this](QString id)
        {
            qDebug() << "删除记录, id:" << id;
            d_ptr->dataCenter.deleteRecord(id.toInt());
            auto list = d_ptr->dataCenter.getAllRecords();
            d_ptr->accountingWidget->fillTable(list);
            updateBalanceCards();
            int total = d_ptr->dataCenter.getRecordCount();
            d_ptr->accountingWidget->getPageController()->setTotalPage(total, 20);
        });

    connect(d->analysisWidget, &AnalysisWidget::dataRequested, [this](QDate start, QDate end, QString type, QString category)
    {
        qDebug() << "图表数据请求:" << start << end << type << category;
        auto catData = d_ptr->dataCenter.getCategoryStats(start, end, type);
        auto dailyData = d_ptr->dataCenter.getDailyStats(start, end, type);
        d_ptr->analysisWidget->loadPieData(catData);
        d_ptr->analysisWidget->loadLineData(dailyData);
    });

    connect(d->accountingWidget, &AccountingWidget::filterRequested, [this](QDate start, QDate end)
    {
        qDebug() << "筛选记录:" << start << end;
        auto list = d_ptr->dataCenter.getRecordsByDate(start, end);
        d_ptr->accountingWidget->fillTable(list);
    });

    connect(d->accountingWidget, &AccountingWidget::searchRequested, [this](QString keyword)
    {
        qDebug() << "搜索记录:" << keyword;
        auto list = d_ptr->dataCenter.searchRecords(keyword);
        d_ptr->accountingWidget->fillTable(list);
    });

    connect(d->accountingWidget, &AccountingWidget::pageRequested, [this](int page)
    {
        qDebug() << "分页请求, page:" << page;
        auto list = d_ptr->dataCenter.getRecords(page, 20);
        d_ptr->accountingWidget->fillTable(list);
    });

    auto list = d_ptr->dataCenter.getAllRecords();
    d->accountingWidget->fillTable(list);

    int totalCount = d_ptr->dataCenter.getRecordCount();
    d->accountingWidget->getPageController()->setTotalPage(totalCount, 20);

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

    d->homeBtn->setChecked(type == "home");
    d->accountingBtn->setChecked(type == "accounting");
    d->analysisBtn->setChecked(type == "analysis");
    d->settingsBtn->setChecked(type == "settings");

    updateBalanceCards();
}

// 更新收支卡片数据
void MainWidget::updateBalanceCards()
{
    Q_D(MainWidget);
    // 模拟数据


    QDate today = QDate::currentDate();

    Statistics totalStats = d->dataCenter.getStatistics(TimeRange::Total);
    Statistics yearStats = d->dataCenter.getStatistics(TimeRange::Year, today.year());
    Statistics monthStats = d->dataCenter.getStatistics(TimeRange::Month, today.year(), today.month());
    Statistics dayStats = d->dataCenter.getStatistics(TimeRange::Day, today.year(), today.month(), today.day());

    d->incomeLabelMap["total"]->setText(d->toLocaleString(totalStats.income));
    d->expenseLabelMap["total"]->setText(d->toLocaleString(totalStats.expense));
    d->profitLabelMap["total"]->setText(d->toLocaleString(totalStats.profit));

    d->incomeLabelMap["year"]->setText(d->toLocaleString(yearStats.income));
    d->expenseLabelMap["year"]->setText(d->toLocaleString(yearStats.expense));
    d->profitLabelMap["year"]->setText(d->toLocaleString(yearStats.profit));

    d->incomeLabelMap["month"]->setText(d->toLocaleString(monthStats.income));
    d->expenseLabelMap["month"]->setText(d->toLocaleString(monthStats.expense));
    d->profitLabelMap["month"]->setText(d->toLocaleString(monthStats.profit));
     
    d->incomeLabelMap["day"]->setText(d->toLocaleString(dayStats.income));
    d->expenseLabelMap["day"]->setText(d->toLocaleString(dayStats.expense));
    d->profitLabelMap["day"]->setText(d->toLocaleString(dayStats.profit));

    d->titleLabelMap["year"]->setText(QString::number(today.year()) + "年收支");
    d->titleLabelMap["month"]->setText(QString::number(today.month()) + "月收支");
    d->titleLabelMap["day"]->setText(QString::number(today.month()) + "月" + QString::number(today.day()) + "日收支");
}