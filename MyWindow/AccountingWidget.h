#pragma once

#include <QWidget>
#include <QScopedPointer>
#include <QDate>
#include <DataCenter/TradeRecord.h>

#include "MainWindow.h"

class AccountingWidgetPrivate;
class PageController;

class MYWINDOW_EXPORT AccountingWidget : public QWidget
{
    Q_OBJECT
        Q_DECLARE_PRIVATE(AccountingWidget)

public:
    explicit AccountingWidget(QWidget* parent = nullptr);
    ~AccountingWidget() override;

    void fillTable(const QList<TradeRecord>& list);
    void afterAddRecord(TradeRecord record);
    PageController* getPageController() const;

signals:
    void addRecord(TradeRecord record);
    void updateRecord(TradeRecord record);
    void deleteRecord(QString id);
    void filterRequested(QDate start, QDate end);
    void searchRequested(QString keyword);
    void pageRequested(int page);

private slots:
    // 左侧表单：添加/修改记录
    void onSaveRecord();
    // 右侧操作：删除选中记录
    void onDeleteRecord();
    // 右侧操作：修改选中记录
    void onEditRecord();
    // 右侧操作：筛选记录
    void onFilterRecords();
    // 右侧操作：搜索记录
    void onSearchRecords();
    // 表格选中行变化
    void onTableSelectionChanged();

private:
    QScopedPointer<AccountingWidgetPrivate> d_ptr;
};