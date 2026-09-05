# Проект по дисциплине «Технологии и методы программирования»

**Учебная группа:** XXX-XXX (заглушка)
**Репозиторий:** [godgg582-ai/TIMP_AES_AUDIO](https://github.com/godgg582-ai/TIMP_AES_AUDIO)
**Wiki:** [Документация проекта](https://github.com/godgg582-ai/TIMP_AES_AUDIO/blob/main/Docs/Wiki.md)

---

## Состав команды

| Участник | Роль |
|:---|:---|
| **Участник 1** (заглушка) | Team Lead / DevOps |
| **Участник 2** (заглушка) | Backend Developer |
| **Участник 3** (заглушка) | Frontend Developer |

---

## Вариант задания (Вариант 21)

| Алгоритм | Реализация |
|:---|:---|
| **AES-128-CBC** | Симметричное шифрование/дешифрование (OpenSSL EVP, IV=0) |
| **SHA-1** | Хеширование вручную (FIPS 180-4, без библиотек) |
| **Метод Ньютона** | Решение f(x) = x³ − x − 2 = 0, 15 знаков |
| **Стеганография** | LSB внедрение текста в WAV (1 бит на байт аудио после 44-байт заголовка) |

---

## Структура репозитория

```
TIMP_AES_AUDIO/
├── EchoServer/               # Qt TCP-сервер (порт 34944, мультиклинт)
│   ├── EchoServer.pro
│   ├── main.cpp
│   ├── mytcpserver.h/.cpp    # TCP-сервер (QTcpServer, буфер и auth на сокет)
│   ├── functionsforserver.h/.cpp  # Все 4 алгоритма + REGISTER/AUTH
│   └── dataBase.h            # Singleton SQLite (users + logs)
├── Client/                   # Qt Widgets GUI клиент
│   ├── Client.pro
│   ├── tcpclient.h           # Singleton-обёртка над QTcpSocket
│   ├── mainwindow.h/.cpp     # Главное окно (5 вкладок + лог + таблица)
│   └── connectiondialog.h/.cpp  # Диалог подключения
├── UnitTests/                # Модульные тесты (QtTest, 4 теста)
│   ├── UnitTests.pro
│   └── tst_tests.cpp
├── DataBase/                 # Демо работы с SQLite
├── Singleton/                # Три реализации паттерна Singleton
│   ├── singleton_classic.h   # Классическая (ручное удаление)
│   └── singleton_safe.h      # С Destroyer + Meyers C++11
├── Docs/                     # Wiki, ТЗ, тесты (без doxygen)
│   ├── Wiki.md               # Описание, протокол, сборка
│   ├── git-structure.md      # Ветки и схема git
│   ├── spec-requirements.md  # Спецификация требований (FR1–10)
│   ├── test-strategy.md      # Стратегия тестирования
│   ├── test-plan.xls         # Тест-план + чек-лист
│   ├── test-cases.xls        # TestCase + дефекты
│   └── diagrams/             # UML диаграммы (PlantUML)
│       ├── usecase.puml/.png/.svg      # UseCase диаграмма
│       └── classdiagram.puml/.png/.svg # Диаграмма классов
├── Dockerfile
└── docker-compose.yml
```

---

## Быстрый старт (Docker)

```bash
git clone https://github.com/godgg582-ai/TIMP_AES_AUDIO.git
cd TIMP_AES_AUDIO
docker compose up --build -d
```

Тест (после `AUTH`, иначе `error: not authenticated`):

```bash
echo 'REGISTER|bob|123' | nc -q1 localhost 34944
echo 'AUTH|bob|123' | nc -q1 localhost 34944
```

---

## Протокол сервера (порт 34944, разделитель `|`, конец строки `\n`)

| Команда | Описание |
|:---|:---|
| `REGISTER\|login\|pass` | Регистрация нового пользователя |
| `AUTH\|login\|pass` | Авторизация сокета |
| `SHA1\|данные` | SHA-1 хеш (40 hex, ручная реализация) |
| `AES_ENCRYPT\|ключ\|текст` | Шифрование AES-128-CBC → hex |
| `AES_DECRYPT\|ключ\|hex` | Дешифрование |
| `NEWTON\|x0\|eps` | Корень f(x)=x³−x−2 методом Ньютона (15 знаков) |
| `AUDIO_EMBED\|b64wav\|msg` | Внедрить сообщение в WAV → base64 WAV |
| `AUDIO_EXTRACT\|b64wav` | Извлечь сообщение из WAV |

Без `AUTH` функциональные команды возвращают `error: not authenticated`.
Неизвестная команда → `error`

---

## Сборка вручную (Linux / WSL)

```bash
# Зависимости
sudo apt install qt6-base-dev libssl-dev build-essential cmake ninja-build

# Сервер
cd EchoServer && qmake6 EchoServer.pro && make && ./EchoServer

# GUI клиент
cd Client && qmake6 Client.pro && make && ./Client

# Unit-тесты
cd UnitTests && qmake6 UnitTests.pro && make && ./UnitTests
```

Windows (Qt Creator + MinGW): OpenSSL берётся из `C:/Qt/Tools/mingw1310_64/opt` (прописано в `.pro`).
