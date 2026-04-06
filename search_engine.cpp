#include "search_engine.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>

namespace minisearch {

SearchEngine::SearchEngine() : totalDocuments_(0) {}

void SearchEngine::indexDocuments(DocumentStore& store) {
    // Очищаем существующий индекс
    invertedIndex_.clear();
    documentFrequency_.clear();
    documentLengths_.clear();
    documentContents_.clear();
    totalDocuments_ = 0;

    // Получаем все документы из хранилища
    auto documents = store.getAllDocuments();
    totalDocuments_ = documents.size();

    std::cout << "[SearchEngine] Индексация " << totalDocuments_ << " документов..." << std::endl;

    // Обрабатываем каждый документ
    for (const auto& doc : documents) {
        // Сохраняем содержимое для сниппетов
        documentContents_[doc.id] = doc.content;

        // Токенизируем и нормализуем содержимое
        auto words = parser_.processText(doc.content + " " + doc.title);

        // Сохраняем длину документа (количество слов)
        documentLengths_[doc.id] = words.size();

        // Подсчитываем частоту каждого слова в документе
        std::unordered_map<std::string, int> termFreq;
        std::unordered_set<std::string> uniqueWordsInDoc;

        for (const auto& word : words) {
            termFreq[word]++;
            uniqueWordsInDoc.insert(word);
        }

        // Добавляем слова в инвертированный индекс
        for (const auto& [word, freq] : termFreq) {
            // Проверяем, есть ли уже запись для этого слова и документа
            auto& entries = invertedIndex_[word];
            bool found = false;
            for (auto& entry : entries) {
                if (entry.docId == doc.id) {
                    entry.termFrequency = freq;
                    found = true;
                    break;
                }
            }
            if (!found) {
                entries.emplace_back(doc.id, freq);
            }

            // Увеличиваем количество документов, содержащих это слово
            documentFrequency_[word]++;
        }
    }

    std::cout << "[SearchEngine] Индексация завершена. Уникальных слов: "
              << getUniqueWordCount() << std::endl;
}

std::vector<SearchResult> SearchEngine::search(const std::string& query, size_t maxResults) {
    std::vector<SearchResult> results;

    if (query.empty() || totalDocuments_ == 0) {
        return results;
    }

    // Токенизируем запрос
    auto queryWords = parser_.processText(query);

    if (queryWords.empty()) {
        return results;
    }

    // Удаляем дубликаты из запроса
    std::sort(queryWords.begin(), queryWords.end());
    queryWords.erase(std::unique(queryWords.begin(), queryWords.end()), queryWords.end());

    // Карта для накопления_scores документов
    std::map<int, double> docScores;

    // Для каждого слова в запросе находим релевантные документы
    for (const auto& word : queryWords) {
        // Проверяем наличие слова в индексе
        auto it = invertedIndex_.find(word);
        if (it == invertedIndex_.end()) {
            continue;
        }

        // Рассчитываем IDF для слова
        double idf = calculateIDF(word);

        // Обновляем score для каждого документа, содержащего слово
        for (const auto& entry : it->second) {
            double tf = calculateTF(word, entry.docId);
            docScores[entry.docId] += tf * idf;
        }
    }

    // Преобразуем карту в вектор результатов
    for (const auto& [docId, score] : docScores) {
        if (score > 0.0) {
            // Получаем документ из кэша или создаем пустой
            auto contentIt = documentContents_.find(docId);
            std::string content = (contentIt != documentContents_.end()) ?
                                   contentIt->second : "";

            // Создаем сниппет
            std::string snippet = createSnippet(content, queryWords);

            // Получаем заголовок (упрощенно - первая строка контента)
            std::string title = "Document #" + std::to_string(docId);
            size_t newlinePos = content.find('\n');
            if (newlinePos != std::string::npos && newlinePos < 100) {
                title = content.substr(0, newlinePos);
                if (title.length() > 80) {
                    title = title.substr(0, 77) + "...";
                }
            } else if (content.length() > 80) {
                title = content.substr(0, 77) + "...";
            }

            results.emplace_back(docId, title, snippet, score);
        }
    }

    // Сортируем результаты по убыванию score
    std::sort(results.begin(), results.end(),
              [](const SearchResult& a, const SearchResult& b) {
                  return a.score > b.score;
              });

    // Ограничиваем количество результатов
    if (results.size() > maxResults) {
        results.resize(maxResults);
    }

    return results;
}

double SearchEngine::calculateTF(const std::string& term, int docId) const {
    // Находим частоту термина в документе
    auto it = invertedIndex_.find(term);
    if (it == invertedIndex_.end()) {
        return 0.0;
    }

    int termFreq = 0;
    for (const auto& entry : it->second) {
        if (entry.docId == docId) {
            termFreq = entry.termFrequency;
            break;
        }
    }

    if (termFreq == 0) {
        return 0.0;
    }

    // Получаем длину документа
    auto lenIt = documentLengths_.find(docId);
    size_t docLength = (lenIt != documentLengths_.end()) ? lenIt->second : 1;

    // Нормализованный TF: частота / длина документа
    return static_cast<double>(termFreq) / static_cast<double>(docLength);
}

double SearchEngine::calculateIDF(const std::string& term) const {
    // Получаем количество документов, содержащих термин
    auto it = documentFrequency_.find(term);
    int docFreq = (it != documentFrequency_.end()) ? it->second : 0;

    if (docFreq == 0) {
        return 0.0;
    }

    // IDF = log(N / df), где N - общее число документов, df - число документов с термином
    // Добавляем 1 к знаменателю для избежания деления на ноль и сглаживания
    return std::log(static_cast<double>(totalDocuments_) / static_cast<double>(docFreq));
}

std::string SearchEngine::createSnippet(const std::string& content,
                                         const std::vector<std::string>& queryWords,
                                         size_t snippetLength) {
    if (content.empty()) {
        return "";
    }

    // Ищем первое вхождение любого слова из запроса
    size_t bestPos = std::string::npos;
    std::string bestWord;

    for (const auto& word : queryWords) {
        // Ищем слово в нижнем регистре
        std::string lowerContent = content;
        std::transform(lowerContent.begin(), lowerContent.end(),
                       lowerContent.begin(), ::tolower);

        size_t pos = lowerContent.find(word);
        if (pos != std::string::npos && (bestPos == std::string::npos || pos < bestPos)) {
            bestPos = pos;
            bestWord = word;
        }
    }

    // Если ничего не найдено, возвращаем начало документа
    if (bestPos == std::string::npos) {
        if (content.length() > snippetLength) {
            return content.substr(0, snippetLength) + "...";
        }
        return content;
    }

    // Вычисляем начало и конец сниппета
    size_t start = (bestPos > snippetLength / 3) ? bestPos - snippetLength / 3 : 0;
    size_t end = std::min(bestPos + snippetLength * 2 / 3, content.length());

    // Корректируем начало до границы слова
    while (start > 0 && !std::isspace(static_cast<unsigned char>(content[start]))) {
        start--;
    }

    // Корректируем конец до границы слова
    while (end < content.length() && !std::isspace(static_cast<unsigned char>(content[end]))) {
        end++;
    }

    std::string snippet = content.substr(start, end - start);

    // Добавляем многоточие если нужно
    if (start > 0) {
        snippet = "..." + snippet;
    }
    if (end < content.length()) {
        snippet += "...";
    }

    return snippet;
}

size_t SearchEngine::getUniqueWordCount() const {
    return invertedIndex_.size();
}

size_t SearchEngine::getDocumentCount() const {
    return totalDocuments_;
}

bool SearchEngine::isIndexed() const {
    return totalDocuments_ > 0;
}

} // namespace minisearch
