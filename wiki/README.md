# Local Wiki

This project ships with a serveable local wiki powered by **Gollum**, which is actual wiki software
backed by Git.

## Why there is a second documentation layer

The static documentation site is ideal for polished reference material. The local wiki is better
for browseable tutorial-style pages, glossaries, troubleshooting notes, and future lab notebook
content.

## Start the wiki

```bash
docker compose -f wiki/docker-compose.yml up --build
```

Then open:

```text
http://localhost:4567
```

## Seeded pages

- `Home`
- `Beginner-Glossary`
- `Signal-Flow`
- `Troubleshooting`
