--- MiniSearch/src/document_store.cpp (原始)


+++ MiniSearch/src/document_store.cpp (修改后)
#include "document_store.h"
#include <iostream>
#include <sstream>

namespace minisearch {

DocumentStore::DocumentStore() : db_(nullptr) {}

DocumentStore::~DocumentStore() {
    close();
}

bool DocumentStore::initialize(const std::string& dbPath) {
    dbPath_ = dbPath;

    int rc = sqlite3_open(dbPath.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка открытия базы данных: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    return createTable();
}

bool DocumentStore::createTable() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS documents (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            content TEXT NOT NULL,
            path TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
        CREATE INDEX IF NOT EXISTS idx_title ON documents(title);
    )";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка создания таблицы: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

int DocumentStore::addDocument(const std::string& title, const std::string& content,
                               const std::string& path) {
    if (!db_) {
        return -1;
    }

    // Проверяем, существует ли документ с таким заголовком
    if (documentExists(title)) {
        // Возвращаем ID существующего документа
        const char* selectSql = "SELECT id FROM documents WHERE title = ?";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db_, selectSql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                sqlite3_finalize(stmt);
                return id;
            }
            sqlite3_finalize(stmt);
        }
        return -1;
    }

    const char* sql = "INSERT INTO documents (title, content, path) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db_) << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, path.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        std::cerr << "Ошибка вставки документа: " << sqlite3_errmsg(db_) << std::endl;
        return -1;
    }

    // Получаем ID последнего вставленного документа
    return static_cast<int>(sqlite3_last_insert_rowid(db_));
}

std::optional<Document> DocumentStore::getDocumentById(int id) const {
    if (!db_) {
        return std::nullopt;
    }

    const char* sql = "SELECT id, title, content, path FROM documents WHERE id = ?";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    Document doc;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        doc.id = sqlite3_column_int(stmt, 0);
        doc.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        doc.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        doc.path = path ? path : "";

        sqlite3_finalize(stmt);
        return doc;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Document> DocumentStore::getAllDocuments() const {
    std::vector<Document> documents;

    if (!db_) {
        return documents;
    }

    const char* sql = "SELECT id, title, content, path FROM documents ORDER BY id";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return documents;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Document doc;
        doc.id = sqlite3_column_int(stmt, 0);
        doc.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        doc.content = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        doc.path = path ? path : "";

        documents.push_back(std::move(doc));
    }

    sqlite3_finalize(stmt);
    return documents;
}

bool DocumentStore::documentExists(const std::string& title) const {
    if (!db_) {
        return false;
    }

    const char* sql = "SELECT COUNT(*) FROM documents WHERE title = ?";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        exists = count > 0;
    }

    sqlite3_finalize(stmt);
    return exists;
}

size_t DocumentStore::getDocumentCount() const {
    if (!db_) {
        return 0;
    }

    const char* sql = "SELECT COUNT(*) FROM documents";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    size_t count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = static_cast<size_t>(sqlite3_column_int(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return count;
}

void DocumentStore::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

} // namespace minisearch