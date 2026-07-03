#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

// SQL接口
class DBHelper
{

public:
    using Row = std::vector<std::string>;

    class QueryResult
    {
    public:
        QueryResult() : success(false), error(""), data() {}
        QueryResult(bool success, const std::string& error, const std::vector<Row>& data)
			: success(success), error(error), data(data) {}
    public:
        std::vector<Row> data; 
        std::string error;
        bool success = false;
    };

public:
    explicit DBHelper();
    ~DBHelper();
   
    bool isOpen() const;
	QueryResult open(const std::string& path);
    QueryResult exec(const std::string& sql);
    QueryResult exec(const std::string& sql, const std::vector<std::string>& params);
	QueryResult query(const std::string& sql);
    QueryResult query(const std::string& sql, const std::vector<std::string>& params);

private:
    class Impl;
    std::unique_ptr<Impl> m_pImpl;

	QueryResult submit(const std::function<QueryResult()>& task);
	void workerLoop();

};