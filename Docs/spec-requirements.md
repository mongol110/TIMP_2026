# Спецификация требований

## 1. Функциональные
- FR1: Сервер слушает TCP 34944, держит N клиентов одновременно.
- FR2: `REGISTER|login|pass` создаёт пользователя, повтор — ошибка.
- FR3: `AUTH|login|pass` аутентифицирует сокет, дальше `user` привязан к сокету.
- FR4: Без AUTH команды SHA1/AES/NEWTON/AUDIO возвращают `error: not authenticated`.
- FR5: SHA1 возвращает 40 hex символов.
- FR6: AES-128-CBC encrypt/decrypt round-trip.
- FR7: NEWTON решает `x^3-x-2=0`, точность eps 1e-12..1.
- FR8: AUDIO_EMBED/EXTRACT корректны, проверка размера WAV и длины.
- FR9: Все запросы логируются в SQLite (без паролей в открытом виде).
- FR10: GUI: авторизация, 4 вкладки, лог + таблица, синглтон TcpClient.

## 2. Нефункциональные
- NFR1: Qt6 + OpenSSL, C++17, сборка CMake и qmake.
- NFR2: Docker-образ собирается `docker compose up --build`.
- NFR3: Неизвестная команда → `error`.

## 3. Приёмка
- Register→Auth→SHA1/AES/NEWTON/AUDIO через nc и через GUI.
- Два одновременных клиента не мешают друг другу.
- UnitTests: 4 теста зелёные.
