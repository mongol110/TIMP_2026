# Стратегия тестирования

- Уровень 1: UnitTests (QtTest) — sha1-константы, AES round-trip, Ньютон ~1.5213797, parsing invalid + not authenticated.
- Уровень 2: Ручной тест-план через nc и GUI (таблица в test-plan.xls):
  - REGISTER новый/повтор, AUTH верный/неверный, команды без AUTH.
  - SHA1 пустой/обычной строки, AES с разными ключами, NEWTON с плохими eps/x0.
  - WAV: embed→extract round-trip, слишком длинное сообщение, битый WAV.
- Уровень 3: Два параллельных клиента, перезапуск сервера (логи в SQLite сохраняются).
- Артефакты: `Docs/test-plan.xls` (тест-план + чек-лист), `Docs/test-cases.xls` (testCase + дефекты).

Файлы xls генерируются скриптом `Docs/make_xls.py` (openpyxl).
