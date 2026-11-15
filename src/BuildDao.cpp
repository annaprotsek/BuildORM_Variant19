#include "BuildDao.h"

using namespace sqlite_orm;

int BuildDao::add(const Build& build)
{
    return storage_.insert(build);
}

std::optional<Build> BuildDao::getById(int id)
{
    auto r = storage_.get_optional<Build>(id);
    if (!r) return std::nullopt;
    return *r;
}

std::vector<Build> BuildDao::getByProject(int projectId)
{
    return storage_.get_all<Build>(
        where(c(&Build::projectId) == projectId),
        order_by(&Build::buildNumber));
}

// WHERE status = 'failed' AND branch = ?
std::vector<Build> BuildDao::findFailedOnBranch(const std::string& branch)
{
    return storage_.get_all<Build>(
        where(c(&Build::status) == "failed" && c(&Build::branch) == branch));
}

// ORDER BY duration_sec DESC LIMIT ?
std::vector<Build> BuildDao::longestBuilds(int maxCount)
{
    return storage_.get_all<Build>(
        order_by(&Build::durationSec).desc(),
        limit(maxCount));
}

// SELECT COUNT(*) FROM builds WHERE status = ?
int BuildDao::countByStatus(const std::string& status)
{
    return storage_.count<Build>(where(c(&Build::status) == status));
}

// SELECT AVG(duration_sec) FROM builds WHERE project_id = ?
double BuildDao::averageDuration(int projectId)
{
    double v = storage_.avg(&Build::durationSec, where(c(&Build::projectId) == projectId));
    return v;
}

// SELECT SUM(duration_sec) FROM builds
int BuildDao::totalDuration()
{
    auto v = storage_.sum(&Build::durationSec);
    return v ? static_cast<int>(*v) : 0;
}

// SELECT status, COUNT(*), AVG(duration_sec) FROM builds GROUP BY status
std::vector<StatusStat> BuildDao::statsByStatus()
{
    auto rows = storage_.select(
        columns(&Build::status, count<Build>(), avg(&Build::durationSec)),
        group_by(&Build::status),
        order_by(&Build::status));
    std::vector<StatusStat> out;
    for (auto& [status, cnt, average] : rows)
        out.push_back({status, cnt, average});
    return out;
}

// SELECT ... FROM builds INNER JOIN projects ON builds.project_id = projects.id
std::vector<BuildReportRow> BuildDao::reportWithProjects(const std::string& status)
{
    auto rows = storage_.select(
        columns(&Project::name, &Project::language, &Build::buildNumber,
                &Build::branch, &Build::status, &Build::durationSec),
        inner_join<Project>(on(c(&Build::projectId) == &Project::id)),
        where(c(&Build::status) == status),
        order_by(&Project::name));
    std::vector<BuildReportRow> out;
    for (auto& [name, lang, no, br, st, dur] : rows)
        out.push_back({name, lang, no, br, st, dur});
    return out;
}
