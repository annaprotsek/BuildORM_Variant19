#include <filesystem>
#include <iostream>
#include <system_error>
#include <sqlite3.h>

#include "Database.h"
#include "RawSqliteDemo.h"
#include "Scenarios.h"

int main()
{
    const std::string dbFile = "build_orm.db";
    try
    {
        std::cout << "BuildORM: CI/CD build tracking (SQLite + sqlite_orm)\n";
        std::cout << "SQLite version: " << sqlite3_libversion() << "\n\n";

        std::cout << "##### Task 2: SQLite C API vs sqlite_orm #####\n";
        runComparisonDemo();

        // Нова база при кожному запуску: дані не накопичуються.
        std::filesystem::remove(dbFile);
        Storage storage = openStorage(dbFile);

        demoSchemaAndSeed(storage);
        demoRepository(storage);
        demoQueries(storage);
        demoTransactions(storage);

        std::cout << "\nDone.\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
}
