--- MiniSearch/README.md (原始)


+++ MiniSearch/README.md (修改后)
# MiniSearch - Локальный движок полнотекстового поиска на C++17

## 🎯 Описание проекта

**MiniSearch** — это легкий HTTP-сервер для полнотекстового поиска по набору документов. Проект демонстрирует знание современных возможностей C++17, работу со структурами данных (инвертированный индекс), многопоточность и алгоритмы ранжирования (TF-IDF).

## 🏗 Архитектура

Проект имеет модульную архитектуру:

```
MiniSearch/
├── include/
│   ├── text_parser.h      # Токенизация и нормализация текста
│   ├── document_store.h   # Работа с SQLite (хранение документов)
│   ├── search_engine.h    # Инвертированный индекс и TF-IDF
│   └── http_server.h      # HTTP-сервер для обработки запросов
├── src/
│   ├── main.cpp           # Точка входа
│   ├── text_parser.cpp
│   ├── document_store.cpp
│   ├── search_engine.cpp
│   └── http_server.cpp
├── data/                  # Тестовые документы
├── CMakeLists.txt
└── README.md
```

### Компоненты:

1. **TextParser** — обрабатывает текст: токенизация, нормализация, удаление стоп-слов
2. **DocumentStore** — хранит документы в SQLite для персистентности
3. **SearchEngine** — строит инвертированный индекс в памяти, реализует TF-IDF ранжирование
4. **HttpServer** — простой сервер на сокетах, обрабатывает GET-запросы

## 🔧 Требования

- Компилятор с поддержкой C++17 (GCC 8+, Clang 7+)
- CMake 3.10+
- SQLite3 (библиотека и заголовочные файлы)
- make или ninja

## 📦 Сборка и запуск

### 1. Клонирование и сборка

```bash
cd MiniSearch
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 2. Запуск сервера

```bash
./minisearch
```

Сервер запустится на порту **8080**.

### 3. Тестовые данные

При первом запуске автоматически загружаются 10 тестовых статей о технологиях (C++, Python, Linux, Алгоритмы).

## 🌐 API Endpoints

### Поиск документов

**GET** `/search?q=<запрос>`

Возвращает релевантные документы в формате JSON.

**Пример:**
```bash
curl "http://localhost:8080/search?q=C++ programming"
```

**Ответ:**
```json
{
  "query": "C++ programming",
  "results": [
    {
      "id": 1,
      "title": "Introduction to C++",
      "score": 2.456,
      "snippet": "C++ is a general-purpose programming language..."
    },
    {
      "id": 3,
      "title": "Modern C++ Features",
      "score": 1.823,
      "snippet": "C++17 introduced many new features..."
    }
  ],
  "total": 2,
  "time_ms": 0.45
}
```

### Статус сервера

**GET** `/status`

Возвращает статистику индекса.

**Пример:**
```bash
curl http://localhost:8080/status
```

**Ответ:**
```json
{
  "documents": 10,
  "unique_words": 523,
  "status": "ready"
}
```

## 🔍 Алгоритмы

### Инвертированный индекс

Для каждого слова хранится список документов, в которых оно встречается:
```
слово -> [(doc_id, tf), (doc_id, tf), ...]
```

Поиск работает за O(1) для каждого слова в запросе.

### TF-IDF ранжирование

**TF (Term Frequency)** — частота термина в документе:
```
TF(t, d) = количество вхождений t в d / общее количество слов в d
```

**IDF (Inverse Document Frequency)** — обратная частота документа:
```
IDF(t) = log(общее число документов / число документов содержащих t)
```

**Скор документа:**
```
Score(d, query) = Σ TF(t, d) * IDF(t) для всех t в запросе
```

## 📝 Примеры запросов

```bash
# Поиск по одному слову
curl "http://localhost:8080/search?q=python"

# Поиск по фразе (несколько слов)
curl "http://localhost:8080/search?q=machine learning"

# Поиск по технологическому термину
curl "http://localhost:8080/search?q=algorithm complexity"

# Проверка статуса
curl http://localhost:8080/status
```

## 🛠 Расширение

### Добавление своих документов

Поместите текстовые файлы (.txt) в папку `data/` и перезапустите сервер.

### Изменение порта

Отредактируйте константу `DEFAULT_PORT` в `src/main.cpp`.

## 📄 Лицензия

MIT License

## 👨‍💻 Автор

Проект создан для демонстрации навыков C++ разработки.