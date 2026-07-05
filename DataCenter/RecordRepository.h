#pragma once

#include <string>
#include <vector>
#include <QDate>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

#include "TradeRecord.h"
#include "DBHelper.h"

class RecordRepository
{
public:
    explicit RecordRepository(DBHelper& helper);

    // DDL
    bool initializeSchema();

    // CRUD
    bool insert(const TradeRecord& record);
    bool remove(int id);
    bool update(const TradeRecord& record);
    QList<TradeRecord> fetchAll();
    TradeRecord fetchNewest();
    QList<TradeRecord> fetchPage(int page, int pageSize);
    int countAll();

    // 统计
    struct IncomeExpense {
        double income = 0;
        double expense = 0;
    };
    IncomeExpense fetchIncomeExpense(TimeRange range, int year = 0, int month = 0, int day = 0);

    QMap<QString, double> fetchCategoryStats(const QDate& start, const QDate& end,
                                              const QString& type);
    QMap<QString, double> fetchDailyStats(const QDate& start, const QDate& end,
                                           const QString& type);

    // 设置项
    std::string getSetting(const std::string& key);
    bool setSetting(const std::string& key, const std::string& value);

    // 分类/账户列表
    QStringList fetchCategoryList(const QString& type);
    QStringList fetchAccountList(const QString& role);

    // 筛选搜索
    QList<TradeRecord> fetchByDateRange(const QDate& start, const QDate& end);
    QList<TradeRecord> searchByKeyword(const QString& keyword);

private:
    TradeRecord parseRecord(const std::vector<std::string>& row) const;
    QList<TradeRecord> parseRecords(const DBHelper::QueryResult& res) const;

    DBHelper& m_helper;
};