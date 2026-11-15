#include "ProjectRepository.h"

int ProjectRepository::add(const Project& project)
{
    return storage_.insert(project);     // повертає згенерований id
}

std::optional<Project> ProjectRepository::getById(int id)
{
    auto result = storage_.get_optional<Project>(id);
    if (!result)
        return std::nullopt;             // відсутній запис - не виняток
    return *result;
}

std::vector<Project> ProjectRepository::getAll()
{
    return storage_.get_all<Project>();
}

bool ProjectRepository::update(const Project& project)
{
    storage_.update(project);
    return storage_.changes() > 0;       // false, якщо запису з таким id немає
}

bool ProjectRepository::remove(int id)
{
    storage_.remove<Project>(id);        // виняток, якщо є залежні збірки (FK)
    return storage_.changes() > 0;
}
