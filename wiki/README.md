# Local Wiki

This project ships with a serveable local wiki powered by **Gollum**, which is actual wiki software
backed by Git.

## Why there is a second documentation layer

The static documentation site is ideal for polished reference material. The local wiki is better
for browseable tutorial-style pages, glossaries, troubleshooting notes, and future lab notebook
content.

## Start the wiki

```powershell
./scripts/start-wiki.ps1
```

Then open:

```text
http://localhost:4567
```

If Docker Desktop is not installed yet, run:

```powershell
./scripts/install-prerequisites.ps1 -IncludeWikiTools
```

## Seeded pages

- `Home`
- `Beginner-Glossary`
- `Signal-Flow`
- `Troubleshooting`
