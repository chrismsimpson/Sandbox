#include <print>
#include <iostream>
#include <mutex>
#include <memory>
#include <optional>
#include <variant>

#include "Foo.h"
#include "sqlite3.h"

class SqliteServer
{
public:
    SqliteServer(const std::string &db_name) : db_name_(db_name)
    {
        if (sqlite3_open(db_name.c_str(), &db_) != SQLITE_OK)
        {
            throw std::runtime_error("Failed to open database");
        }
    }

    ~SqliteServer()
    {
        if (db_)
        {
            sqlite3_close(db_);
        }
    }

    std::optional<sqlite3_stmt *> prepareQuery(const std::string &query, bool is_write)
    {
        std::optional<sqlite3_stmt *> stmt;
        std::unique_lock<std::mutex> lock(write_mutex_, std::defer_lock);

        if (is_write)
        {
            lock.lock(); // Lock the mutex for write queries
        }

        sqlite3_stmt *prepared_stmt = nullptr;
        if (sqlite3_prepare_v2(db_, query.c_str(), -1, &prepared_stmt, nullptr) == SQLITE_OK)
        {
            stmt = prepared_stmt;
        }

        return stmt;
    }

    void executeQuery(sqlite3_stmt *stmt)
    {
        if (stmt)
        {
            int rc;
            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
            {
                // Process row data if needed
            }
            if (rc != SQLITE_DONE)
            {
                std::cerr << "Failed to execute query: " << sqlite3_errmsg(db_) << std::endl;
            }
            sqlite3_finalize(stmt);
        }
    }

private:
    std::string db_name_;
    sqlite3 *db_ = nullptr;
    std::mutex write_mutex_;
};

int main()
{
    try
    {
        SqliteServer server("example.db");

        auto read_stmt = server.prepareQuery("SELECT * FROM users", false);
        if (read_stmt)
        {
            server.executeQuery(*read_stmt);
        }

        auto write_stmt = server.prepareQuery("INSERT INTO users (name, age) VALUES ('Alice', 30)", true);
        if (write_stmt)
        {
            server.executeQuery(*write_stmt);
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }

    return 0;
}