#include "DataCenter.h"
#include "RecordRepository.h"
#include "DBHelper.h"

#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QString>

#include <cstdio>
#include <string>
#include <vector>

class DataCenter::Impl
{
public:
    DBHelper helper;
    std::unique_ptr<RecordRepository> repo;

    void ensureRepo()
    {
        if (!repo)
            repo = std::make_unique<RecordRepository>(helper);
    }
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
    m_pImpl->ensureRepo();
    bool ok = m_pImpl->repo->insert(record);
    if (!ok)
        qDebug() << "插入失败";
    return ok;
}

bool DataCenter::deleteRecord(int id)
{
    m_pImpl->ensureRepo();
    bool ok = m_pImpl->repo->remove(id);
    if (!ok)
        qDebug() << "删除失败";
    return ok;
}

bool DataCenter::updateRecord(const TradeRecord& record)
{
    m_pImpl->ensureRepo();
    bool ok = m_pImpl->repo->update(record);
    if (!ok)
        qDebug() << "更新失败";
    return ok;
}

QList<TradeRecord> DataCenter::getAllRecords()
{
    m_pImpl->ensureRepo();
    return m_pImpl->repo->fetchAll();
}

TradeRecord DataCenter::getNewRecord()
{
    m_pImpl->ensureRepo();
    return m_pImpl->repo->fetchNewest();
}

void DataCenter::initTables(const QString& dbPath)
{
    QString path = dbPath.isEmpty()
        ? QCoreApplication::applicationDirPath() + "/config/account.db"
        : dbPath;
    m_pImpl->helper.open(path.toStdString());
    m_pImpl->ensureRepo();
    auto ok = m_pImpl->repo->initializeSchema();
    if (!ok)
        qDebug() << "创建表失败";
}

Statistics DataCenter::getStatistics(TimeRange range, int year, int month, int day)
{
    m_pImpl->ensureRepo();
    Statistics stats;

    std::string whereClause;
    std::vector<std::string> params;

    switch (range) {
    case TimeRange::Total:
        break;
    case TimeRange::Year:
        whereClause = " WHERE strftime('%Y', date) = ?";
        params.push_back(std::to_string(year));
        break;
    case TimeRange::Month: {
        char monthStr[8];
        snprintf(monthStr, sizeof(monthStr), "%04d-%02d", year, month);
        whereClause = " WHERE strftime('%Y-%m', date) = ?";
        params.push_back(std::string(monthStr));
        break;
    }
    case TimeRange::Day: {
        char dayStr[11];
        snprintf(dayStr, sizeof(dayStr), "%04d-%02d-%02d", year, month, day);
        whereClause = " WHERE date = ?";
        params.push_back(std::string(dayStr));
        break;
    }
    }

    auto ie = m_pImpl->repo->fetchIncomeExpense(whereClause, params);
    stats.income = ie.income;
    stats.expense = ie.expense;
    stats.profit = stats.income - stats.expense;

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
    m_pImpl->ensureRepo();
    return m_pImpl->repo->fetchCategoryStats(start, end, type);
}

QMap<QString, double> DataCenter::getDailyStats(const QDate& start, const QDate& end, const QString& type)
{
    m_pImpl->ensureRepo();
    return m_pImpl->repo->fetchDailyStats(start, end, type);
}

QList<TradeRecord> DataCenter::getRecordsByDate(const QDate& start, const QDate& end)
{
    m_pImpl->ensureRepo();
    return m_pImpl->repo->fetchByDateRange(start, end);
}

QList<TradeRecord> DataCenter::searchRecords(const QString& keyword)
{
    m_pImpl->ensureRepo();
    return m_pImpl->repo->searchByKeyword(keyword);
}

QList<TradeRecord> DataCenter::getRecords(int page, int pageSize)
{
    m_pImpl->ensureRepo();
    return m_pImpl->repo->fetchPage(page, pageSize);
}

int DataCenter::getRecordCount()
{
    m_pImpl->ensureRepo();
    return m_pImpl->repo->countAll();
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