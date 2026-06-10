cmake_minimum_required(VERSION 3.10)
project(MiniSearch LANGUAGES CXX)

# Устанавливаем стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Находим SQLite3
find_package(SQLite3 REQUIRED)

# Находим потоки (threads) для std::thread
find_package(Threads REQUIRED)

# Список исходных файлов проекта (файлы лежат в корне, без папки src/)
set(SOURCES
    main.cpp
    search_engine.cpp
    document_store.cpp
    text_parser.cpp
    http_server.cpp
)

# Создаем исполняемый файл
add_executable(mini_search ${SOURCES})

# Линкуем SQLite3 и потоки
target_link_libraries(mini_search PRIVATE SQLite::SQLite3 Threads::Threads)

# Включаем директорию заголовков SQLite
target_include_directories(mini_search PRIVATE ${SQLite3_INCLUDE_DIRS})

# Выводим информацию для отладки
message(STATUS "=== MiniSearch Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "SQLite3 Include: ${SQLite3_INCLUDE_DIRS}")
message(STATUS "SQLite3 Library: ${SQLite3_LIBRARIES}")
message(STATUS "================================")
