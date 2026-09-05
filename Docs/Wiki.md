# Wiki — TIMP AES/SHA1/Newton/Audio

## 1. Тема
Вариант 21: AES + SHA1 + метод Ньютона + внедрение сообщения в музыкальный файл (WAV).

## 2. Что делает проект
- TCP-сервер (Qt6, порт 34944) обслуживает **несколько клиентов одновременно**.
- Авторизация и регистрация (`REGISTER|`, `AUTH|`), пароли хранятся как SHA1(login:pass).
- Функционал:
  - `SHA1|data` — хеш SHA1 вручную по FIPS 180-4 (40 hex).
  - `AES_ENCRYPT|key|text` / `AES_DECRYPT|key|hex` — AES-128-CBC через OpenSSL EVP (IV=0, ключ до 16 байт).
  - `NEWTON|x0|eps` — корень `x^3-x-2=0` методом Ньютона.
  - `AUDIO_EMBED|b64wav|msg` / `AUDIO_EXTRACT|b64wav` — LSB-стего в WAV.
- SQLite в синглтоне `DataBase`: таблицы `users`, `logs`.
- GUI-клиент: вкладки Авторизация/AES/SHA1/Ньютон/Аудио + Лог + Таблица команд.
- Сеть клиента — синглтон `TcpClient` (единственная точка доступа).
- Docker: сервер собирается и запускается одной командой.

## 3. Протокол
Разделитель `|`, строка заканчивается `\n`. Без `AUTH` функционал отвечает `error: not authenticated`.

## 4. Примеры
```
REGISTER|alice|123      -> ok: registered
AUTH|alice|123          -> ok: authenticated
SHA1|hello              -> aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d
AES_ENCRYPT|secretkey|Hi -> <hex>
NEWTON|1.5|1e-9         -> 1.5213797068
```

## 5. WAV-стего
- Вход: PCM WAV 44 байта заголовка (RIFF/WAVE).
- Формат нагрузки: 4 байта BE длины + UTF-8 сообщение, по 1 биту в LSB каждого байта аудио.
- Проверка вместимости, иначе `error: message too long`.

## 6. Сборка и запуск
См. README.md. Docker: `docker compose up --build`.
