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

public:
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
    QString sql = 
        QString("INSERT INTO records (type, category, source, amount, date, remark, 'from', 'to') VALUES('%1' , '%2' , '%3' , '%4' , '%5' , '%6' , '%7', '%8');")
		.arg(record.trade_type)
		.arg(record.trade_category)
		.arg(record.source)
		.arg(record.amount)
		.arg(record.trade_time)
		.arg(record.remark)
        .arg(record.from)
        .arg(record.to);


    auto res = m_pImpl->helper.exec(sql.toStdString());
    if (!res.success)
    {
        qDebug() << "插入失败: " << res.error;
        record.info();
    }
    return res.success;
}

bool DataCenter::deleteRecord(int id)
{
    auto res = m_pImpl->helper.exec(std::string());
    return res.success;
}

bool DataCenter::updateRecord(const TradeRecord& record)
{
    QString sql =
        QString(R"(
        UPDATE records 
        SET
            'type' = '%1',
            'category' = '%2',
            'source' = '%3',
            'amount' = '%4',
            'date' = '%5',
            'remark' = '%6',
            'from' = '%7',
            'to' = '%8'
        WHERE id = '%9';)")
        .arg(record.trade_type)
        .arg(record.trade_category)
        .arg(record.source)
        .arg(record.amount)
        .arg(record.trade_time)
        .arg(record.remark)
        .arg(record.from)
        .arg(record.to)
        .arg(record.id);

    auto res = m_pImpl->helper.exec(sql.toStdString());
    if (!res.success)
    {
        qDebug() << "更新失败: " << res.error;
        record.info();
    }
    return res.success;
}

QList<TradeRecord> DataCenter::getAllRecords()
{
    QString sql = R"(
        SELECT * FROM records ORDER BY date DESC;
    )";
    auto res = m_pImpl->helper.query(sql.toStdString());
	QList <TradeRecord> records;
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
    QString sql = R"(
        SELECT * FROM records
        ORDER BY id DESC
        LIMIT 1;
    )";
    auto res = m_pImpl->helper.query(sql.toStdString());
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

void DataCenter::initTables()
{
    // 创建数据库
    // 原则上通过配置文件读取, 后面修改
    m_pImpl->helper.open(QCoreApplication::applicationDirPath().toStdString() + "/config/account.db");
    
    // 创建表
    auto res = m_pImpl->helper.exec(R"(
        CREATE TABLE IF NOT EXISTS records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            type INTEGER NOT NULL,
            category TEXT NOT NULL,
            source TEXT NOT NULL,
            amount REAL NOT NULL,
            date TEXT NOT NULL,
            remark TEXT,
            frome TEXT,
            to TEXT
        );
    )");
    if (!res.success)
    {
        qDebug() << "创建表失败: " << QString::fromStdString(res.error);
    }
}

Statistics DataCenter::getStatistics(TimeRange range, int year, int month, int day)
{

    Statistics stats;
    std::string sql;

    // 构建基础SQL
    sql = R"(
        SELECT 
            COALESCE(SUM(CASE WHEN type = '收入' THEN amount ELSE 0 END), 0) as income,
            COALESCE(SUM(CASE WHEN type = '支出' THEN amount ELSE 0 END), 0) as expense
        FROM records
    )";

    // 根据时间范围添加WHERE条件
    switch (range) {
    case TimeRange::Total:
        // 无条件，查询所有记录
        break;

    case TimeRange::Year:
        sql += " WHERE strftime('%Y', date) = '" + std::to_string(year) + "'";
        break;

    case TimeRange::Month: {
        // 格式化为 YYYY-MM
        char monthStr[8];
        snprintf(monthStr, sizeof(monthStr), "%04d-%02d", year, month);
        sql += " WHERE strftime('%Y-%m', date) = '" + std::string(monthStr) + "'";
        break;
    }

    case TimeRange::Day: {
        // 格式化为 YYYY-MM-DD
        char dayStr[11];
        snprintf(dayStr, sizeof(dayStr), "%04d-%02d-%02d", year, month, day);
        sql += " WHERE date = '" + std::string(dayStr) + "'";
        break;
    }
    }

    // 执行查询
    DBHelper::QueryResult result = m_pImpl->helper.query(sql);

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
