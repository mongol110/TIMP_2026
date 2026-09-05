# Структура git и ветки

## Репозиторий
Инициализировать в корне проекта:

```bash
cd TIMP_AES_AUDIO
git init -b main
git add .
git commit -m "feat: audio server+client base (AES/SHA1/Newton/WAV)"
git branch develop
git branch feature/server-auth
git branch feature/client-gui
git branch feature/docs-tests
```

## Схема веток (картинка текстом)

```
main ──●─────────────────────────●  (stable, merge из develop)
        \                       /
develop ──●──┬──────────┬──────●
             │          │
feature/server-auth ●─●─● (parse, multiclient, DB singleton, AUTH)
feature/client-gui  ●─●─● (TcpClient singleton, GUI, таблица)
feature/docs-tests  ●─●─● (puml, Wiki, spec, UnitTests)
```

## Описание
- `main`: только стабильные релизы, тег `v1.0`.
- `develop`: интеграция фич.
- `feature/*`: точечные задачи, merge через `--no-ff`.
- Скриншот `git log --graph --oneline --all` приложить в отчёт (команда ниже).

```bash
git log --graph --oneline --all --decorate
```

## Проверка наличия веток
`git branch -a` должен показать main, develop и 3 feature-ветки.
