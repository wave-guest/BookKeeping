#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <iostream>
#include <cmath>

#include <DataCenter/DataCenter.h>

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name, expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "[FAIL] " << name << std::endl; \
            g_failed++; \
        } else { \
            std::cout << "[PASS] " << name << std::endl; \
            g_passed++; \
        } \
    } while(0)

static bool approxEq(double a, double b, double eps = 0.001)
{
    return std::fabs(a - b) < eps;
}

static TradeRecord makeRec(const QString& type, const QString& cat,
    double amount, const QString& date, const QString& note = QString(),
    const QString& from = QString(), const QString& to = QString())
{
    TradeRecord r;
    r.trade_type = type;
    r.trade_category = cat;
    r.amount = amount;
    r.trade_time = date;
    r.remark = note;
    r.source = "manual";
    r.from = from;
    r.to = to;
    return r;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString dbDir = QCoreApplication::applicationDirPath() + "/test_config";
QDir(dbDir).removeRecursively();
    QDir().mkpath(dbDir);
    QString dbFile = dbDir + "/account.db";

    {
        DataCenter dc;
        dc.initTables(dbFile);

        // use \uXXXX escapes for Chinese chars to avoid source encoding issues
        auto r1 = makeRec(
            QStringLiteral("\u652f\u51fa"),       // \xE6\x94\xAF\xE5\x87\xBA = 支出
            QStringLiteral("\u9910\u996e"),       // \xE9\xA4\x90\xE9\xA5\xAE = 餐饮
            35.50, "2026-07-01", "lunch");
        bool ok = dc.addRecord(r1);
        TEST("addRecord 1st expense", ok);

        auto r2 = makeRec(
            QStringLiteral("\u6536\u5165"),       // \xE6\x94\xB6\xE5\x85\xA5 = 收入
            QStringLiteral("\u5de5\u8d44"),       // \xE5\xB7\xA5\xE8\xB5\x84 = 工资
            15000.00, "2026-07-01", "july salary");
        ok = dc.addRecord(r2);
        TEST("addRecord 2nd income", ok);

        auto all = dc.getAllRecords();
        TEST("getAllRecords count=2", all.size() == 2);

        auto latest = dc.getNewRecord();
        TEST("getNewRecord id ok", !latest.id.isEmpty());
        TEST("getNewRecord amount=15000", approxEq(latest.amount, 15000.00));

        latest.remark = "updated";
        latest.amount = 138.00;
        ok = dc.updateRecord(latest);
        TEST("updateRecord ok", ok);
        auto updated = dc.getNewRecord();
        TEST("updateRecord remark", updated.remark == "updated");
        TEST("updateRecord amount=138", approxEq(updated.amount, 138.00));

        ok = dc.deleteRecord(latest.id.toInt());
        TEST("deleteRecord ok", ok);
        auto afterDel = dc.getAllRecords();
        TEST("deleteRecord count=1", afterDel.size() == 1);

        Statistics s = dc.getStatistics(TimeRange::Total);
        TEST("Total income=15000", approxEq(s.income, 15000.00));
        TEST("Total expense=35.50", approxEq(s.expense, 35.50));

        s = dc.getStatistics(TimeRange::Year, 2026);
        TEST("Year income=15000", approxEq(s.income, 15000.00));
        TEST("Year expense=35.50", approxEq(s.expense, 35.50));

        s = dc.getStatistics(TimeRange::Month, 2026, 7);
        TEST("Month income=15000", approxEq(s.income, 15000.00));
        TEST("Month expense=35.50", approxEq(s.expense, 35.50));

        s = dc.getStatistics(TimeRange::Day, 2026, 7, 1);
        TEST("Day income=15000", approxEq(s.income, 15000.00));
        TEST("Day expense=35.50", approxEq(s.expense, 35.50));

        s = dc.getTotalStats();
        TEST("getTotalStats income=15000", approxEq(s.income, 15000.00));
        s = dc.getYearStats(2026);
        TEST("getYearStats income=15000", approxEq(s.income, 15000.00));
        s = dc.getMonthStats(2026, 7);
        TEST("getMonthStats income=15000", approxEq(s.income, 15000.00));
        s = dc.getDayStats(2026, 7, 1);
        TEST("getDayStats income=15000", approxEq(s.income, 15000.00));

        double inc = dc.getIncome(TimeRange::Total);
        double exp = dc.getExpense(TimeRange::Total);
        double prof = dc.getProfit(TimeRange::Total);
        TEST("getIncome=15000", approxEq(inc, 15000.00));
        TEST("getExpense=35.50", approxEq(exp, 35.50));
        TEST("getProfit=inc-exp", approxEq(prof, inc - exp));

        // SQL injection test
        auto rInject = makeRec(
            QStringLiteral("\u652f\u51fa"),
            QStringLiteral("\u9910\u996e"),
            10.00, "2026-07-03",
            "remark' OR '1'='1",
            "source' OR '1'='1");
        ok = dc.addRecord(rInject);
        TEST("inject add ok", ok);

        auto all2 = dc.getAllRecords();
        TEST("inject count=3 table alive", all2.size() == 3);

        bool found = false;
        for (const auto& r : all2) {
            if (r.remark.contains("OR")) {
                found = true;
                TEST("inject remark intact", r.remark == "remark' OR '1'='1");
                TEST("inject from intact", r.from == "source' OR '1'='1");
                break;
            }
        }
        TEST("inject record found", found);
    }

    QDir(dbDir).removeRecursively();

    std::cout << "\n========================================" << std::endl;
    std::cout << "Total: " << (g_passed + g_failed)
              << " | Passed: " << g_passed
              << " | Failed: " << g_failed << std::endl;
    std::cout << "========================================" << std::endl;

    return g_failed > 0 ? 1 : 0;
}
