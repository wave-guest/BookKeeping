#pragma once

#include <memory>
#include <QObject>
#include <QList>
#include <QMap>

#include "TradeRecord.h"

#if defined(_MSC_VER) || defined(WIN32) || defined(_WIN32)
#if defined(DATA_CENTER_LIBRARY)
#define DATA_CENTER_EXPORT __declspec(dllexport)
#else
#define DATA_CENTER_EXPORT __declspec(dllimport)
#endif
#else
#define DATA_CENTER_EXPORT
#endif

class DATA_CENTER_EXPORT DataCenter : public QObject
{
    Q_OBJECT
public:
    explicit DataCenter(QObject* parent = nullptr);
    ~DataCenter() override;

    void initTables(const QString& dbPath = QString());
    // 核心统计接口（一个函数搞定所有）
    Statistics getStatistics(TimeRange range,
        int year = 0,
        int month = 0,
        int day = 0);

    // 便捷函数（内联调用上面的函数）
    Statistics getTotalStats();
    Statistics getYearStats(int year);
    Statistics getMonthStats(int year, int month);
    Statistics getDayStats(int year, int month, int day);

    // 如果需要单独获取某个指标
    double getIncome(TimeRange range, int year = 0, int month = 0, int day = 0);
    double getExpense(TimeRange range, int year = 0, int month = 0, int day = 0);
    double getProfit(TimeRange range, int year = 0, int month = 0, int day = 0);


    // --------------- 交易记录业务接口 ---------------
    bool addRecord(const TradeRecord& record);        // 添加记录（手动/支付宝）
    bool deleteRecord(int id);                        // 删除记录
    bool updateRecord(const TradeRecord& record);     // 修改记录
    QList<TradeRecord> getAllRecords();               // 获取所有记录
    TradeRecord getNewRecord();                       // 获取最新记录

private:
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
};