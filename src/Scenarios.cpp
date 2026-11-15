#include "Scenarios.h"

#include <iomanip>
#include <iostream>
#include <system_error>

#include "BuildDao.h"
#include "IProjectRepository.h"
#include "ProjectRepository.h"

using namespace sqlite_orm;

namespace
{
void printProject(const Project& p)
{
    std::cout << "  #" << p.id << " " << std::left << std::setw(13) << p.name
              << std::setw(11) << p.language << p.owner << '\n';
}

void printBuild(const Build& b)
{
    std::cout << "  build #" << b.id << " project=" << b.projectId << " no=" << b.buildNumber
              << " " << std::left << std::setw(8) << b.branch << std::setw(10) << b.status
              << b.durationSec << " s\n";
}

void printCounts(Storage& s, const char* label)
{
    std::cout << "  [" << label << "] projects=" << s.count<Project>()
              << ", builds=" << s.count<Build>() << '\n';
}
} // namespace

void printTables(Storage& storage, const char* title)
{
    std::cout << "== " << title << " ==\n";
    for (const auto& t : storage.table_names())
        std::cout << "  table: " << t << '\n';
    printCounts(storage, "rows");
}

// -------------------------------------------------------- Завдання 3
void demoSchemaAndSeed(Storage& storage)
{
    std::cout << "\n##### Task 3: schema, foreign keys, seed data #####\n";
    std::cout << "sync_schema() result:\n";
    for (const auto& [name, result] : storage.sync_schema(true))
        std::cout << "  " << name << " -> " << result << '\n';   // sync_schema(true) = перевірка без змін

    seedDatabase(storage);
    printTables(storage, "After seeding");


    std::cout << "Attempt to insert a build for non-existing project 999:\n";
    try
    {
        storage.insert(Build{0, 999, 1, "main", "success", 10, "2026-11-05 10:00:00"});
        std::cout << "  inserted (unexpected!)\n";
    }
    catch (const std::system_error& e)
    {
        std::cout << "  rejected: " << e.what() << '\n';
    }
    printCounts(storage, "after failed insert");
}

// -------------------------------------------------------- Завдання 4
void demoRepository(Storage& storage)
{
    std::cout << "\n##### Task 4: Repository (used via IProjectRepository) #####\n";
    ProjectRepository impl(storage);
    IProjectRepository& repo = impl;               // робота через інтерфейс базового класу

    int id = repo.add({0, "DocsPortal", "Ruby", "https://github.com/acme/docs-portal", "Anna"});
    std::cout << "CREATE: added project id=" << id << '\n';

    if (auto p = repo.getById(id))
    {
        std::cout << "READ:   found:\n";
        printProject(*p);

        p->language = "Rust";
        p->owner = "Oksana";
        std::cout << "UPDATE: " << (repo.update(*p) ? "ok" : "no such record") << '\n';
        printProject(*repo.getById(id));
    }

    std::cout << "READ missing id=999: "
              << (repo.getById(999) ? "found" : "std::nullopt (not found)") << '\n';
    Project ghost{999, "Ghost", "-", "-", "-"};
    std::cout << "UPDATE missing id=999: " << (repo.update(ghost) ? "ok" : "false (nothing changed)") << '\n';
    std::cout << "DELETE missing id=999: " << (repo.remove(999) ? "ok" : "false (nothing deleted)") << '\n';

    std::cout << "READ ALL (" << repo.getAll().size() << " projects):\n";
    for (const auto& p : repo.getAll())
        printProject(p);

    std::cout << "DELETE project 1 (has builds -> FK must refuse):\n";
    try
    {
        repo.remove(1);
        std::cout << "  deleted (unexpected!)\n";
    }
    catch (const std::system_error& e)
    {
        std::cout << "  rejected: " << e.what() << '\n';
    }

    std::cout << "DELETE project id=" << id << " (no builds): "
              << (repo.remove(id) ? "ok" : "false") << '\n';
    std::cout << "READ deleted id: " << (repo.getById(id) ? "found" : "not found") << '\n';
}

// -------------------------------------------------------- Завдання 5
void demoQueries(Storage& storage)
{
    std::cout << "\n##### Task 5: queries #####\n";
    BuildDao dao(storage);

    std::cout << "1) Filter: failed builds on branch 'main'\n";
    for (const auto& b : dao.findFailedOnBranch("main"))
        printBuild(b);

    std::cout << "2) Sort + limit: 3 longest builds\n";
    for (const auto& b : dao.longestBuilds(3))
        printBuild(b);

    std::cout << "3) Aggregates:\n";
    std::cout << "  COUNT failed builds      = " << dao.countByStatus("failed") << '\n';
    std::cout << "  SUM of all durations     = " << dao.totalDuration() << " s\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  AVG duration, project 1  = " << dao.averageDuration(1) << " s\n";
    std::cout << "  GROUP BY status:\n";
    for (const auto& s : dao.statsByStatus())
        std::cout << "    " << std::left << std::setw(10) << s.status << " count=" << s.count
                  << " avg=" << s.avgDuration << " s\n";

    std::cout << "4) INNER JOIN: failed builds with project data\n";
    for (const auto& r : dao.reportWithProjects("failed"))
        std::cout << "  " << std::left << std::setw(13) << r.projectName << std::setw(11) << r.language
                  << "build " << r.buildNumber << " " << std::setw(8) << r.branch << r.durationSec << " s\n";
}

// -------------------------------------------------------- Завдання 6
namespace
{
// Логічна дія: зареєструвати проєкт разом із першою збіркою.
void registerProjectWithFirstBuild(Storage& storage, const Project& project, Build firstBuild, bool useTransaction)
{
    auto work = [&] {
        int pid = storage.insert(project);        // операція 1
        firstBuild.projectId = pid;
        storage.insert(firstBuild);               // операція 2
        return true;
    };

    if (useTransaction)
        storage.transaction(work);                // COMMIT, а при винятку - ROLLBACK
    else
        work();
}
} // namespace

void demoTransactions(Storage& storage)
{
    std::cout << "\n##### Task 6: transactions #####\n";

    std::cout << "\nScenario A: success (COMMIT)\n";
    printCounts(storage, "before");
    registerProjectWithFirstBuild(
        storage, {0, "NewService", "Go", "https://github.com/acme/new-service", "Anna"},
        {0, 0, 1, "main", "success", 120, "2026-11-05 09:00:00"}, true);
    printCounts(storage, "after ");

    std::cout << "\nScenario B: second operation violates CHECK(duration_sec >= 0) -> ROLLBACK\n";
    printCounts(storage, "before");
    try
    {
        registerProjectWithFirstBuild(
            storage, {0, "BrokenService", "Go", "https://github.com/acme/broken", "Anna"},
            {0, 0, 1, "main", "success", -5, "2026-11-05 09:30:00"}, true);
    }
    catch (const std::system_error& e)
    {
        std::cout << "  exception: " << e.what() << '\n';
    }
    printCounts(storage, "after ");
    std::cout << "  project 'BrokenService' present: "
              << (storage.count<Project>(where(c(&Project::name) == "BrokenService")) ? "YES (error!)" : "no (rolled back)") << '\n';

    std::cout << "\nScenario C: same failure WITHOUT explicit transaction\n";
    printCounts(storage, "before");
    try
    {
        registerProjectWithFirstBuild(
            storage, {0, "PartialService", "Go", "https://github.com/acme/partial", "Anna"},
            {0, 0, 1, "main", "success", -5, "2026-11-05 09:45:00"}, false);
    }
    catch (const std::system_error& e)
    {
        std::cout << "  exception: " << e.what() << '\n';
    }
    printCounts(storage, "after ");
    std::cout << "  project 'PartialService' present: "
              << (storage.count<Project>(where(c(&Project::name) == "PartialService")) ? "YES (partial save!)" : "no") << '\n';
    std::cout << "  builds of that project: "
              << storage.count<Build>(where(in(&Build::projectId,
                     select(&Project::id, where(c(&Project::name) == "PartialService"))))) << '\n';
}
