#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Database.h"

// Рядок результату JOIN: поля обох сутностей.
struct BuildReportRow
{
    std::string projectName;
    std::string language;
    int buildNumber = 0;
    std::string branch;
    std::string status;
    int durationSec = 0;
};

struct StatusStat
{
    std::string status;
    int count = 0;
    double avgDuration = 0.0;
};

// Окремий компонент доступу до залежної сутності Build (без абстрактного інтерфейсу).
class BuildDao
{
public:
    explicit BuildDao(Storage& storage) : storage_(storage) {}

    int add(const Build& build);
    std::optional<Build> getById(int id);
    std::vector<Build> getByProject(int projectId);

    // Завдання 5: запити
    std::vector<Build> findFailedOnBranch(const std::string& branch);
    std::vector<Build> longestBuilds(int limit);
    int countByStatus(const std::string& status);
    double averageDuration(int projectId);
    int totalDuration();
    std::vector<StatusStat> statsByStatus();
    std::vector<BuildReportRow> reportWithProjects(const std::string& status);

private:
    Storage& storage_;
};
