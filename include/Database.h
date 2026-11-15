#pragma once

#include <string>

#include <sqlite_orm/sqlite_orm.h>

#include "Build.h"
#include "Project.h"

// ORM-відображення: структури C++ <-> таблиці SQLite.
inline auto makeStorage(const std::string& path)
{
    using namespace sqlite_orm;
    return make_storage(
        path,
        make_table(
            "projects",
            make_column("id", &Project::id, primary_key().autoincrement()),
            make_column("name", &Project::name, unique()),
            make_column("language", &Project::language),
            make_column("repository_url", &Project::repositoryUrl),
            make_column("owner", &Project::owner)),
        make_table(
            "builds",
            make_column("id", &Build::id, primary_key().autoincrement()),
            make_column("project_id", &Build::projectId),
            make_column("build_number", &Build::buildNumber),
            make_column("branch", &Build::branch),
            make_column("status", &Build::status),
            make_column("duration_sec", &Build::durationSec, check(c(&Build::durationSec) >= 0)),
            make_column("started_at", &Build::startedAt),
            foreign_key(&Build::projectId).references(&Project::id),
            unique(&Build::projectId, &Build::buildNumber)));
}

using Storage = decltype(makeStorage(std::string{}));

// Відкриває сховище, вмикає перевірку зовнішніх ключів і синхронізує схему.
Storage openStorage(const std::string& path);

// Заповнює порожню базу початковими даними (5 проєктів, 18 збірок).
void seedDatabase(Storage& storage);

// Повністю очищає дані обох таблиць (дочірньої першою) для повторного запуску.
void clearDatabase(Storage& storage);
