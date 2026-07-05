#pragma once

#include <QWidget>
#include <QScopedPointer>

#include "Export.h"

class MainWidgetPrivate;

class MYWINDOW_EXPORT MainWidget : public QWidget
{
    Q_OBJECT
        Q_DECLARE_PRIVATE(MainWidget)

public:
    explicit MainWidget(const QString& dbPath, QWidget* parent = nullptr);
    ~MainWidget() override;

private slots:
    // 切换右侧内容区
    void onSwitchContent(const QString& type);
    // 更新收支卡片数据
    void updateBalanceCards();

private:
    QScopedPointer<MainWidgetPrivate> d_ptr;
};