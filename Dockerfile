# Используем стабильный образ Ubuntu 22.04
FROM ubuntu:22.04

# Предотвращаем интерактивные запросы при установке пакетов
ENV DEBIAN_FRONTEND=noninteractive

# Устанавливаем компилятор C++, CMake, Make и SQLite
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    make \
    g++ \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# Устанавливаем рабочую директорию внутри контейнера
WORKDIR /app

# Копируем все файлы проекта в контейнер
COPY . .

# Создаем папку для данных (SQLite база будет здесь)
RUN mkdir -p data

# Создаем папку build и собираем проект через CMake
RUN mkdir -p build && \
    cd build && \
    cmake .. && \
    make

# Открываем порт 8080 для HTTP-сервера
EXPOSE 8080

# Запускаем наш поисковый движок
CMD ["./build/mini_search"]
