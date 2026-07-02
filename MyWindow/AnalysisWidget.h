#pragma once

#include <QWidget>
#include <QScopedPointer>
#include <QtCharts> // 引入图表头文件

class AnalysisWidgetPrivate;

class AnalysisWidget : public QWidget
{
    Q_OBJECT
        Q_DECLARE_PRIVATE(AnalysisWidget)

public:
    explicit AnalysisWidget(QWidget* parent = nullptr);
    ~AnalysisWidget() override;

private slots:
    // 筛选条件变化时更新图表
    void onFilterChanged();
    // 切换图表类型（饼图/折线图）
    void onChartTypeChanged();

private:
    QScopedPointer<AnalysisWidgetPrivate> d_ptr;
};