#include "AnalysisWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>

// Impl类
class AnalysisWidgetPrivate
{
public:
    AnalysisWidgetPrivate(AnalysisWidget* q) : q_ptr(q) {}

    void initUI();
    // 生成模拟数据（实际需从记账记录中读取）
    void generateMockData();
    // 更新饼图
    void updatePieChart();
    // 更新折线图
    void updateLineChart();

    // 筛选控件
    QComboBox* typeCombo;      // 收支类型（全部/收入/支出）
    QComboBox* categoryCombo;  // 类别（全部/餐饮/工资等）
    QDateEdit* startDateEdit;  // 开始日期
    QDateEdit* endDateEdit;    // 结束日期
    QRadioButton* pieRadio;    // 饼图
    QRadioButton* lineRadio;   // 折线图
    QPushButton* refreshBtn;   // 刷新按钮

    // 图表控件
    QChart* chart;
    QChartView* chartView;
    QPieSeries* pieSeries;     // 饼图数据
    QLineSeries* lineSeries;   // 折线图数据

    AnalysisWidget* q_ptr;
};

// 初始化UI
void AnalysisWidgetPrivate::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(q_ptr);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 1. 筛选栏
    QWidget* filterWidget = new QWidget(q_ptr);
    QHBoxLayout* filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setSpacing(10);

    // 收支类型筛选
    filterLayout->addWidget(new QLabel("收支类型:"));
    typeCombo = new QComboBox(q_ptr);
    typeCombo->addItems({ "全部", "收入", "支出" });
    filterLayout->addWidget(typeCombo);

    // 类别筛选
    filterLayout->addWidget(new QLabel("类别:"));
    categoryCombo = new QComboBox(q_ptr);
    categoryCombo->addItems({ "全部", "餐饮", "交通", "工资", "购物", "房租" });
    filterLayout->addWidget(categoryCombo);

    // 日期筛选
    filterLayout->addWidget(new QLabel("日期范围:"));
    startDateEdit = new QDateEdit(QDate::currentDate().addMonths(-1), q_ptr);
    endDateEdit = new QDateEdit(QDate::currentDate(), q_ptr);
    filterLayout->addWidget(startDateEdit);
    filterLayout->addWidget(new QLabel("至"));
    filterLayout->addWidget(endDateEdit);

    // 图表类型
    filterLayout->addWidget(new QLabel("图表类型:"));
    pieRadio = new QRadioButton("饼图", q_ptr);
    lineRadio = new QRadioButton("折线图", q_ptr);
    pieRadio->setChecked(true);
    filterLayout->addWidget(pieRadio);
    filterLayout->addWidget(lineRadio);

    // 刷新按钮
    refreshBtn = new QPushButton("刷新", q_ptr);
    filterLayout->addWidget(refreshBtn);
    filterLayout->addStretch();

    mainLayout->addWidget(filterWidget);

    // 2. 图表区
    generateMockData();
    updatePieChart(); // 默认显示饼图
    chartView = new QChartView(chart, q_ptr);
    chartView->setRenderHint(QPainter::Antialiasing); // 抗锯齿
    mainLayout->addWidget(chartView, 1); // 占满剩余空间

    // 连接信号
    QObject::connect(refreshBtn, &QPushButton::clicked, q_ptr, &AnalysisWidget::onFilterChanged);
    QObject::connect(pieRadio, &QRadioButton::toggled, q_ptr, &AnalysisWidget::onChartTypeChanged);
}

// 生成模拟数据
void AnalysisWidgetPrivate::generateMockData()
{
    // 饼图：支出类别占比
    pieSeries = new QPieSeries();
    pieSeries->append("餐饮", 850.5);
    pieSeries->append("交通", 230);
    pieSeries->append("购物", 580);
    pieSeries->append("房租", 1500);

    // 折线图：近7天支出
    lineSeries = new QLineSeries();
    lineSeries->append(0, 120); // 7天前
    lineSeries->append(1, 80);
    lineSeries->append(2, 200);
    lineSeries->append(3, 90);
    lineSeries->append(4, 150);
    lineSeries->append(5, 70);
    lineSeries->append(6, 180); // 今天
}

// 更新饼图
void AnalysisWidgetPrivate::updatePieChart()
{
    chart = new QChart();
    chart->addSeries(pieSeries);
    chart->setTitle("收支类别占比");
    chart->legend()->setAlignment(Qt::AlignBottom); // 图例在底部
}

// 更新折线图
void AnalysisWidgetPrivate::updateLineChart()
{
    chart = new QChart();
    chart->addSeries(lineSeries);
    chart->setTitle("近7天收支趋势");
    // 添加坐标轴
    QValueAxis* xAxis = new QValueAxis();
    xAxis->setTitleText("天数");
    xAxis->setRange(0, 6);
    QValueAxis* yAxis = new QValueAxis();
    yAxis->setTitleText("金额(¥)");
    yAxis->setRange(0, 250);
    chart->addAxis(xAxis, Qt::AlignBottom);
    chart->addAxis(yAxis, Qt::AlignLeft);
    lineSeries->attachAxis(xAxis);
    lineSeries->attachAxis(yAxis);
}

// 外部类实现
AnalysisWidget::AnalysisWidget(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new AnalysisWidgetPrivate(this))
{
    Q_D(AnalysisWidget);
    d->initUI();
    setWindowTitle("收支分析");
}

AnalysisWidget::~AnalysisWidget() = default;

// 筛选条件变化更新图表
void AnalysisWidget::onFilterChanged()
{
    Q_D(AnalysisWidget);
    // 这里可根据筛选条件（类型/类别/日期）重新计算数据
    // 示例：仅刷新当前图表类型
    if (d->pieRadio->isChecked()) {
        d->updatePieChart();
    }
    else {
        d->updateLineChart();
    }
    d->chartView->setChart(d->chart);
}

// 切换图表类型
void AnalysisWidget::onChartTypeChanged()
{
    Q_D(AnalysisWidget);
    onFilterChanged(); // 复用筛选逻辑更新图表
}