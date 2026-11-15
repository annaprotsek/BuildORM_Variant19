#pragma once

#include <optional>
#include <vector>

#include "Project.h"

// Абстрактний інтерфейс доступу до основної сутності (патерн Repository).
class IProjectRepository
{
public:
    virtual ~IProjectRepository() = default;

    virtual int add(const Project& project) = 0;
    virtual std::optional<Project> getById(int id) = 0;
    virtual std::vector<Project> getAll() = 0;
    virtual bool update(const Project& project) = 0;
    virtual bool remove(int id) = 0;
};
