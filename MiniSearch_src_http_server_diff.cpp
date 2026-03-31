--- MiniSearch/src/http_server.cpp (原始)


+++ MiniSearch/src/http_server.cpp (修改后)
#include "http_server.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <algorithm>
#include <iomanip>

namespace minisearch {

HttpServer::HttpServer()
    : port_(0), running_(false), serverSocket_(-1), searchEngine_(nullptr) {}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start(int port, SearchEngine& searchEngine) {
    port_ = port;
    searchEngine_ = &searchEngine;

    // Создаем сокет
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        std::cerr << "[HttpServer] Ошибка создания сокета: " << strerror(errno) << std::endl;
        return false;
    }

    // Разрешаем повторное использование адреса
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[HttpServer] Ошибка setsockopt: " << strerror(errno) << std::endl;
        close(serverSocket_);
        return false;
    }

    // Настраиваем адрес
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Привязываем сокет к адресу
    if (bind(serverSocket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[HttpServer] Ошибка bind: " << strerror(errno) << std::endl;
        close(serverSocket_);
        return false;
    }

    // Начинаем слушать
    if (listen(serverSocket_, 10) < 0) {
        std::cerr << "[HttpServer] Ошибка listen: " << strerror(errno) << std::endl;
        close(serverSocket_);
        return false;
    }

    running_ = true;
    std::cout << "[HttpServer] Сервер запущен на порту " << port << std::endl;

    // Запускаем цикл обработки в отдельном потоке
    serve();

    return true;
}

void HttpServer::stop() {
    running_ = false;

    if (serverSocket_ >= 0) {
        close(serverSocket_);
        serverSocket_ = -1;
    }

    std::cout << "[HttpServer] Сервер остановлен" << std::endl;
}

bool HttpServer::isRunning() const {
    return running_;
}

int HttpServer::getPort() const {
    return port_;
}

void HttpServer::serve() {
    while (running_) {
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);

        // Принимаем соединение с таймаутом
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket_, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int activity = select(serverSocket_ + 1, &readfds, nullptr, nullptr, &timeout);

        if (activity > 0 && FD_ISSET(serverSocket_, &readfds)) {
            int clientSocket = accept(serverSocket_,
                                       (struct sockaddr*)&clientAddress,
                                       &clientLen);

            if (clientSocket >= 0) {
                // Читаем запрос
                char buffer[8192];
                ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    std::string request(buffer);

                    // Обрабатываем запрос
                    std::string response = handleRequest(request);

                    // Отправляем ответ
                    send(clientSocket, response.c_str(), response.length(), 0);
                }

                close(clientSocket);
            }
        }
    }
}

std::string HttpServer::handleRequest(const std::string& request) {
    std::string method, path, queryParams;

    if (!parseRequest(request, method, path, queryParams)) {
        std::string body = "{\"error\": \"Invalid request\"}";
        return createHeaders("application/json", body.length()) + body;
    }

    std::cout << "[HttpServer] " << method << " " << path;
    if (!queryParams.empty()) {
        std::cout << "?" << queryParams;
    }
    std::cout << std::endl;

    std::string responseBody;

    if (path == "/search" && method == "GET") {
        // Поиск
        std::string query = getQueryParam(queryParams, "q");
        query = urlDecode(query);

        if (query.empty()) {
            responseBody = "{\"error\": \"Missing query parameter 'q'\"}";
        } else {
            auto startTime = std::chrono::high_resolution_clock::now();
            auto results = searchEngine_->search(query);
            auto endTime = std::chrono::high_resolution_clock::now();

            double timeMs = std::chrono::duration<double, std::milli>(
                endTime - startTime).count();

            responseBody = createSearchResponse(query, results, timeMs);

            std::cout << "[HttpServer] Найдено " << results.size()
                      << " результатов за " << std::fixed << std::setprecision(2)
                      << timeMs << " мс" << std::endl;
        }
    } else if (path == "/status" || path == "/health") {
        // Статус
        responseBody = createStatusResponse();
    } else if (path == "/" ) {
        // Главная страница
        responseBody = R"({
            "name": "MiniSearch",
            "version": "1.0.0",
            "endpoints": {
                "/search?q=<query>": "Поиск документов",
                "/status": "Статус системы",
                "/health": "Проверка здоровья"
            }
        })";
    } else {
        // 404
        std::string body = "{\"error\": \"Not found\"}";
        return createHeaders("application/json", body.length()) + body;
    }

    return createHeaders("application/json", responseBody.length()) + responseBody;
}

bool HttpServer::parseRequest(const std::string& request, std::string& method,
                               std::string& path, std::string& queryParams) {
    std::istringstream iss(request);
    std::string firstLine;

    if (!std::getline(iss, firstLine)) {
        return false;
    }

    // Удаляем \r если есть
    if (!firstLine.empty() && firstLine.back() == '\r') {
        firstLine.pop_back();
    }

    // Парсим первую строку: METHOD /path?query HTTP/1.1
    std::istringstream lineStream(firstLine);
    lineStream >> method >> path;

    if (method.empty() || path.empty()) {
        return false;
    }

    // Разделяем путь и query параметры
    size_t queryPos = path.find('?');
    if (queryPos != std::string::npos) {
        queryParams = path.substr(queryPos + 1);
        path = path.substr(0, queryPos);
    }

    return true;
}

std::string HttpServer::getQueryParam(const std::string& queryParams,
                                        const std::string& paramName) {
    std::istringstream iss(queryParams);
    std::string param;

    while (std::getline(iss, param, '&')) {
        size_t eqPos = param.find('=');
        if (eqPos != std::string::npos) {
            std::string name = param.substr(0, eqPos);
            std::string value = param.substr(eqPos + 1);

            if (name == paramName) {
                return value;
            }
        }
    }

    return "";
}

std::string HttpServer::urlDecode(const std::string& encoded) {
    std::string decoded;
    decoded.reserve(encoded.length());

    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            // Декодируем %XX
            std::string hex = encoded.substr(i + 1, 2);
            char ch = static_cast<char>(std::stoi(hex, nullptr, 16));
            decoded += ch;
            i += 2;
        } else if (encoded[i] == '+') {
            decoded += ' ';
        } else {
            decoded += encoded[i];
        }
    }

    return decoded;
}

std::string HttpServer::createSearchResponse(const std::string& query,
                                              const std::vector<SearchResult>& results,
                                              double timeMs) {
    std::ostringstream json;
    json << std::fixed << std::setprecision(3);

    json << "{\n";
    json << "  \"query\": \"" << escapeJson(query) << "\",\n";
    json << "  \"results\": [\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const auto& result = results[i];
        json << "    {\n";
        json << "      \"id\": " << result.docId << ",\n";
        json << "      \"title\": \"" << escapeJson(result.title) << "\",\n";
        json << "      \"score\": " << result.score << ",\n";
        json << "      \"snippet\": \"" << escapeJson(result.snippet) << "\"\n";
        json << "    }";
        if (i < results.size() - 1) {
            json << ",";
        }
        json << "\n";
    }

    json << "  ],\n";
    json << "  \"total\": " << results.size() << ",\n";
    json << "  \"time_ms\": " << timeMs << "\n";
    json << "}";

    return json.str();
}

std::string HttpServer::createStatusResponse() {
    std::ostringstream json;

    json << "{\n";
    json << "  \"documents\": " << searchEngine_->getDocumentCount() << ",\n";
    json << "  \"unique_words\": " << searchEngine_->getUniqueWordCount() << ",\n";
    json << "  \"status\": \"ready\",\n";
    json << "  \"indexed\": " << (searchEngine_->isIndexed() ? "true" : "false") << "\n";
    json << "}";

    return json.str();
}

std::string HttpServer::createHeaders(const std::string& contentType,
                                       size_t contentLength) {
    std::ostringstream headers;

    headers << "HTTP/1.1 200 OK\r\n";
    headers << "Content-Type: " << contentType << "\r\n";
    headers << "Content-Length: " << contentLength << "\r\n";
    headers << "Access-Control-Allow-Origin: *\r\n";
    headers << "Connection: close\r\n";
    headers << "\r\n";

    return headers.str();
}

std::string HttpServer::escapeJson(const std::string& input) {
    std::string output;
    output.reserve(input.length() * 1.2);

    for (char c : input) {
        switch (c) {
            case '"':  output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\b': output += "\\b"; break;
            case '\f': output += "\\f"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Контрольные символы
                    std::ostringstream oss;
                    oss << "\\u" << std::hex << std::setw(4)
                        << std::setfill('0') << static_cast<int>(c);
                    output += oss.str();
                } else {
                    output += c;
                }
        }
    }

    return output;
}

} // namespace minisearch