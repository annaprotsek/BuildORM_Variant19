#pragma once

#include <string>

// Основна (батьківська) сутність: програмний проєкт, для якого виконуються збірки CI/CD.
struct Project
{
    int id = 0;
    std::string name;
    std::string language;
    std::string repositoryUrl;
    std::string owner;
};
