#include <iostream>
#include <string>
#include <thread>
#include <csignal>
#include <atomic>

#include "text_parser.h"
#include "document_store.h"
#include "search_engine.h"
#include "http_server.h"

using namespace minisearch;

// Глобальные переменные для graceful shutdown
std::atomic<bool> g_running(true);
HttpServer* g_server = nullptr;

void signalHandler(int signum) {
    std::cout << "\n[Main] Получен сигнал " << signum << ", завершение работы..." << std::endl;
    g_running = false;
    if (g_server) {
        g_server->stop();
    }
}

/**
 * Загружает тестовые документы в базу данных
 */
void loadTestData(DocumentStore& store) {
    std::cout << "[Main] Загрузка тестовых данных..." << std::endl;

    // Тестовые статьи о технологиях
    struct TestData {
        const char* title;
        const char* content;
    };

    std::vector<TestData> testDocs = {
        {
            "Introduction to C++",
            "C++ is a general-purpose programming language created by Bjarne Stroustrup. "
            "It was first released in 1985 as an extension of the C language. C++ has since "
            "evolved significantly, with major updates including C++11, C++14, C++17, and C++20. "
            "The language supports multiple programming paradigms including procedural, "
            "object-oriented, and generic programming. C++ is widely used in systems programming, "
            "game development, real-time simulations, and performance-critical applications."
        },
        {
            "Modern C++ Features",
            "C++17 introduced many powerful features that make code more expressive and efficient. "
            "Key features include structured bindings, if constexpr, fold expressions, and std::optional. "
            "Structured bindings allow unpacking of tuples and structs directly into named variables. "
            "The std::optional type provides a safe way to represent optional values without using "
            "raw pointers or special sentinel values. C++17 also added parallel algorithms to the STL, "
            "making it easier to write concurrent code."
        },
        {
            "Python Programming Language",
            "Python is a high-level, interpreted programming language known for its simplicity and "
            "readability. Created by Guido van Rossum and first released in 1991, Python has become "
            "one of the most popular languages for web development, data science, artificial intelligence, "
            "and automation. Python's extensive standard library and vibrant ecosystem of third-party "
            "packages make it suitable for almost any programming task. The language emphasizes code "
            "readability with its significant whitespace and clean syntax."
        },
        {
            "Machine Learning with Python",
            "Machine learning is a subset of artificial intelligence that enables systems to learn and "
            "improve from experience. Python has become the de facto language for machine learning thanks "
            "to libraries like TensorFlow, PyTorch, scikit-learn, and Keras. These frameworks provide "
            "powerful tools for building and training neural networks, performing data preprocessing, "
            "and evaluating model performance. Machine learning applications include image recognition, "
            "natural language processing, recommendation systems, and predictive analytics."
        },
        {
            "Linux Operating System",
            "Linux is a free and open-source Unix-like operating system kernel first released by Linus "
            "Torvalds in 1991. Combined with GNU tools and utilities, it forms complete Linux distributions "
            "like Ubuntu, Fedora, and Debian. Linux powers most of the world's servers, supercomputers, "
            "and embedded systems. Its key advantages include stability, security, flexibility, and a "
            "vast community of developers. The Linux kernel uses a monolithic architecture with support "
            "for loadable kernel modules."
        },
        {
            "Data Structures and Algorithms",
            "Understanding data structures and algorithms is fundamental to computer science. Common data "
            "structures include arrays, linked lists, stacks, queues, hash tables, trees, and graphs. "
            "Algorithms are step-by-step procedures for solving problems, classified by their time and "
            "space complexity using Big O notation. Important algorithm categories include sorting, "
            "searching, dynamic programming, and graph algorithms. Mastery of these concepts is essential "
            "for writing efficient code and passing technical interviews."
        },
        {
            "Database Management Systems",
            "A database management system (DBMS) is software for creating, managing, and interacting with "
            "databases. There are several types: relational (SQL), document-oriented, key-value stores, "
            "and graph databases. SQLite is a lightweight, serverless relational database that stores data "
            "in a single file. PostgreSQL and MySQL are popular open-source relational databases for web "
            "applications. NoSQL databases like MongoDB and Redis excel at handling unstructured data and "
            "high-throughput scenarios."
        },
        {
            "Web Development Fundamentals",
            "Web development involves creating websites and web applications. It encompasses frontend "
            "(client-side) and backend (server-side) development. Frontend technologies include HTML, CSS, "
            "and JavaScript, along with frameworks like React, Vue, and Angular. Backend development uses "
            "languages like Python, Java, Node.js, and PHP, often with frameworks such as Django, Spring, "
            "or Express. HTTP is the protocol that powers the web, defining how clients and servers communicate."
        },
        {
            "Computer Networks Basics",
            "Computer networks enable communication between devices. The OSI model defines seven layers: "
            "physical, data link, network, transport, session, presentation, and application. TCP/IP is "
            "the fundamental protocol suite of the Internet. Key networking concepts include IP addressing, "
            "routing, DNS, HTTP/HTTPS, and sockets. Understanding networks is crucial for developing "
            "distributed systems, web applications, and cloud services."
        },
        {
            "Software Engineering Best Practices",
            "Software engineering encompasses methodologies and practices for developing high-quality software. "
            "Key principles include version control with Git, code review, testing (unit, integration, end-to-end), "
            "continuous integration and deployment (CI/CD), and agile development methodologies. Writing clean, "
            "maintainable code requires following coding standards, proper documentation, and design patterns. "
            "DevOps practices bridge development and operations for faster, more reliable software delivery."
        }
    };

    int loaded = 0;
    for (const auto& doc : testDocs) {
        int id = store.addDocument(doc.title, doc.content);
        if (id > 0) {
            loaded++;
            std::cout << "  [Main] Добавлен документ: " << doc.title << std::endl;
        }
    }

    std::cout << "[Main] Загружено " << loaded << " тестовых документов" << std::endl;
}

int main(int argc, char* argv[]) {
    // Настраиваем обработчик сигналов
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "========================================" << std::endl;
    std::cout << "  MiniSearch - Поисковый движок на C++" << std::endl;
    std::cout << "========================================" << std::endl;

    // Определяем порт из аргументов командной строки или используем default
    int port = 8080;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (...) {
            std::cerr << "[Main] Неверный номер порта, используем 8080" << std::endl;
        }
    }

    // Путь к базе данных
    std::string dbPath = "minisearch.db";

    // Инициализируем хранилище документов
    DocumentStore store;
    if (!store.initialize(dbPath)) {
        std::cerr << "[Main] Ошибка инициализации базы данных!" << std::endl;
        return 1;
    }
    std::cout << "[Main] База данных инициализирована: " << dbPath << std::endl;

    // Проверяем, есть ли документы в базе
    if (store.getDocumentCount() == 0) {
        loadTestData(store);
    } else {
        std::cout << "[Main] Найдено " << store.getDocumentCount()
                  << " существующих документов" << std::endl;
    }

    // Создаем и индексируем поисковый движок
    SearchEngine searchEngine;
    searchEngine.indexDocuments(store);

    if (!searchEngine.isIndexed()) {
        std::cerr << "[Main] Ошибка индексации документов!" << std::endl;
        return 1;
    }

    // Создаем и запускаем HTTP сервер
    HttpServer server;
    g_server = &server;

    std::cout << "[Main] Запуск HTTP сервера на порту " << port << "..." << std::endl;

    if (!server.start(port, searchEngine)) {
        std::cerr << "[Main] Ошибка запуска сервера!" << std::endl;
        return 1;
    }

    // Ждем сигнала завершения
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Закрываем хранилище
    store.close();

    std::cout << "[Main] Работа завершена" << std::endl;
    return 0;
}
