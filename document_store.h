#ifndef DOCUMENT_STORE_H
#define DOCUMENT_STORE_H

#include <string>
#include <vector>
#include <optional>
#include <sqlite3.h>

namespace minisearch {

/**
 * Структура документа
 */
struct Document {
    int id;
    std::string title;
    std::string content;
    std::string path;

    Document() : id(0) {}
    Document(int id_, const std::string& title_, const std::string& content_,
             const std::string& path_)
        : id(id_), title(title_), content(content_), path(path_) {}
};

/**
 * DocumentStore - класс для работы с SQLite базой данных
 *
 * Отвечает за:
 * - Хранение документов в таблице documents
 * - Добавление и получение документов по ID
 * - Персистентное хранение метаданных
 */
class DocumentStore {
public:
    DocumentStore();
    ~DocumentStore();

    /**
     * Инициализирует базу данных
     * @param dbPath Путь к файлу базы данных
     * @return true если успешно
     */
    bool initialize(const std::string& dbPath);

    /**
     * Добавляет документ в базу
     * @param title Заголовок документа
     * @param content Содержимое документа
     * @param path Путь к файлу (опционально)
     * @return ID добавленного документа или -1 при ошибке
     */
    int addDocument(const std::string& title, const std::string& content,
                    const std::string& path = "");

    /**
     * Получает документ по ID
     * @param id ID документа
     * @return Document если найден, std::nullopt если нет
     */
    std::optional<Document> getDocumentById(int id) const;

    /**
     * Получает все документы из базы
     * @return Вектор всех документов
     */
    std::vector<Document> getAllDocuments() const;

    /**
     * Проверяет, существует ли документ с таким заголовком
     * @param title Заголовок для проверки
     * @return true если документ существует
     */
    bool documentExists(const std::string& title) const;

    /**
     * Возвращает количество документов в базе
     * @return Количество документов
     */
    size_t getDocumentCount() const;

    /**
     * Закрывает соединение с базой
     */
    void close();

private:
    sqlite3* db_;
    std::string dbPath_;

    /**
     * Создает таблицу documents если она не существует
     * @return true если успешно
     */
    bool createTable();
};

} // namespace minisearch

#endif // DOCUMENT_STORE_H
