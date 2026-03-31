--- MiniSearch/src/text_parser.cpp (原始)


+++ MiniSearch/src/text_parser.cpp (修改后)
#include "text_parser.h"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace minisearch {

TextParser::TextParser() {
    initStopWords();
}

void TextParser::initStopWords() {
    // Русские стоп-слова
    stopWords_ = {
        "и", "в", "на", "о", "с", "к", "по", "за", "от", "до", "для",
        "а", "но", "или", "же", "ли", "бы", "что", "как", "где", "когда",
        "кто", "чем", "кому", "кем", "этом", "этот", "эти", "эта", "это",
        "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for",
        "of", "with", "by", "from", "as", "is", "was", "are", "were", "been",
        "be", "have", "has", "had", "do", "does", "did", "will", "would",
        "could", "should", "may", "might", "must", "shall", "can", "need",
        "it", "its", "this", "that", "these", "those", "i", "you", "he",
        "she", "we", "they", "what", "which", "who", "whom", "whose",
        "if", "then", "else", "when", "where", "why", "how", "all", "each",
        "every", "both", "few", "more", "most", "other", "some", "such",
        "no", "nor", "not", "only", "own", "same", "so", "than", "too",
        "very", "just", "also", "now", "here", "there", "about", "into",
        "through", "during", "before", "after", "above", "below", "between"
    };
}

std::vector<std::string> TextParser::tokenize(const std::string& text) const {
    std::vector<std::string> tokens;
    std::string currentToken;

    for (char c : text) {
        // Разделяем по пробельным символам и знакам препинания
        if (std::isspace(static_cast<unsigned char>(c)) ||
            c == ',' || c == '.' || c == '!' || c == '?' ||
            c == ';' || c == ':' || c == '(' || c == ')' ||
            c == '[' || c == ']' || c == '{' || c == '}' ||
            c == '"' || c == '\'' || c == '-' || c == '_' ||
            c == '/' || c == '\\' || c == '|' || c == '&' ||
            c == '*' || c == '+' || c == '=' || c == '<' || c == '>' ||
            c == '@' || c == '#' || c == '$' || c == '%' || c == '^') {

            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            currentToken += c;
        }
    }

    // Добавляем последний токен
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }

    return tokens;
}

std::string TextParser::normalize(std::string_view token) const {
    std::string result;
    result.reserve(token.size());

    for (char c : token) {
        // Приводим к нижнему регистру
        result += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    return result;
}

bool TextParser::isStopWord(const std::string& word) const {
    return stopWords_.find(word) != stopWords_.end();
}

std::vector<std::string> TextParser::processText(const std::string& text) const {
    std::vector<std::string> result;

    // Токенизируем текст
    auto tokens = tokenize(text);

    // Обрабатываем каждый токен
    for (const auto& token : tokens) {
        // Нормализуем
        std::string normalized = normalize(token);

        // Пропускаем пустые и слишком короткие слова
        if (normalized.empty() || normalized.length() < 2) {
            continue;
        }

        // Пропускаем стоп-слова
        if (isStopWord(normalized)) {
            continue;
        }

        result.push_back(std::move(normalized));
    }

    return result;
}

} // namespace minisearch