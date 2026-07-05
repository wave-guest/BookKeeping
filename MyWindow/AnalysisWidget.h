#pragma once

#include <QWidget>
#include <QScopedPointer>
#include <QDate>
#include <QMap>
#include <QString>
#include <QtCharts>

#include "MainWindow.h"

class AnalysisWidgetPrivate;

class MYWINDOW_EXPORT AnalysisWidget : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AnalysisWidget)

public:
    explicit AnalysisWidget(QWidget* parent = nullptr);
    ~AnalysisWidget() override;

    // MainWidget 调用回填数据
    void loadPieData(const QMap<QString, double>& categoryData);
    void loadLineData(const QMap<QString, double>& dailyData);

signals:
    // 筛选条件变化时请求数据
    void dataRequested(QDate start, QDate end, QString type);

private slots:
    void onFilterChanged();
    void onChartTypeChanged();

private:
    QScopedPointer<AnalysisWidgetPrivate> d_ptr;
};