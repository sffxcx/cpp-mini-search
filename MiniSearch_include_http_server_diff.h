--- MiniSearch/include/http_server.h (原始)


+++ MiniSearch/include/http_server.h (修改后)
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "search_engine.h"
#include <string>
#include <functional>
#include <atomic>

namespace minisearch {

/**
 * HTTP сервер для обработки поисковых запросов
 *
 * Простой сервер на сокетах, обрабатывающий:
 * - GET /search?q=<запрос> - поиск документов
 * - GET /status - статус системы
 * - GET /health - проверка здоровья
 */
class HttpServer {
public:
    using RequestHandler = std::function<std::string(const std::string&, const std::string&)>;

    HttpServer();
    ~HttpServer();

    /**
     * Запускает сервер на указанном порту
     * @param port Порт для прослушивания
     * @param searchEngine Ссылка на поисковый движок
     * @return true если сервер успешно запущен
     */
    bool start(int port, SearchEngine& searchEngine);

    /**
     * Останавливает сервер
     */
    void stop();

    /**
     * Проверяет, работает ли сервер
     * @return true если сервер работает
     */
    bool isRunning() const;

    /**
     * Возвращает порт, на котором работает сервер
     * @return Номер порта
     */
    int getPort() const;

private:
    int port_;
    std::atomic<bool> running_;
    int serverSocket_;
    SearchEngine* searchEngine_;

    /**
     * Основной цикл обработки соединений
     */
    void serve();

    /**
     * Обрабатывает HTTP запрос
     * @param request Текст запроса
     * @return HTTP ответ
     */
    std::string handleRequest(const std::string& request);

    /**
     * Парсит HTTP запрос и извлекает метод, путь и параметры
     * @param request Текст запроса
     * @param method Выходной параметр: HTTP метод
     * @param path Выходной параметр: путь
     * @param queryParams Выходной параметр: параметры запроса
     * @return true если парсинг успешен
     */
    bool parseRequest(const std::string& request, std::string& method,
                      std::string& path, std::string& queryParams);

    /**
     * Извлекает значение параметра из query строки
     * @param queryParams Query строка
     * @param paramName Имя параметра
     * @return Значение параметра или пустая строка
     */
    std::string getQueryParam(const std::string& queryParams, const std::string& paramName);

    /**
     * URL-декодирует строку
     * @param encoded Закодированная строка
     * @return Декодированная строка
     */
    std::string urlDecode(const std::string& encoded);

    /**
     * Создает JSON ответ для поиска
     * @param query Исходный запрос
     * @param results Результаты поиска
     * @param timeMs Время выполнения в мс
     * @return JSON строка
     */
    std::string createSearchResponse(const std::string& query,
                                     const std::vector<SearchResult>& results,
                                     double timeMs);

    /**
     * Создает JSON ответ для статуса
     * @return JSON строка
     */
    std::string createStatusResponse();

    /**
     * Создает HTTP заголовки
     * @param contentType Тип контента
     * @param contentLength Длина контента
     * @return Строка заголовков
     */
    std::string createHeaders(const std::string& contentType, size_t contentLength);

    /**
     * Экранирует строку для JSON
     * @param input Входная строка
     * @return Экранированная строка
     */
    std::string escapeJson(const std::string& input);
};

} // namespace minisearch

#endif // HTTP_SERVER_H