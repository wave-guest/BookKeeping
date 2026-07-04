#include "DataCenter.h"
#include "DBHelper.h"

#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QString>


class DataCenter::Impl
{
public:
    DBHelper helper;
};

DataCenter::DataCenter(QObject* parent)
    : QObject(parent)
    , m_pImpl(std::make_unique<Impl>())
{
}

DataCenter::~DataCenter()
{
}

bool DataCenter::addRecord(const TradeRecord& record)
{
    std::string sql = R"(
        INSERT INTO records (type, category, source, amount, date, remark, source_account, target_account)
        VALUES(?, ?, ?, ?, ?, ?, ?, ?);
    )";

    auto res = m_pImpl->helper.exec(sql, {
        record.trade_type.toStdString(),
        record.trade_category.toStdString(),
        record.source.toStdString(),
        std::to_string(record.amount),
        record.trade_time.toStdString(),
        record.remark.toStdString(),
        record.from.toStdString(),
        record.to.toStdString()
    });

    if (!res.success)
    {
        qDebug() << "鎻掑叆澶辫触: " << res.error;
    }
    return res.success;
}

bool DataCenter::deleteRecord(int id)
{
    std::string sql = "DELETE FROM records WHERE id = ?;";
    auto res = m_pImpl->helper.exec(sql, { std::to_string(id) });
    if (!res.success)
    {
        qDebug() << "鍒犻櫎澶辫触: " << res.error;
    }
    return res.success;
}

bool DataCenter::updateRecord(const TradeRecord& record)
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

    auto res = m_pImpl->helper.exec(sql, {
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

    if (!res.success)
    {
        qDebug() << "鏇存柊澶辫触: " << res.error;
    }
    return res.success;
}

QList<TradeRecord> DataCenter::getAllRecords()
{
    std::string sql = R"(
        SELECT * FROM records ORDER BY date DESC;
    )";
    auto res = m_pImpl->helper.query(sql);
    QList<TradeRecord> records;
    if (res.success)
    {
        for (const auto& row : res.data)
        {
            TradeRecord record;
            record.id = QString::fromStdString(row[0]);
            record.trade_type = QString::fromStdString(row[1]);
            record.trade_category = QString::fromStdString(row[2]);
            record.source = QString::fromStdString(row[3]);
            record.amount = std::stod(row[4]);
            record.trade_time = QString::fromStdString(row[5]);
            record.remark = QString::fromStdString(row[6]);
            record.from = QString::fromStdString(row[7]);
            record.to = QString::fromStdString(row[8]);
            records.append(record);
        }
    }
    return records;
}

TradeRecord DataCenter::getNewRecord()
{
    std::string sql = R"(
        SELECT * FROM records
        ORDER BY id DESC
        LIMIT 1;
    )";
    auto res = m_pImpl->helper.query(sql);
    TradeRecord record;
    if (res.success)
    {
        for (const auto& row : res.data)
        {
            record.id = QString::fromStdString(row[0]);
            record.trade_type = QString::fromStdString(row[1]);
            record.trade_category = QString::fromStdString(row[2]);
            record.source = QString::fromStdString(row[3]);
            record.amount = std::stod(row[4]);
            record.trade_time = QString::fromStdString(row[5]);
            record.remark = QString::fromStdString(row[6]);
            record.from = QString::fromStdString(row[7]);
            record.to = QString::fromStdString(row[8]);
        }
    }
    return record;
}

void DataCenter::initTables(const QString& dbPath)
{
    QString path = dbPath.isEmpty()
        ? QCoreApplication::applicationDirPath() + "/config/account.db"
        : dbPath;
    m_pImpl->helper.open(path.toStdString());

    auto res = m_pImpl->helper.exec(R"(
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
    if (!res.success)
    {
        qDebug() << "鍒涘缓琛ㄥけ璐? " << QString::fromStdString(res.error);
    }
}

Statistics DataCenter::getStatistics(TimeRange range, int year, int month, int day)
{
    Statistics stats;
    std::string sql = R"(
        SELECT
            COALESCE(SUM(CASE WHEN type = '鏀跺叆' THEN amount ELSE 0 END), 0) as income,
            COALESCE(SUM(CASE WHEN type = '鏀嚭' THEN amount ELSE 0 END), 0) as expense
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

    DBHelper::QueryResult result = m_pImpl->helper.query(sql, params);

    if (result.success && !result.data.empty()) {
        const auto& row = result.data[0];
        if (row.size() >= 2) {
            stats.income = std::stod(row[0]);
            stats.expense = std::stod(row[1]);
            stats.profit = stats.income - stats.expense;
        }
    }

    return stats;
}

Statistics DataCenter::getTotalStats()
{
    return getStatistics(TimeRange::Total);
}

Statistics DataCenter::getYearStats(int year)
{
    return getStatistics(TimeRange::Year, year);
}

Statistics DataCenter::getMonthStats(int year, int month)
{
    return getStatistics(TimeRange::Month, year, month);
}

Statistics DataCenter::getDayStats(int year, int month, int day)
{
    return getStatistics(TimeRange::Day, year, month, day);
}

QMap<QString, double> DataCenter::getCategoryStats(const QDate& start, const QDate& end, const QString& type)
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

    auto res = m_pImpl->helper.query(sql, params);
    if (res.success)
    {
        for (const auto& row : res.data)
        {
            if (row.size() >= 2)
            {
                result[QString::fromStdString(row[0])] = std::stod(row[1]);
            }
        }
    }
    return result;
}

QMap<QString, double> DataCenter::getDailyStats(const QDate& start, const QDate& end, const QString& type)
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

    auto res = m_pImpl->helper.query(sql, params);
    if (res.success)
    {
        for (const auto& row : res.data)
        {
            if (row.size() >= 2)
            {
                result[QString::fromStdString(row[0])] = std::stod(row[1]);
            }
        }
    }
    return result;
}

QList<TradeRecord> DataCenter::getRecordsByDate(const QDate& start, const QDate& end)
{
    std::string sql = R"(
        SELECT * FROM records
        WHERE date >= ? AND date <= ?
        ORDER BY date DESC;
    )";

    std::vector<std::string> params;
    params.push_back(start.toString("yyyy-MM-dd").toStdString());
    params.push_back(end.toString("yyyy-MM-dd").toStdString());

    auto res = m_pImpl->helper.query(sql, params);
    QList<TradeRecord> records;
    if (res.success)
    {
        for (const auto& row : res.data)
        {
            TradeRecord record;
            record.id = QString::fromStdString(row[0]);
            record.trade_type = QString::fromStdString(row[1]);
            record.trade_category = QString::fromStdString(row[2]);
            record.source = QString::fromStdString(row[3]);
            record.amount = std::stod(row[4]);
            record.trade_time = QString::fromStdString(row[5]);
            record.remark = QString::fromStdString(row[6]);
            record.from = QString::fromStdString(row[7]);
            record.to = QString::fromStdString(row[8]);
            records.append(record);
        }
    }
    return records;
}

QList<TradeRecord> DataCenter::searchRecords(const QString& keyword)
{
    std::string sql = R"(
        SELECT * FROM records
        WHERE remark LIKE ? OR CAST(amount AS TEXT) LIKE ?
        ORDER BY date DESC;
    )";

    std::string pattern = "%" + keyword.toStdString() + "%";
    auto res = m_pImpl->helper.query(sql, { pattern, pattern });
    QList<TradeRecord> records;
    if (res.success)
    {
        for (const auto& row : res.data)
        {
            TradeRecord record;
            record.id = QString::fromStdString(row[0]);
            record.trade_type = QString::fromStdString(row[1]);
            record.trade_category = QString::fromStdString(row[2]);
            record.source = QString::fromStdString(row[3]);
            record.amount = std::stod(row[4]);
            record.trade_time = QString::fromStdString(row[5]);
            record.remark = QString::fromStdString(row[6]);
            record.from = QString::fromStdString(row[7]);
            record.to = QString::fromStdString(row[8]);
            records.append(record);
        }
    }
    return records;
}

double DataCenter::getIncome(TimeRange range, int year, int month, int day)
{
    return getStatistics(range, year, month, day).income;
}

double DataCenter::getExpense(TimeRange range, int year, int month, int day)
{
    return getStatistics(range, year, month, day).expense;
}

double DataCenter::getProfit(TimeRange range, int year, int month, int day)
{
    return getStatistics(range, year, month, day).profit;
}

