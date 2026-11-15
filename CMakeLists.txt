cmake_minimum_required(VERSION 3.21)

project(BuildORM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8 /W4)
endif()

find_package(SqliteOrm CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

add_executable(BuildORM
    src/main.cpp
    src/Database.cpp
    src/ProjectRepository.cpp
    src/BuildDao.cpp
    src/RawSqliteDemo.cpp
    src/Scenarios.cpp
)

target_include_directories(BuildORM
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(BuildORM
    PRIVATE
        sqlite_orm::sqlite_orm
        unofficial::sqlite3::sqlite3
)
