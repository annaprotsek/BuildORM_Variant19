#include "Database.h"

#include <iostream>

Storage openStorage(const std::string& path)
{
    Storage storage = makeStorage(path);
    // sqlite_orm сам вмикає PRAGMA foreign_keys = ON при відкритті з'єднання,
    // бо в схемі оголошено foreign_key(...); SQLite за замовчуванням їх не перевіряє.
    storage.open_forever();              // одне з'єднання на весь час життя: потрібно для changes()
    storage.sync_schema();               // створення/узгодження таблиць зі структурами C++
    return storage;
}

void clearDatabase(Storage& storage)
{
    storage.remove_all<Build>();         // спочатку дочірня таблиця (обмеження FK)
    storage.remove_all<Project>();
}

void seedDatabase(Storage& storage)
{
    if (storage.count<Project>() > 0)
        return;                          // повторний запуск не дублює дані

    struct Seed { const char* name; const char* lang; const char* url; const char* owner; };
    const Seed projects[] = {
        {"WebShop",       "TypeScript", "https://github.com/acme/webshop",       "Olena"},
        {"PaymentsAPI",   "Java",       "https://github.com/acme/payments-api",  "Taras"},
        {"MobileApp",     "Kotlin",     "https://github.com/acme/mobile-app",    "Iryna"},
        {"DataPipeline",  "Python",     "https://github.com/acme/data-pipeline", "Taras"},
        {"GameEngine",    "C++",        "https://github.com/acme/game-engine",   "Maksym"},
    };

    storage.transaction([&] {
        for (const auto& p : projects)
            storage.insert(Project{0, p.name, p.lang, p.url, p.owner});
        return true;
    });

    // projectId, number, branch, status, duration, startedAt
    struct B { int pid; int no; const char* br; const char* st; int dur; const char* at; };
    const B builds[] = {
        {1, 1, "main",    "success",   312, "2026-11-01 09:10:00"},
        {1, 2, "main",    "failed",     95, "2026-11-01 11:42:00"},
        {1, 3, "develop", "success",   340, "2026-11-02 08:05:00"},
        {1, 4, "main",    "success",   305, "2026-11-03 10:20:00"},
        {2, 1, "main",    "success",   610, "2026-11-01 12:00:00"},
        {2, 2, "feature", "failed",    220, "2026-11-02 14:30:00"},
        {2, 3, "main",    "failed",    180, "2026-11-03 09:15:00"},
        {2, 4, "main",    "success",   655, "2026-11-04 16:45:00"},
        {3, 1, "develop", "success",   480, "2026-11-01 13:00:00"},
        {3, 2, "develop", "cancelled",  40, "2026-11-02 10:00:00"},
        {3, 3, "main",    "success",   495, "2026-11-04 11:11:00"},
        {4, 1, "main",    "success",   150, "2026-11-01 07:30:00"},
        {4, 2, "main",    "success",   142, "2026-11-02 07:30:00"},
        {4, 3, "main",    "failed",     60, "2026-11-03 07:30:00"},
        {5, 1, "main",    "success",  1210, "2026-11-01 15:00:00"},
        {5, 2, "develop", "failed",   1105, "2026-11-02 15:20:00"},
        {5, 3, "main",    "success",  1180, "2026-11-03 15:10:00"},
        {5, 4, "main",    "cancelled",  75, "2026-11-04 15:00:00"},
    };

    storage.transaction([&] {
        for (const auto& b : builds)
            storage.insert(Build{0, b.pid, b.no, b.br, b.st, b.dur, b.at});
        return true;
    });
}
