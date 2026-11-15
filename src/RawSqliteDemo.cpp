#include "RawSqliteDemo.h"

#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include <sqlite3.h>

#include "Database.h"
#include "ProjectRepository.h"

namespace
{
// ---------------------------------------------------------------- SQLite C API
class RawDb
{
public:
    explicit RawDb(const std::string& path)
    {
        if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK)
        {
            std::string msg = sqlite3_errmsg(db_);
            sqlite3_close(db_);
            throw std::runtime_error("open: " + msg);
        }
    }
    ~RawDb() { sqlite3_close(db_); }
    sqlite3* get() const { return db_; }

private:
    sqlite3* db_ = nullptr;
};

void rawCreateTable(sqlite3* db)
{
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS projects (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            language TEXT NOT NULL,
            repository_url TEXT NOT NULL,
            owner TEXT NOT NULL);)";
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK)
    {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("create: " + msg);
    }
}

int rawInsertProject(sqlite3* db, const Project& p)
{
    const char* sql =
        "INSERT INTO projects (name, language, repository_url, owner) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));

    sqlite3_bind_text(stmt, 1, p.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, p.language.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, p.repositoryUrl.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, p.owner.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    std::string msg = sqlite3_errmsg(db);
    sqlite3_finalize(stmt);                       // ручне звільнення ресурсу
    if (rc != SQLITE_DONE)
        throw std::runtime_error("insert: " + msg);
    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

std::optional<Project> rawGetProject(sqlite3* db, int id)
{
    const char* sql =
        "SELECT id, name, language, repository_url, owner FROM projects WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db));

    sqlite3_bind_int(stmt, 1, id);

    std::optional<Project> result;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        Project p;                                // ручне перетворення стовпців у типи C++
        p.id = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.language = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.repositoryUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        p.owner = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        result = p;
    }
    else if (rc != SQLITE_DONE)
    {
        std::string msg = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error("select: " + msg);
    }
    sqlite3_finalize(stmt);
    return result;
}

void print(const char* label, const std::optional<Project>& p)
{
    if (!p) { std::cout << label << " -> not found\n"; return; }
    std::cout << label << " -> {id=" << p->id << ", name=" << p->name
              << ", language=" << p->language << ", owner=" << p->owner << "}\n";
}
} // namespace

void runComparisonDemo()
{
    const std::string rawFile = "compare_raw.db";
    const std::string ormFile = "compare_orm.db";
    std::filesystem::remove(rawFile);
    std::filesystem::remove(ormFile);

    Project sample{0, "DemoService", "Go", "https://github.com/acme/demo-service", "Anna"};

    std::cout << "--- SQLite C API (prepared statements) ---\n";
    {
        RawDb db(rawFile);
        rawCreateTable(db.get());
        int id = rawInsertProject(db.get(), sample);
        std::cout << "Inserted project, id = " << id << '\n';
        print("get(id)", rawGetProject(db.get(), id));
        print("get(999)", rawGetProject(db.get(), 999));
    }

    std::cout << "--- sqlite_orm ---\n";
    {
        Storage storage = openStorage(ormFile);
        int id = storage.insert(sample);
        std::cout << "Inserted project, id = " << id << '\n';
        auto p = storage.get_optional<Project>(id);
        print("get(id)", p ? std::optional<Project>(*p) : std::nullopt);
        auto missing = storage.get_optional<Project>(999);
        print("get(999)", missing ? std::optional<Project>(*missing) : std::nullopt);
    }

    std::filesystem::remove(rawFile);
    std::filesystem::remove(ormFile);
}
