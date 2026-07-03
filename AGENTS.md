# BookKeeping Project Instructions

## Project Goal

A Qt6 + C++17 + SQLite3 desktop accounting app with clean three-layer architecture.
Primary goal: reliable data storage and intuitive expense/income tracking.
Secondary goal: maintainable codebase that supports future feature additions.

## Tech Stack

| Component | Detail |
|-----------|--------|
| Language  | C++17 |
| UI        | Qt6 (Widgets + Charts) |
| Database  | SQLite3 via sqlite_modern_cpp |
| Build     | CMake 3.22+, MSVC 2022 |
| Encoding  | UTF-8 (compile flag /utf-8) |

## Architecture

```
MainApp.exe (entry)
  ├─ MyWindow.dll (UI layer, Mediator pattern)
  └─ DataCenter.dll (data layer, Facade pattern)
       └─ sqlite3
       └─ TradeRecord.h (pure model, zero dependencies)
Tests/DataCenterTest.exe (console, no GUI needed)
  └─ DataCenter.dll
```

### Layer Rules

- UI Layer (MyWindow): layout, signals, user interaction. Never access database directly.
- Data Layer (DataCenter): all business logic + database access. Never include UI headers.
- Model (TradeRecord.h): pure data structures only. No business logic, no database code.

## Code Style

### Naming

| Category   | Style       | Examples |
|----------- |-------------|----------|
| Class      | PascalCase  | MainWidget, AccountingWidget |
| Function   | camelCase   | initLayout, addRecord |
| Variable   | camelCase   | dbPath, recordCount |
| Member     | m_pImpl / d_ptr / q_ptr | m_pImpl, d_ptr |

### Patterns

- PIMPL: MainWidget / AccountingWidget use QScopedPointer + Q_DECLARE_PRIVATE
- PIMPL (variant): DataCenter / PageController / DBHelper use std::unique_ptr<Impl>
- Facade: DataCenter wraps all data access behind a single interface
- Mediator: MainWindow connects UI signals to DataCenter slots

### Signal-Slot Convention

```
UI component emits signal -> MainWindow catches it -> calls DataCenter -> refreshes UI
```

UI components must NOT hold a DataCenter reference. All coordination goes through MainWindow.

## Development Workflow

### Step 1: Analyze

Read the full file context before making any changes. Search for all related code. Understand the existing behavior.

### Step 2: Plan

State your plan before modifying. For any deletion, ask the user first. For any refactoring, explain what behavior stays the same.

### Step 3: Modify

- When adding new features: do NOT modify existing working code. Add new files or extend interfaces.
- When refactoring: the old and new behavior MUST be identical. Verify after refactoring.
- After modifying a .h file: ALWAYS run `cmake -S . -B build` before building.

### Step 4: Build

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --config RelWithDebInfo
```

### Step 5: Test

```powershell
chcp 65001
& E:\code\sdk\DataCenterTest.exe
```

- New features must have corresponding tests.
- After modifying existing logic, verify all existing tests still pass.
- Never write test data to the production database. Use initTables(tempPath).

### Step 6: Commit

```
<type>: <short description>
```

Types: feat / fix / refactor / test / docs / chore

## Safety Rules (NEVER Violate)

### Data Safety (Priority 1)

1. NEVER delete user data without explicit confirmation.
2. NEVER write test data to the production database (E:\code\sdk\config\account.db).
3. ALWAYS use a temporary path for testing: initTables(tempDbPath).
4. BEFORE any deletion: create a backup or confirm with the user.

### Code Safety (Priority 2)

1. NEVER concatenate strings to build SQL. Use parameterized queries only: exec(sql, params) / query(sql, params) with ? placeholders.
2. NEVER modify working code when adding a new feature. Extend existing interfaces or create new files.
3. When refactoring: behavior before and after MUST be identical. No functionality changes during refactoring.
4. ALWAYS run cmake -S . -B build after editing a .h file, otherwise SDKPATH headers are stale.

### Database Schema (Current)

| Column            | Type    |
|------------------|---------|
| id               | INTEGER PRIMARY KEY AUTOINCREMENT |
| type             | TEXT    |
| category         | TEXT    |
| source           | TEXT    |
| amount           | REAL    |
| date             | TEXT    |
| remark           | TEXT    |
| source_account   | TEXT    |
| target_account   | TEXT    |

## Encoding Notes

- Console code page defaults to 936 (GB2312). Run `chcp 65001` before testing for correct Chinese display.
- Chinese characters in source files: use `QStringLiteral("\uXXXX")` to avoid encoding issues.
- When writing files containing Chinese through opencode tools, use PowerShell `Set-Content -Encoding UTF8` instead of the Write/Edit tools (which can corrupt Chinese text).

## Response Style

1. First analyze: read the code, understand the context, explain what you found.
2. Then plan: state what you will do and why.
3. Finally execute: make precise, minimal changes.
4. After execution: summarize what changed and verify.

## Refactoring Progress

| Phase | Content | Status |
|-------|---------|--------|
| 1 | SQL injection fix, deleteRecord fix, column names, delete UI flow, profit default text | DONE |
| 2 | AnalysisWidget real data, filter/search, redundant queries | TODO |
| 3 | DBHelper async simplify, PageController integration, Repository, CMake cleanup, CSV import | TODO |