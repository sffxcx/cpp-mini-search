cmake_minimum_required(VERSION 3.10)
project(MiniSearch VERSION 1.0.0 LANGUAGES CXX)

# Стандарт C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Компиляторные флаги
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Поиск SQLite3
find_package(SQLite3 REQUIRED)

# Включаем директорию с заголовками
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${SQLite3_INCLUDE_DIRS})

# Исходные файлы проекта
set(SOURCES
    src/main.cpp
    src/text_parser.cpp
    src/document_store.cpp
    src/search_engine.cpp
    src/http_server.cpp
)

# Заголовочные файлы
set(HEADERS
    include/text_parser.h
    include/document_store.h
    include/search_engine.h
    include/http_server.h
)

# Создаем исполняемый файл
add_executable(minisearch ${SOURCES} ${HEADERS})

# Линкуем библиотеки
target_link_libraries(minisearch
    PRIVATE SQLite::SQLite3
    PRIVATE pthread
)

# Установка
install(TARGETS minisearch DESTINATION bin)

# Копирование базы данных и тестовых файлов при сборке
file(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# Вывод информации о конфигурации
message(STATUS "=== MiniSearch Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "SQLite3 Include: ${SQLite3_INCLUDE_DIRS}")
message(STATUS "SQLite3 Library: ${SQLite3_LIBRARIES}")
message(STATUS "=================================")
