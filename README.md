# BuildORM — ЛР №8, варіант 19 (облік CI/CD: Проєкт → Збірка)

Консольний застосунок C++20: SQLite + [sqlite_orm](https://github.com/fnc12/sqlite_orm), залежності через vcpkg (manifest), складання CMake.

## Модель даних
- `projects` (id PK, name UNIQUE, language, repository_url, owner)
- `builds` (id PK, project_id FK → projects.id, build_number, branch, status, duration_sec CHECK ≥ 0, started_at, UNIQUE(project_id, build_number))
- Зв'язок 1:N — один проєкт має багато збірок.

## Структура
| Файл | Призначення |
|---|---|
| `include/Project.h`, `Build.h` | моделі (звичайні структури C++) |
| `include/Database.h`, `src/Database.cpp` | ORM-відображення, `sync_schema()`, початкові дані |
| `include/IProjectRepository.h`, `ProjectRepository.*` | Repository (інтерфейс + реалізація на sqlite_orm) |
| `include/BuildDao.h`, `src/BuildDao.cpp` | доступ до залежної сутності, запити (фільтр, сортування, агрегати, JOIN) |
| `src/RawSqliteDemo.cpp` | порівняння: SQLite C API vs sqlite_orm |
| `src/Scenarios.cpp` | демонстраційні сценарії, транзакції |
| `src/main.cpp` | точка входу |

## Збирання (Windows, Visual Studio 2022/2026 або Build Tools)
Потрібні: CMake ≥ 3.21, Ninja, vcpkg (за замовчуванням `C:\vcpkg`).

```powershell
cmake --preset default          # vcpkg сам встановить sqlite3 та sqlite-orm
cmake --build build
.\build\BuildORM.exe
```

Якщо vcpkg встановлено в інше місце — змініть `CMAKE_TOOLCHAIN_FILE` у `CMakePresets.json`
(або створіть `CMakeUserPresets.json`). Для запуску з консолі виконуйте команди в
«x64 Native Tools Command Prompt / Developer PowerShell for VS».

## Повторні запуски
Файл `build_orm.db` видаляється та створюється заново при кожному запуску — дані не накопичуються.
