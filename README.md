# TIMP — AES + SHA1 + Метод Ньютона + Стеганография в аудио (WAV)

**Тема №21:** `aes / sha1 / Метод Ньютона / внедрение сообщения в музыкальный файл`

Учебный клиент-серверный проект по дисциплине «Технологии и методы программирования».
Сделан с нуля по мотивам примера `TIMP_2026`, но со своей темой и своим протоколом.

**Целевой набор: ~160 баллов** (без doxygen):
- Wiki (5) + структура git + ветки (5)
- Сервер: parse + заглушки (10) + БД в синглтоне (10) + несколько клиентов (10) + подключение БД, авторизация и регистрация (10)
- Docker (10)
- Диаграмма классов (5) + UseCase (5)
- Оконный интерфейс (10) + клиент в синглтоне (10) + клиент готовый (10)
- Функционал 4 задачи x10 = 40 (AES, SHA1, Ньютон, аудио-стего)
- UnitTest 4 теста x5 = 20
- Итого: 100 + 40 + 20 = **160**

Все некодовые артефакты лежат в `Docs/`. Все диаграммы — PlantUML (`.puml`). Doxygen не используется.

## Структура

```
TIMP_AES_AUDIO/
├── EchoServer/          # Qt TCP-сервер, порт 34944
│   ├── main.cpp
│   ├── mytcpserver.h/.cpp   # multi-client TCP (QMap buffers + auth set)
│   ├── functionsforserver.h/.cpp  # AES, SHA1, Newton, WAV-stego
│   └── dataBase.h           # Singleton SQLite: users + logs
├── Client/              # Qt Widgets GUI клиент
│   ├── main.cpp
│   ├── tcpclient.h/.cpp     # Singleton-обёртка над QTcpSocket
│   ├── mainwindow.h/.cpp    # Auth + 4 вкладки + лог
│   └── connectiondialog.h/.cpp
├── Singleton/           # Демо паттерна Singleton
├── DataBase/            # Демо работы с SQLite
├── UnitTests/           # QtTest: sha1, aes, newton, parsing
├── Docs/                # Wiki, git-структура, puml, spec, стратегия
│   ├── Wiki.md
│   ├── git-structure.md
│   ├── spec-requirements.md
│   ├── test-strategy.md
│   └── diagrams/*.puml
├── Dockerfile
├── docker-compose.yml
├── TIMP_AES_AUDIO.pro
└── CMakeLists.txt
```

## Протокол сервера (порт 34944, разделитель `|`, конец строки `\n`)

| Команда | Пример | Ответ |
|:---|:---|:---|
| `REGISTER\|login\|pass` | `REGISTER\|alice\|123` | `ok: registered` / `error: ...` |
| `AUTH\|login\|pass` | `AUTH\|alice\|123` | `ok: authenticated` / `error: ...` |
| `SHA1\|data` | `SHA1\|hello` | hex SHA1 (40 символов) |
| `AES_ENCRYPT\|key\|text` | `AES_ENCRYPT\|secretkey\|Hello` | hex шифротекст (AES-128-CBC) |
| `AES_DECRYPT\|key\|hex` | `AES_DECRYPT\|secretkey\|<hex>` | исходный текст |
| `NEWTON\|x0\|eps` | `NEWTON\|1.5\|1e-9` | корень `x^3-x-2=0` (~1.5213797) |
| `AUDIO_EMBED\|b64wav\|msg` | base64 WAV + текст | base64 WAV со встроенным сообщением |
| `AUDIO_EXTRACT\|b64wav` | base64 WAV | извлечённое сообщение |
| прочее | | `error...` |

Функциональные команды (кроме REGISTER/AUTH) требуют авторизации на этом сокете:
без `AUTH` сервер отвечает `error: not authenticated`.

## Быстрый старт (Docker)

```bash
docker compose up --build
# проверка:
# (в другом терминале)
# python -c "import socket; s=socket.create_connection(('127.0.0.1',34944)); print(s.recv(4096).decode()); s.sendall(b'REGISTER|bob|123\n'); print(s.recv(4096).decode())"
```

## Сборка вручную (Linux / WSL, Qt6)

```bash
sudo apt install qt6-base-dev libssl-dev build-essential cmake ninja-build
cmake -B build && cmake --build build
./build/EchoServer/EchoServer        # сервер
./build/Client/Client                # GUI клиент
./build/UnitTests/UnitTests          # тесты
```

## Git-ветки

- `main` — стабильная сборка
- `develop` — интеграция
- `feature/server-auth` — сервер + БД + авторизация
- `feature/client-gui` — клиент + синглтон + GUI
- `feature/docs-tests` — Docs + puml + UnitTests

Подробности: `Docs/git-structure.md`, `Docs/Wiki.md`.
