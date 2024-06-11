
#include <print>
#include <variant>
#include <utility>
#include <queue>
#include <optional>
#include <string>
#include <vector>
#include <thread>

#include <SDL2/SDL.h>

#include "sqlite3.h"

class Error
{
public:
    Error(
        const std::string &type,
        const std::optional<std::string> &message = std::nullopt)
        : m_type(type), m_message(message) {}

    Error(
        std::string &&type,
        std::optional<std::string> &&message)
        : m_type(std::move(type)), m_message(std::move(message)) {}

    Error(
        const Error &other)
        : m_type(other.m_type), m_message(other.m_message) {}

    Error &operator=(
        const Error &other)
    {
        if (this != &other)
        {
            m_type = other.m_type;
            m_message = other.m_message;
        }

        return *this;
    }

    ~Error() {}

    const std::string &type() const { return m_type; }

    const std::optional<std::string> &message() const { return m_message; }

private:
    std::string m_type;
    std::optional<std::string> m_message;
};

class Statement
{
public:
    Statement(
        const std::string &query,
        sqlite3_stmt *stmt)
        : m_query(query), m_stmt(stmt) {}

    Statement(
        std::string &&query,
        sqlite3_stmt *stmt)
        : m_query(std::move(query)), m_stmt(stmt) {}

    ~Statement()
    {
        if (m_stmt)
        {
            std::println("Finalizing statement: {}", m_query);

            sqlite3_finalize(m_stmt);
        }
    }

    const std::string &query() const { return m_query; }

    bool step()
    {
        return sqlite3_step(m_stmt) == SQLITE_ROW;
    }

    std::string_view column_text(int index)
    {
        return std::string_view(reinterpret_cast<const char *>(sqlite3_column_text(m_stmt, index)));
    }

    int column_int(int index)
    {
        return sqlite3_column_int(m_stmt, index);
    }

private:
    std::string m_query;
    sqlite3_stmt *m_stmt;
};

class Database
{
public:
    Database(
        std::string filename,
        sqlite3 *db)
        : m_filename(filename), m_db(db) {}

    ~Database()
    {
        if (m_db)
        {
            std::println("Closing database: {}", m_filename);

            sqlite3_close(m_db);

            m_db = nullptr;
        }
    }

    static std::variant<std::unique_ptr<Database>, Error> open(
        const std::string &filename)
    {
        sqlite3 *db;

        if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK)
        {
            return Error("sqlite", sqlite3_errmsg(db));
        }

        std::println("Opened database: {}", filename);

        return std::make_unique<Database>(filename, db);
    }

    std::variant<std::unique_ptr<Statement>, Error> prepare(
        const std::string &query)
    {
        sqlite3_stmt *stmt;

        const auto &prepare_result = sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr);

        if (prepare_result != SQLITE_OK)
        {
            return Error("sqlite", sqlite3_errmsg(m_db));
        }

        std::println("Prepared statement: {}", query);

        return std::make_unique<Statement>(query, stmt);
    }

    std::variant<std::unique_ptr<Statement>, Error> mutate_prepare(
        const std::string &query)
    {
        std::unique_lock<std::mutex> lock(m_mutation_mutex);

        return prepare(query);
    }

    std::optional<Error> ensure_table_structure()
    {
        std::vector<std::string> table_names;

        ///

        {
            const auto &stmt1OrError = prepare("PRAGMA table_list;");

            if (std::holds_alternative<Error>(stmt1OrError))
            {
                return std::get<Error>(stmt1OrError);
            }

            ///

            const auto &stmt1 = std::get<std::unique_ptr<Statement>>(stmt1OrError);

            while (stmt1->step())
            {
                const auto &schema = stmt1->column_text(0);

                const auto &name = stmt1->column_text(1);

                const auto &type = stmt1->column_text(2);

                ///

                if (name.starts_with("sqlite"))
                {
                    continue;
                }

                ///

                if (type == "table")
                {
                    table_names.push_back(std::string(name));
                }
            }
        }

        ///

        for (const auto &table : tables)
        {
            const auto &table_name = table.first;

            const auto &query = table.second;

            if (std::find(table_names.begin(), table_names.end(), table_name) == table_names.end())
            {
                const auto &create_table_stmt_or_error = mutate_prepare(query);

                if (std::holds_alternative<Error>(create_table_stmt_or_error))
                {
                    std::println("Failed to prepare create table statement: {}", std::get<Error>(create_table_stmt_or_error).message().value());

                    return std::get<Error>(create_table_stmt_or_error);
                }

                ///

                const auto &create_table_stmt = std::get<std::unique_ptr<Statement>>(create_table_stmt_or_error);

                create_table_stmt->step();
            }
            else
            {
                std::println("Table already exists: {}", table_name);
            }
        }

        ///

        return std::nullopt;
    }

private:
    static constexpr std::array<std::pair<const char *, const char *>, 3> tables =
        {{{"table1", "CREATE TABLE table1 (id INTEGER PRIMARY KEY, name TEXT);"},
          {"table2", "CREATE TABLE table2 (id INTEGER PRIMARY KEY, name TEXT);"},
          {"table3", "CREATE TABLE table3 (id INTEGER PRIMARY KEY, name TEXT);"}}};

    std::string m_filename;
    sqlite3 *m_db;
    std::mutex m_mutation_mutex;
};

class Simulation
{
public:
    explicit Simulation(
        std::reference_wrapper<Database> &&database,
        size_t num_threads)
        : m_database(database), m_stop(false), m_num_threads(num_threads)
    {
        std::println("Creating threads");

        for (size_t i = 0; i < num_threads; ++i)
        {
            m_threads.emplace_back(&Simulation::simulate, this, i);
        }
    }

    ~Simulation()
    {
        {
            std::unique_lock<std::mutex> lock(m_id_queue_mutex);

            m_stop = true;
        }

        m_id_queue_cond_var.notify_all();

        for (auto &thread : m_threads)
        {
            thread.join();
        }

        std::println("Ending threads");
    }

    void enqueue_id(int id)
    {
        {
            std::unique_lock<std::mutex> lock(m_id_queue_mutex);

            m_id_queue.emplace(std::move(id));
        }

        m_id_queue_cond_var.notify_one();
    }


    void enqueue_ids(const std::vector<int> &ids)
    {
        {
            std::unique_lock<std::mutex> lock(m_id_queue_mutex);

            for (const auto &id : ids)
            {
                m_id_queue.emplace(id);
            }
        }

        if (ids.size() < m_num_threads)
        {
            for (int i = 0; i < ids.size(); ++i)
            {
                m_id_queue_cond_var.notify_one();
            }
        }
        else
        {
            m_id_queue_cond_var.notify_all();
        }
    }

    void simulate(
        int n)
    {
        while (!m_stop)
        {
            const auto &start = std::chrono::steady_clock::now();

            ///

            int id;

            {
                std::unique_lock<std::mutex> lock(m_id_queue_mutex);

                m_id_queue_cond_var.wait(lock, [this]()
                                         { return m_stop || !m_id_queue.empty(); });

                if (m_stop && m_id_queue.empty())
                {
                    return;
                }

                id = std::move(m_id_queue.front());

                m_id_queue.pop();
            }

            const auto &stop = std::chrono::steady_clock::now();

            const auto &duration_us = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();

            std::println("Thread #{} simulating id: {}, {}us", n, id, duration_us);
        }
    }

private:
    std::reference_wrapper<Database> m_database;
    std::vector<std::thread> m_threads;
    std::queue<int> m_id_queue;
    std::mutex m_id_queue_mutex;
    std::condition_variable m_id_queue_cond_var;
    std::atomic<bool> m_stop;
    int m_num_threads;
};

int main()
{
    const auto &start = std::chrono::steady_clock::now();

    ///

    const auto &dbOrError = Database::open("/Users/chris/foo.db");

    if (std::holds_alternative<Error>(dbOrError))
    {
        const auto &error = std::get<Error>(dbOrError);

        std::println("Failed to open database: {}", error.message().value());

        return 1;
    }

    const auto &db = std::get<std::unique_ptr<Database>>(dbOrError);

    ///

    db->ensure_table_structure();

    ///

    // std::reference_wrapper<Database> db_ref = *db;

    {
        int thread_count = 4;

        Simulation sim(*db, thread_count);

        int id = 0;

        while (std::chrono::steady_clock::now() - start < std::chrono::seconds(1))
        {
            std::vector<int> ids;

            for (int i = 0; i < 1024; ++i)
            {
                ids.push_back(id++);
            }

            sim.enqueue_ids(ids);

            // for (int i = 0; i < thread_count; ++i)
            // {
            //     sim.enqueue_id(id++);
            // }

            // std::this_thread::sleep_for(std::chrono::microseconds(1));

            // std::println("");
        }
    }

    ///

    std::println("");

    ///

    const auto &stop = std::chrono::steady_clock::now();

    ///

    const auto &duration = stop - start;

    const auto &durationNano = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    const auto &durationMicro = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    const auto &durationMilli = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    const auto &durationSec = std::chrono::duration_cast<std::chrono::seconds>(duration).count();

    ///

    std::println("Duration: {} ns", durationNano);

    std::println("Duration: {} us", durationMicro);

    std::println("Duration: {} ms", durationMilli);

    std::println("Duration: {} s", durationSec);

    return 0;
}