#include "DBHelper.h"

#include <sqlite_modern_cpp.h>

class DBHelper::Impl
{
public:
    sqlite::database* m_db = nullptr;
};

DBHelper::DBHelper() : m_pImpl(std::make_unique<Impl>())
{
}

DBHelper::~DBHelper()
{
    if (m_pImpl->m_db)
    {
        delete m_pImpl->m_db;
        m_pImpl->m_db = nullptr;
    }
}

bool DBHelper::isOpen() const
{
    return m_pImpl->m_db != nullptr;
}

DBHelper::QueryResult DBHelper::open(const std::string& path)
{
    if (m_pImpl->m_db) { return { true,"",{} }; }

    try
    {
        m_pImpl->m_db = new sqlite::database(path);
        *m_pImpl->m_db << "PRAGMA foreign_keys = ON;";
        *m_pImpl->m_db << "PRAGMA journal_mode = WAL;";
        *m_pImpl->m_db << "PRAGMA synchronous = NORMAL;";
        *m_pImpl->m_db << "PRAGMA busy_timeout = 5000;";

        return { true, "",{} };
    }
    catch (const sqlite::sqlite_exception& ex)
    {
        return { false, ex.what(),{} };
    }
}

DBHelper::QueryResult DBHelper::exec(const std::string& sql)
{
    if (!m_pImpl->m_db) { return { false,"Database not open",{} }; }

    try
    {
        *m_pImpl->m_db << sql;
        return { true, "",{} };
    }
    catch (const sqlite::sqlite_exception& e)
    {
        switch (e.get_code())
        {
        case SQLITE_CONSTRAINT_FOREIGNKEY:
            return { false, "Foreign key constraint failed",{} };
        default:
            return { false, e.what(),{} };
        }
    }
}

DBHelper::QueryResult DBHelper::exec(const std::string& sql, const std::vector<std::string>& params)
{
    if (!m_pImpl->m_db) { return { false,"Database not open",{} }; }

    sqlite3* db = m_pImpl->m_db->connection().get();
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return { false, sqlite3_errmsg(db), {} };

    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
        stmtPtr(stmt, sqlite3_finalize);

    for (size_t i = 0; i < params.size(); ++i)
    {
        if (sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
            return { false, sqlite3_errmsg(db), {} };
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
        return { false, sqlite3_errmsg(db), {} };

    return { true, "", {} };
}

DBHelper::QueryResult DBHelper::query(const std::string& sql)
{
    if (!m_pImpl->m_db) { return { false,"Database not open",{} }; }

    std::vector<Row> result;
    sqlite3* db = m_pImpl->m_db->connection().get();

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return { false, sqlite3_errmsg(db),{} };

    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
        stmtPtr(stmt, sqlite3_finalize);

    int cols = sqlite3_column_count(stmt);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Row row;
        for (int i = 0; i < cols; ++i)
        {
            const unsigned char* txt = sqlite3_column_text(stmt, i);
            row.emplace_back(txt ? reinterpret_cast<const char*>(txt) : "");
        }
        result.push_back(row);
    }
    return { true, "", result };
}

DBHelper::QueryResult DBHelper::query(const std::string& sql, const std::vector<std::string>& params)
{
    if (!m_pImpl->m_db) { return { false,"Database not open",{} }; }

    std::vector<Row> result;
    sqlite3* db = m_pImpl->m_db->connection().get();

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return { false, sqlite3_errmsg(db), {} };

    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>
        stmtPtr(stmt, sqlite3_finalize);

    for (size_t i = 0; i < params.size(); ++i)
    {
        if (sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK)
            return { false, sqlite3_errmsg(db), {} };
    }

    int cols = sqlite3_column_count(stmt);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Row row;
        for (int i = 0; i < cols; ++i)
        {
            const unsigned char* txt = sqlite3_column_text(stmt, i);
            row.emplace_back(txt ? reinterpret_cast<const char*>(txt) : "");
        }
        result.push_back(row);
    }
    return { true, "", result };
}