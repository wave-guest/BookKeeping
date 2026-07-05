#include "AnalysisWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QDateEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>

class AnalysisWidgetPrivate
{
public:
    AnalysisWidgetPrivate(AnalysisWidget* q) : q_ptr(q) {}

    void initUI();
    void updateChartView();

    // 筛选控件
    QComboBox* typeCombo;
    QComboBox* categoryCombo;
    QDateEdit* startDateEdit;
    QDateEdit* endDateEdit;
    QRadioButton* pieRadio;
    QRadioButton* lineRadio;
    QPushButton* refreshBtn;

    // 图表控件
    QChart* chart = nullptr;
    QChartView* chartView = nullptr;

    // 缓存数据
    QMap<QString, double> cachedCategoryData;
    QMap<QString, double> cachedDailyData;

    AnalysisWidget* q_ptr;
};

void AnalysisWidgetPrivate::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(q_ptr);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    // 1. 筛选栏（两行）
    QWidget* filterWidget = new QWidget(q_ptr);
    filterWidget->setObjectName("filterBar");
    QVBoxLayout* filterOuterLayout = new QVBoxLayout(filterWidget);
    filterOuterLayout->setContentsMargins(0, 0, 0, 0);
    filterOuterLayout->setSpacing(10);

    // 第一行：类型 + 分类 + 图表类型
    QHBoxLayout* row1 = new QHBoxLayout;
    row1->setSpacing(10);
    row1->addWidget(new QLabel(QStringLiteral("\u6536\u652f\u7c7b\u578b:")));
    typeCombo = new QComboBox(q_ptr);
    typeCombo->addItems({ QStringLiteral("\u5168\u90e8"), QStringLiteral("\u6536\u5165"), QStringLiteral("\u652f\u51fa") });
    row1->addWidget(typeCombo);

    row1->addWidget(new QLabel(QStringLiteral("\u7c7b\u522b:")));
    categoryCombo = new QComboBox(q_ptr);
    categoryCombo->addItems({ QStringLiteral("\u5168\u90e8"), QStringLiteral("\u9910\u996e"), QStringLiteral("\u4ea4\u901a"), QStringLiteral("\u5de5\u8d44"), QStringLiteral("\u8d2d\u7269"), QStringLiteral("\u623f\u79df") });
    row1->addWidget(categoryCombo);

    row1->addWidget(new QLabel(QStringLiteral("\u56fe\u8868\u7c7b\u578b:")));
    pieRadio = new QRadioButton(QStringLiteral("\u997c\u56fe"), q_ptr);
    lineRadio = new QRadioButton(QStringLiteral("\u6298\u7ebf\u56fe"), q_ptr);
    pieRadio->setChecked(true);
    row1->addWidget(pieRadio);
    row1->addWidget(lineRadio);
    row1->addStretch();
    filterOuterLayout->addLayout(row1);

    // 第二行：日期 + 刷新
    QHBoxLayout* row2 = new QHBoxLayout;
    row2->setSpacing(10);
    row2->addWidget(new QLabel(QStringLiteral("\u65e5\u671f\u8303\u56f4:")));
    startDateEdit = new QDateEdit(QDate::currentDate().addMonths(-1), q_ptr);
    endDateEdit = new QDateEdit(QDate::currentDate(), q_ptr);
    row2->addWidget(startDateEdit);
    row2->addWidget(new QLabel(QStringLiteral("\u81f3")));
    row2->addWidget(endDateEdit);
    row2->addStretch();

    refreshBtn = new QPushButton(QStringLiteral("\u5237\u65b0"), q_ptr);
    refreshBtn->setObjectName("refreshBtn");
    row2->addWidget(refreshBtn);
    filterOuterLayout->addLayout(row2);

    mainLayout->addWidget(filterWidget);

    // 2. 图表区（初始空状态）
    chart = new QChart();
    chart->setTitle(QStringLiteral("\u8bf7\u70b9\u51fb\u201c\u5237\u65b0\u201d\u52a0\u8f7d\u6570\u636e"));
    chartView = new QChartView(chart, q_ptr);
    chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(chartView, 1);

    // 连接信号
    QObject::connect(refreshBtn, &QPushButton::clicked, q_ptr, &AnalysisWidget::onFilterChanged);
    QObject::connect(pieRadio, &QRadioButton::toggled, q_ptr, &AnalysisWidget::onChartTypeChanged);
}

void AnalysisWidgetPrivate::updateChartView()
{
    if (chartView)
    {
        chartView->setChart(chart);
    }
}

// 外部类实现
AnalysisWidget::AnalysisWidget(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new AnalysisWidgetPrivate(this))
{
    setObjectName("analysisPage");
    Q_D(AnalysisWidget);
    d->initUI();
    setWindowTitle(QStringLiteral("\u6536\u652f\u5206\u6790"));
}

AnalysisWidget::~AnalysisWidget() = default;

void AnalysisWidget::loadPieData(const QMap<QString, double>& categoryData)
{
    Q_D(AnalysisWidget);
    d->cachedCategoryData = categoryData;

    if (!d->pieRadio->isChecked()) return;

    QPieSeries* series = new QPieSeries();
    double total = 0;
    for (auto it = categoryData.begin(); it != categoryData.end(); ++it)
    {
        series->append(it.key(), it.value());
        total += it.value();
    }

    d->chart = new QChart();
    d->chart->addSeries(series);
    d->chart->setTitle(QStringLiteral("\u6536\u652f\u7c7b\u522b\u5360\u6bd4"));

    if (total == 0)
    {
        d->chart->setTitle(QStringLiteral("\u6682\u65e0\u6570\u636e"));
    }

    d->chart->legend()->setAlignment(Qt::AlignBottom);
    d->updateChartView();
}

void AnalysisWidget::loadLineData(const QMap<QString, double>& dailyData)
{
    Q_D(AnalysisWidget);
    d->cachedDailyData = dailyData;

    if (!d->lineRadio->isChecked()) return;

    QLineSeries* series = new QLineSeries();
    double maxVal = 0;
    int idx = 0;
    for (auto it = dailyData.begin(); it != dailyData.end(); ++it, ++idx)
    {
        series->append(idx, it.value());
        if (it.value() > maxVal) maxVal = it.value();
    }

    d->chart = new QChart();
    d->chart->addSeries(series);
    d->chart->setTitle(QStringLiteral("\u6bcf\u65e5\u6536\u652f\u8d8b\u52bf"));

    QValueAxis* xAxis = new QValueAxis();
    xAxis->setTitleText(QStringLiteral("\u65e5\u671f"));
    xAxis->setRange(0, std::max(idx - 1, 1));
    xAxis->setLabelFormat("%d");

    QValueAxis* yAxis = new QValueAxis();
    yAxis->setTitleText(QStringLiteral("\u91d1\u989d(\u00a5)"));
    yAxis->setRange(0, maxVal > 0 ? maxVal * 1.2 : 100);

    d->chart->addAxis(xAxis, Qt::AlignBottom);
    d->chart->addAxis(yAxis, Qt::AlignLeft);
    series->attachAxis(xAxis);
    series->attachAxis(yAxis);

    d->updateChartView();
}

void AnalysisWidget::onFilterChanged()
{
    Q_D(AnalysisWidget);
    emit dataRequested(
        d->startDateEdit->date(),
        d->endDateEdit->date(),
        d->typeCombo->currentText(),
        d->categoryCombo->currentText()
    );
}

void AnalysisWidget::onChartTypeChanged()
{
    Q_D(AnalysisWidget);
    if (d->pieRadio->isChecked())
    {
        loadPieData(d->cachedCategoryData);
    }
    else
    {
        loadLineData(d->cachedDailyData);
    }
}