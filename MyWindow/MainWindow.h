#pragma once

#include <QWidget>
#include <QScopedPointer>

#if defined(_MSC_VER) || defined(WIN32) || defined(_WIN32)
#if defined(MYWINDOW_LIBRARY)
#define MYWINDOW_EXPORT __declspec(dllexport)
#else
#define MYWINDOW_EXPORT __declspec(dllimport)
#endif
#else
#define MYWINDOW_EXPORT
#endif

class MainWidgetPrivate;

class MYWINDOW_EXPORT MainWidget : public QWidget
{
    Q_OBJECT
        Q_DECLARE_PRIVATE(MainWidget)

public:
    explicit MainWidget(QWidget* parent = nullptr);
    ~MainWidget() override;

private slots:
    // 切换右侧内容区
    void onSwitchContent(const QString& type);
    // 更新收支卡片数据
    void updateBalanceCards();

private:
    QScopedPointer<MainWidgetPrivate> d_ptr;
};