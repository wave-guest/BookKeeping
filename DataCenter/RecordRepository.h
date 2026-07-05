#pragma once

#include <string>
#include <vector>
#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

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
    IncomeExpense fetchIncomeExpense(const std::string& whereClause,
                                     const std::vector<std::string>& params);

    QMap<QString, double> fetchCategoryStats(const QDate& start, const QDate& end,
                                              const QString& type);
    QMap<QString, double> fetchDailyStats(const QDate& start, const QDate& end,
                                           const QString& type);

    // 筛选搜索
    QList<TradeRecord> fetchByDateRange(const QDate& start, const QDate& end);
    QList<TradeRecord> searchByKeyword(const QString& keyword);

private:
    TradeRecord parseRecord(const std::vector<std::string>& row) const;
    QList<TradeRecord> parseRecords(const DBHelper::QueryResult& res) const;

    DBHelper& m_helper;
};