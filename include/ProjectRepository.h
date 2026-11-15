#pragma once

#include "Database.h"
#include "IProjectRepository.h"

// Реалізація репозиторію засобами sqlite_orm.
class ProjectRepository : public IProjectRepository
{
public:
    explicit ProjectRepository(Storage& storage) : storage_(storage) {}

    int add(const Project& project) override;
    std::optional<Project> getById(int id) override;
    std::vector<Project> getAll() override;
    bool update(const Project& project) override;
    bool remove(int id) override;

private:
    Storage& storage_;
};
