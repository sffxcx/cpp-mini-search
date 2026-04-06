#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include <string>
#include <vector>
#include <string_view>
#include <unordered_set>

namespace minisearch {

/**
 * TextParser - утилиты для обработки текста
 *
 * Отвечает за:
 * - Токенизацию (разбиение текста на слова)
 * - Нормализацию (приведение к нижнему регистру)
 * - Удаление стоп-слов
 */
class TextParser {
public:
    TextParser();

    /**
     * Токенизирует текст на отдельные слова
     * @param text Входной текст
     * @return Вектор токенов (слов)
     */
    std::vector<std::string> tokenize(const std::string& text) const;

    /**
     * Нормализует токен: приводит к нижнему регистру, удаляет лишние символы
     * @param token Входной токен
     * @return Нормализованный токен
     */
    std::string normalize(std::string_view token) const;

    /**
     * Проверяет, является ли слово стоп-словом
     * @param word Слово для проверки
     * @return true если слово является стоп-словом
     */
    bool isStopWord(const std::string& word) const;

    /**
     * Обрабатывает текст: токенизирует, нормализует и удаляет стоп-слова
     * @param text Входной текст
     * @return Вектор обработанных слов
     */
    std::vector<std::string> processText(const std::string& text) const;

private:
    // Набор стоп-слов для исключения из индекса
    std::unordered_set<std::string> stopWords_;

    /**
     * Инициализирует набор стоп-слов (русские и английские)
     */
    void initStopWords();
};

} // namespace minisearch

#endif // TEXT_PARSER_H
