#include "DBHelper.h"

#include <sqlite_modern_cpp.h>
#include <thread>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>

class DBHelper::Impl
{
public:
    sqlite::database* m_db = nullptr;
    
	std::thread worker;
	std::queue<std::function<void()>> tasks;
	std::mutex mutex;
	std::condition_variable cv;
    bool running = true;
};

DBHelper::DBHelper() : m_pImpl(std::make_unique<Impl>())
{
	m_pImpl->worker = std::thread(&DBHelper::workerLoop, this);
}

DBHelper::~DBHelper()
{
    {
        std::lock_guard<std::mutex> lock(m_pImpl->mutex);
        m_pImpl->running = false;
    }
    m_pImpl->cv.notify_all();

    if (m_pImpl->worker.joinable())
    {
        m_pImpl->worker.join();
    }
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
    return submit([this, path]()->QueryResult
        {
            if (m_pImpl->m_db) { return { true,"",{} }; }

            try
            {
                m_pImpl->m_db = new sqlite::database(path);
                *m_pImpl->m_db << "PRAGMA foreign_keys = ON;";          // 外键约束
                *m_pImpl->m_db << "PRAGMA journal_mode = WAL;";         // 预写日志
                *m_pImpl->m_db << "PRAGMA synchronous = NORMAL;";       // 平衡速度+保存数据
                *m_pImpl->m_db << "PRAGMA busy_timeout = 5000;";        // 忙等待5s

                return { true, "",{} };
            }
            catch (const sqlite::sqlite_exception& ex)
            {
                return { false, ex.what(),{} };
            }
        });
}

DBHelper::QueryResult DBHelper::exec(const std::string& sql)
{
    return submit([this, sql]()->QueryResult
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
        });
}

DBHelper::QueryResult DBHelper::exec(const std::string& sql, const std::vector<std::string>& params)
{
    return submit([this, sql, params]()->QueryResult
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
        });
}

DBHelper::QueryResult DBHelper::query(const std::string& sql)
{
    return submit([this, sql]()->QueryResult
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
        });
}

DBHelper::QueryResult DBHelper::query(const std::string& sql, const std::vector<std::string>& params)
{
    return submit([this, sql, params]()->QueryResult
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
        });
}

DBHelper::QueryResult DBHelper::submit(const std::function<QueryResult()>& task)
{
    auto packaged = std::make_shared<std::packaged_task<QueryResult()>>(task);
    auto future = packaged->get_future();

    { // 控制锁的生命周期
        std::lock_guard<std::mutex> lock(m_pImpl->mutex);
        m_pImpl->tasks.emplace([packaged]() { (*packaged)(); });

        m_pImpl->cv.notify_one();
    }
	return future.get();
}

void DBHelper::workerLoop()
{
    while (m_pImpl->running)
    {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(m_pImpl->mutex);
            m_pImpl->cv.wait(lock, [this] {return !m_pImpl->tasks.empty() || !m_pImpl->running; });

            if (!m_pImpl->running && m_pImpl->tasks.empty()) { return; }
            task = std::move(m_pImpl->tasks.front());
            m_pImpl->tasks.pop();
        }

        task();
    }
}