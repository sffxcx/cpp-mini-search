--- MiniSearch/include/search_engine.h (原始)


+++ MiniSearch/include/search_engine.h (修改后)
#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include "document_store.h"
#include "text_parser.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>

namespace minisearch {

/**
 * Результат поиска
 */
struct SearchResult {
    int docId;
    std::string title;
    std::string snippet;
    double score;

    SearchResult() : docId(0), score(0.0) {}
    SearchResult(int id, const std::string& t, const std::string& s, double sc)
        : docId(id), title(t), snippet(s), score(sc) {}
};

/**
 * Entry в инвертированном индексе
 * Хранит ID документа и частоту термина в нем
 */
struct IndexEntry {
    int docId;
    int termFrequency;

    IndexEntry() : docId(0), termFrequency(0) {}
    IndexEntry(int id, int tf) : docId(id), termFrequency(tf) {}
};

/**
 * SearchEngine - ядро поисковой системы
 *
 * Отвечает за:
 * - Построение инвертированного индекса (Inverted Index)
 * - Расчет TF-IDF для ранжирования результатов
 * - Поиск документов по запросу
 */
class SearchEngine {
public:
    SearchEngine();

    /**
     * Индексирует документы из хранилища
     * @param store Ссылка на DocumentStore
     */
    void indexDocuments(DocumentStore& store);

    /**
     * Выполняет поиск по запросу
     * @param query Текст запроса
     * @param maxResults Максимальное количество результатов
     * @return Отсортированный вектор результатов поиска
     */
    std::vector<SearchResult> search(const std::string& query, size_t maxResults = 10);

    /**
     * Возвращает количество уникальных слов в индексе
     * @return Количество слов
     */
    size_t getUniqueWordCount() const;

    /**
     * Возвращает количество проиндексированных документов
     * @return Количество документов
     */
    size_t getDocumentCount() const;

    /**
     * Проверяет, проиндексированы ли документы
     * @return true если индекс не пуст
     */
    bool isIndexed() const;

private:
    // Инвертированный индекс: слово -> список вхождений в документах
    std::unordered_map<std::string, std::vector<IndexEntry>> invertedIndex_;

    // Количество документов, содержащих каждое слово (для IDF)
    std::unordered_map<std::string, int> documentFrequency_;

    // Общее количество проиндексированных документов
    size_t totalDocuments_;

    // Длина каждого документа (количество слов)
    std::unordered_map<int, size_t> documentLengths_;

    // Кэш содержимого документов для создания сниппетов
    std::unordered_map<int, std::string> documentContents_;

    // Парсер текста
    TextParser parser_;

    /**
     * Рассчитывает TF (Term Frequency) для слова в документе
     * @param term Слово
     * @param docId ID документа
     * @return TF значение
     */
    double calculateTF(const std::string& term, int docId) const;

    /**
     * Рассчитывает IDF (Inverse Document Frequency) для слова
     * @param term Слово
     * @return IDF значение
     */
    double calculateIDF(const std::string& term) const;

    /**
     * Создает сниппет из содержимого документа с подсветкой запроса
     * @param content Содержимое документа
     * @param queryWords Слова из запроса
     * @param snippetLength Длина сниппета
     * @return Сниппет с контекстом вокруг найденных слов
     */
    std::string createSnippet(const std::string& content,
                              const std::vector<std::string>& queryWords,
                              size_t snippetLength = 150);
};

} // namespace minisearch

#endif // SEARCH_ENGINE_H