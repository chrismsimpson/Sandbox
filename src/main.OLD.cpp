#include <print>

#include "Foo.h"
#include "sqlite3.h"

void foo()
{
    sqlite3 *db;

    const auto &createDbResult = sqlite3_open("/Users/chris/foo.db", &db);
}

int main()
{
    const auto &start = std::chrono::steady_clock::now();


    ///

    sqlite3 *db;

    const auto &openDbResult = sqlite3_open("/Users/chris/foo2.db", &db);

    if (openDbResult != SQLITE_OK)
    {
        std::println("Failed to open database: {}", sqlite3_errmsg(db));

        sqlite3_close(db);

        return 1;
    }

    ///

    sqlite3_stmt *stmt;

    const auto &prepareStmtResult = sqlite3_prepare_v2(db, "PRAGMA table_list;", -1, &stmt, nullptr);

    if (prepareStmtResult != SQLITE_OK)
    {
        std::println("Failed to prepare statement: {}", sqlite3_errmsg(db));

        sqlite3_finalize(stmt);

        sqlite3_close(db);

        return 1;
    }

    ///

    

    ///

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const std::string_view schema(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));

        const std::string_view name(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)));

        const std::string_view type(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)));

        const auto nCol = sqlite3_column_int(stmt, 3);

        const auto wr = sqlite3_column_int(stmt, 4) > 0 
            ? true 
            : false;

        const auto strict = sqlite3_column_int(stmt, 5) > 0 
            ? true 
            : false;

        ///

        std::println("schema: {}, name: {}, type: {}, nCol: {}, wr: {}, strict: {}", schema, name, type, nCol, wr, strict);


    }

    ///

    sqlite3_finalize(stmt);

    ///







    // std::println("Hello, World!");

    ///



    sqlite3_close(db);

    ///

    const auto &stop = std::chrono::steady_clock::now();

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



    ///

    return 0;
}