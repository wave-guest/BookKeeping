#include "RecordRepository.h"

#include <cstdio>
#include <string>
#include <vector>

static const char* kColumnList = "id, type, category, source, amount, date, remark, source_account, target_account";

RecordRepository::RecordRepository(DBHelper& helper)
    : m_helper(helper)
{
}

bool RecordRepository::initializeSchema()
{
    auto res = m_helper.exec(R"(
        CREATE TABLE IF NOT EXISTS records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            type TEXT NOT NULL,
            category TEXT NOT NULL,
            source TEXT NOT NULL,
            amount REAL NOT NULL,
            date TEXT NOT NULL,
            remark TEXT,
            source_account TEXT,
            target_account TEXT
        );
    )");
    return res.success;
}

bool RecordRepository::insert(const TradeRecord& record)
{
    std::string sql = R"(
        INSERT INTO records (type, category, source, amount, date, remark, source_account, target_account)
        VALUES(?, ?, ?, ?, ?, ?, ?, ?);
    )";
    auto res = m_helper.exec(sql, {
        record.trade_type.toStdString(),
        record.trade_category.toStdString(),
        record.source.toStdString(),
        std::to_string(record.amount),
        record.trade_time.toStdString(),
        record.remark.toStdString(),
        record.from.toStdString(),
        record.to.toStdString()
    });
    return res.success;
}

bool RecordRepository::remove(int id)
{
    std::string sql = "DELETE FROM records WHERE id = ?;";
    auto res = m_helper.exec(sql, { std::to_string(id) });
    return res.success;
}

bool RecordRepository::update(const TradeRecord& record)
{
    std::string sql = R"(
        UPDATE records
        SET
            type = ?,
            category = ?,
            source = ?,
            amount = ?,
            date = ?,
            remark = ?,
            source_account = ?,
            target_account = ?
        WHERE id = ?;
    )";
    auto res = m_helper.exec(sql, {
        record.trade_type.toStdString(),
        record.trade_category.toStdString(),
        record.source.toStdString(),
        std::to_string(record.amount),
        record.trade_time.toStdString(),
        record.remark.toStdString(),
        record.from.toStdString(),
        record.to.toStdString(),
        record.id.toStdString()
    });
    return res.success;
}

QList<TradeRecord> RecordRepository::fetchAll()
{
    std::string sql = "SELECT " + std::string(kColumnList) + " FROM records ORDER BY date DESC;";
    auto res = m_helper.query(sql);
    return parseRecords(res);
}

TradeRecord RecordRepository::fetchNewest()
{
    std::string sql = "SELECT " + std::string(kColumnList) + " FROM records ORDER BY id DESC LIMIT 1;";
    auto res = m_helper.query(sql);
    auto records = parseRecords(res);
    if (!records.isEmpty())
        return records.first();
    return TradeRecord();
}

QList<TradeRecord> RecordRepository::fetchPage(int page, int pageSize)
{
    int offset = (page - 1) * pageSize;
    std::string sql = "SELECT " + std::string(kColumnList) + " FROM records ORDER BY date DESC LIMIT ? OFFSET ?;";
    auto res = m_helper.query(sql, { std::to_string(pageSize), std::to_string(offset) });
    return parseRecords(res);
}

int RecordRepository::countAll()
{
    std::string sql = "SELECT COUNT(*) FROM records;";
    auto res = m_helper.query(sql);
    if (res.success && !res.data.empty() && !res.data[0].empty()) {
        try { return std::stoi(res.data[0][0]); } catch (...) {}
    }
    return 0;
}

RecordRepository::IncomeExpense RecordRepository::fetchIncomeExpense(
    TimeRange range, int year, int month, int day)
{
    IncomeExpense result;
    std::string sql = R"(
        SELECT
            COALESCE(SUM(CASE WHEN type = '收入' THEN amount ELSE 0 END), 0) as income,
            COALESCE(SUM(CASE WHEN type = '支出' THEN amount ELSE 0 END), 0) as expense
        FROM records
    )";

    std::vector<std::string> params;
    switch (range) {
    case TimeRange::Total:
        break;
    case TimeRange::Year:
        sql += " WHERE strftime('%Y', date) = ?";
        params.push_back(std::to_string(year));
        break;
    case TimeRange::Month: {
        char monthStr[8];
        snprintf(monthStr, sizeof(monthStr), "%04d-%02d", year, month);
        sql += " WHERE strftime('%Y-%m', date) = ?";
        params.push_back(std::string(monthStr));
        break;
    }
    case TimeRange::Day: {
        char dayStr[11];
        snprintf(dayStr, sizeof(dayStr), "%04d-%02d-%02d", year, month, day);
        sql += " WHERE date = ?";
        params.push_back(std::string(dayStr));
        break;
    }
    }

    auto res = m_helper.query(sql, params);
    if (res.success && !res.data.empty() && res.data[0].size() >= 2)
    {
        try { result.income = std::stod(res.data[0][0]); } catch (...) {}
        try { result.expense = std::stod(res.data[0][1]); } catch (...) {}
    }
    return result;
}

QMap<QString, double> RecordRepository::fetchCategoryStats(
    const QDate& start, const QDate& end, const QString& type)
{
    QMap<QString, double> result;
    std::string sql = R"(
        SELECT category, SUM(amount) as total
        FROM records
        WHERE date >= ? AND date <= ?
    )";

    std::vector<std::string> params;
    params.push_back(start.toString("yyyy-MM-dd").toStdString());
    params.push_back(end.toString("yyyy-MM-dd").toStdString());

    if (!type.isEmpty() && type != QStringLiteral("\u5168\u90e8"))
    {
        sql += " AND type = ?";
        params.push_back(type.toStdString());
    }
    sql += " GROUP BY category ORDER BY total DESC";

    auto res = m_helper.query(sql, params);
    if (res.success)
    {
        for (const auto& row : res.data)
        {
            if (row.size() >= 2)
                result[QString::fromStdString(row[0])] = std::stod(row[1]);
        }
    }
    return result;
}

QMap<QString, double> RecordRepository::fetchDailyStats(
    const QDate& start, const QDate& end, const QString& type)
{
    QMap<QString, double> result;
    std::string sql = R"(
        SELECT date, SUM(amount) as total
        FROM records
        WHERE date >= ? AND date <= ?
    )";

    std::vector<std::string> params;
    params.push_back(start.toString("yyyy-MM-dd").toStdString());
    params.push_back(end.toString("yyyy-MM-dd").toStdString());

    if (!type.isEmpty() && type != QStringLiteral("\u5168\u90e8"))
    {
        sql += " AND type = ?";
        params.push_back(type.toStdString());
    }
    sql += " GROUP BY date ORDER BY date ASC";

    auto res = m_helper.query(sql, params);
    if (res.success)
    {
        for (const auto& row : res.data)
        {
            if (row.size() >= 2)
                result[QString::fromStdString(row[0])] = std::stod(row[1]);
        }
    }
    return result;
}

QStringList RecordRepository::fetchCategoryList(const QString& type)
{
    std::string sql = "SELECT DISTINCT category FROM records WHERE type = ? ORDER BY category";
    auto res = m_helper.query(sql, { type.toStdString() });
    QStringList list;
    if (res.success) {
        for (const auto& row : res.data)
            if (!row.empty())
                list << QString::fromStdString(row[0]);
    }
    return list;
}

QStringList RecordRepository::fetchAccountList(const QString& role)
{
    const char* col = (role == QStringLiteral("source") || role == QStringLiteral("from"))
        ? "source_account" : "target_account";
    std::string sql = "SELECT DISTINCT " + std::string(col) + " FROM records ORDER BY " + std::string(col);
    auto res = m_helper.query(sql);
    QStringList list;
    if (res.success) {
        for (const auto& row : res.data)
            if (!row.empty())
                list << QString::fromStdString(row[0]);
    }
    return list;
}

QList<TradeRecord> RecordRepository::fetchByDateRange(const QDate& start, const QDate& end)
{
    std::string sql = "SELECT " + std::string(kColumnList) + R"( FROM records
        WHERE date >= ? AND date <= ?
        ORDER BY date DESC;
    )";

    std::vector<std::string> params;
    params.push_back(start.toString("yyyy-MM-dd").toStdString());
    params.push_back(end.toString("yyyy-MM-dd").toStdString());

    auto res = m_helper.query(sql, params);
    return parseRecords(res);
}

QList<TradeRecord> RecordRepository::searchByKeyword(const QString& keyword)
{
    std::string sql = "SELECT " + std::string(kColumnList) + R"( FROM records
        WHERE remark LIKE ? ESCAPE '/' OR CAST(amount AS TEXT) LIKE ? ESCAPE '/'
        ORDER BY date DESC;
    )";

    std::string escaped;
    escaped.reserve(keyword.size());
    for (const QChar& ch : keyword) {
        if (ch == QLatin1Char('%') || ch == QLatin1Char('_') || ch == QLatin1Char('/'))
            escaped += '/';
        escaped += ch.toLatin1();
    }
    std::string pattern = "%" + escaped + "%";
    auto res = m_helper.query(sql, { pattern, pattern });
    return parseRecords(res);
}

TradeRecord RecordRepository::parseRecord(const std::vector<std::string>& row) const
{
    TradeRecord record;
    record.id = QString::fromStdString(row[0]);
    record.trade_type = QString::fromStdString(row[1]);
    record.trade_category = QString::fromStdString(row[2]);
    record.source = QString::fromStdString(row[3]);
    try { record.amount = std::stod(row[4]); } catch (...) { record.amount = 0; }
    record.trade_time = QString::fromStdString(row[5]);
    record.remark = QString::fromStdString(row[6]);
    record.from = QString::fromStdString(row[7]);
    record.to = QString::fromStdString(row[8]);
    return record;
}

QList<TradeRecord> RecordRepository::parseRecords(const DBHelper::QueryResult& res) const
{
    QList<TradeRecord> records;
    if (res.success)
    {
        for (const auto& row : res.data)
            records.append(parseRecord(row));
    }
    return records;
}