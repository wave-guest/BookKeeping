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

### Step 0: Update AGENTS.md

- Upon completing each step, immediately update AGENTS.md:
  - Mark the step as **DONE** in the **Refactoring Progress** table.
  - Mark corresponding entries in **Known Issues** as ~~strikethrough~~ with "→ **已修复 (Step N)**".
  - Remove or update the step's detailed instructions from **Execution Checklist**.

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

### Step 6: Summarize

Upon completing execution, provide a summary covering:
- **修改内容**: 什么文件变了，变的内容是什么
- **问题**: 执行过程中遇到的问题
- **解决方案**: 问题如何解决
- **计划同步**: AGENTS.md 的 Refactoring Progress / Known Issues / Execution Checklist 是否已更新
- **下一步**: 如果还有剩余 TODO，说明下一步预期做什么；如果所有计划完成，则明确说明"所有步骤已完成"

### Step 7: Commit

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

## Project Structure Reference

```
E:\code\BookKeeping/
├── CMakeLists.txt            # 顶层构建（4个子模块 + /utf-8）
├── QtConfig.cmake            # 自定义CMake函数：setup_qt_library/setup_qt_exe/头文件递归拷贝
├── build.bat                 # cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
├── ARCHITECTURE.md           # 架构审查与三阶段重构计划
├── AGENTS.md                 # 本文件：项目指令
│
├── MainApp/                  # [EXE] 程序入口（BKPro.exe）
│   ├── main.cpp              # QApplication + HighDPI + SimHei字体
│   └── CMakeLists.txt        # 链接 MyWindow + DataCenter
│
├── MyWindow/                 # [DLL] UI层（MyWindow.dll）
│   ├── MainWindow.h/.cpp     # 中介者：导航栏 + QStackedWidget + 所有信号连接
│   ├── AccountingWidget.h/.cpp   # 记账表单(左) + 表格+筛选/搜索/删改(右)
│   ├── AnalysisWidget.h/.cpp     # 饼图/折线图（有接口但未接真实数据）
│   ├── PageController.h/.cpp     # 分页控件（编译但未使用）
│   └── CMakeLists.txt        # 依赖 Qt Charts + DataCenter
│
├── DataCenter/               # [DLL] 数据层（DataCenter.dll）
│   ├── TradeRecord.h         # 纯模型：TradeRecord, Statistics, TradeType, StatType, TimeRange
│   ├── RecordRepository.h/.cpp # Repository：所有SQL语句 + 行解析，内部被DataCenter调用
│   ├── DataCenter.h/.cpp     # Facade：业务编排，调用Repository接口
│   ├── DBHelper.h/.cpp       # SQLite封装：纯同步，参数化查询
│   └── CMakeLists.txt        # 依赖 sqlite3
│
├── Tests/                    # [EXE] 测试（DataCenterTest.exe）
│   ├── test_datacenter.cpp   # 20+测试：CRUD + SQL注入验证 + TimeRange统计
│   └── CMakeLists.txt        # 链接 DataCenter + sqlite3
│
└── 账单/支付宝/              # 6个支付宝年度CSV账单源文件
```

### Current Design

| Pattern | Where | Notes |
|---------|-------|-------|
| Facade | DataCenter | 单一接口封装所有数据操作 |
| Mediator | MainWindow | 所有UI↔DataCenter通信经MainWindow中转 |
| PIMPL (QScopedPointer) | MainWidget, AccountingWidget, AnalysisWidget | d_ptr + Q_DECLARE_PRIVATE |
| PIMPL (unique_ptr) | DataCenter, DBHelper, PageController | std::unique_ptr<Impl> |

### Signal Flow
```
UI组件 emit signal → MainWindow Lambda槽 → DataCenter方法 → DBHelper.exec/query → 返回结果 → UI刷新
```
UI组件**不持有**DataCenter引用，全经过MainWindow协调。

### Known Issues
1. ~~**DataCenter.cpp 函数重复**~~ → **已修复 (Step 0)**
2. ~~**AnalysisWidget::dataRequested 信号无人连接**~~ → **已修复 (Step 1)**
3. **筛选/搜索** UI层是存根（仅弹框），DataCenter层已有完整实现未连通
4. **updateBalanceCards** 12次独立SQL调用，可合并为4次
5. ~~**PageController** 编译但无人使用~~ → **已修复 (Step 5)**
6. **DBHelper** 伪异步（线程+queue+mutex+cv+future.get阻塞）
7. ~~**WINDOWS_EXPORT_ALL_SYMBOLS** 导出所有DLL符号~~ → **已修复 (Step 7)**

## Execution Checklist

```mermaid
flowchart TD
    A["Step 0: 修复函数重复定义"] --> B["Step 1: AnalysisWidget 接入真实数据"]
    B --> C["Step 2: 实现筛选/搜索功能"]
    C --> D["Step 3: 合并 updateBalanceCards 冗余查询"]
    D --> E["Step 4: DBHelper 异步模型简化"]
    E --> F["Step 5: PageController 集成"]
    F --> G["Step 6: 引入 Repository 层"]
    G --> H["Step 7: CMake 配置清理"]
    H --> I["Step 8: 支付宝 CSV 导入"]
```

### Step 0: 修复 DataCenter.cpp 函数重复定义（阻塞项）

- **位置**: `DataCenter/DataCenter.cpp` 第249-379行 与 第397-526行
- **问题**: `getCategoryStats` / `getDailyStats` / `getRecordsByDate` / `searchRecords` 各定义两次，导致链接错误
- **操作**: 删除第397-526行的重复实现，保留249-379行的版本
- **验证**: `cmake --build build --config RelWithDebInfo` 通过

### Step 1: AnalysisWidget 接入真实数据 (Phase 2.1)

| # | 操作 | 文件 |
|---|------|------|
| 1.1 | connect `AnalysisWidget::dataRequested` → 调用 `DataCenter::getCategoryStats/getDailyStats` | `MyWindow/MainWindow.cpp` |
| 1.2 | Lambda内调用 `loadPieData()`/`loadLineData()` 回填图表 | `MyWindow/MainWindow.cpp` |

### Step 2: 实现筛选/搜索功能 (Phase 2.2)

| # | 操作 | 文件 |
|---|------|------|
| 2.1 | connect 筛选信号 → `DataCenter::getRecordsByDate` | `MyWindow/MainWindow.cpp` |
| 2.2 | connect 搜索信号 → `DataCenter::searchRecords` | `MyWindow/MainWindow.cpp` |
| 2.3 | `onFilterRecords()`/`onSearchRecords()` 改为发射信号而非弹框 | `MyWindow/AccountingWidget.cpp:528-557` |

### Step 3: 合并 updateBalanceCards 冗余查询 (Phase 2.3)

- **位置**: `MyWindow/MainWindow.cpp:400-446`
- **当前**: 12次独立SQL调用（4组 × income/expense/profit）
- **改为**: 4次 `getStatistics(Total/Year/Month/Day)` 调用，复用 Statistics 结构体

### Step 4: DBHelper 异步模型简化 (Phase 3.1)

- **文件**: `DataCenter/DBHelper.h/.cpp`
- **当前**: 后台线程 + 任务队列 + mutex + cv + future.get() 阻塞（伪异步）
- **改为**: 纯同步，移除线程池；未来如需异步用 Qt 信号通知

### Step 5: PageController 集成 (Phase 3.2)

| # | 操作 | 文件 |
|---|------|------|
| 5.1 | 新增 `getRecords(page, pageSize)` 分页查询接口 | `DataCenter/DataCenter.h/.cpp` |
| 5.2 | `AccountingWidget` 添加分页控件，绑定上/下一页 | `MyWindow/AccountingWidget.cpp` |
| 5.3 | FirstPage/PrevPage/NextPage/LastPage 真实实现 | `MyWindow/PageController.cpp` |

### Step 6: 引入 Repository 层 (Phase 3.3)

| # | 操作 | 文件 |
|---|------|------|
| 6.1 | 新建 `RecordRepository.h/.cpp`，抽取所有SQL语句+行解析 | `DataCenter/RecordRepository.*` (新文件) |
| 6.2 | DataCenter 改为仅编排业务，调用 Repository 接口 | `DataCenter/DataCenter.cpp` |
| 6.3 | 私有助手 `parseRecord()` 消除4处重复的行解析代码 | `DataCenter/RecordRepository.cpp` |
- **验证**: 30个测试通过29个（`inject count=3` 为重构前已有失败，非本次引入）

### Step 7: CMake 配置清理 (Phase 3.4)

| # | 操作 | 文件 |
|---|------|------|
| 7.1 | 移除 `WINDOWS_EXPORT_ALL_SYMBOLS` | `QtConfig.cmake:73` |
| 7.2 | 显式 `MYWINDOW_EXPORT` 宏标记公开接口 | `MyWindow/*.h` |

### Step 8: 支付宝 CSV 导入 (搁置)

### Step 9: 修复编辑后不刷新收支卡片（Bug）

- **问题**: `MainWindow.cpp:357-361` 中 `updateRecord` lambda 未调用 `updateBalanceCards()` 和 `setTotalPage()`
- **操作**: lambda 尾部追加 `updateBalanceCards()` 和分页总数刷新
- **文件**: `MyWindow/MainWindow.cpp`

### Step 10: 分析页分类筛选生效（半成品完善）

- **问题**: `AnalysisWidget` 的 `categoryCombo` 有 UI 但 `onFilterChanged()` 未读取其值
- **操作**: `onFilterChanged()` 读取 `categoryCombo->currentText()` 传给 `dataRequested` 信号
- **文件**: `MyWindow/AnalysisWidget.cpp`

### Step 11: 删除无用代码（评判：之后无使用价值）

| # | 操作 | 文件 | 理由 |
|---|------|------|------|
| 11.1 | 删除 `TradeType` 枚举 | `TradeRecord.h:33-50` | 0 引用，项目用 `QString` 存分类 |
| 11.2 | 删除 `StatType` 枚举 | `TradeRecord.h:52-56` | 0 引用，用 `TimeRange` 区分维度 |
| 11.3 | 删除 `TimeRange::Custom` | `TradeRecord.h:63` | 0 引用，`switch` 无 `case` 处理 |
| 11.4 | 删除 `PageController` 的 `onFirstPage/onPrevPage/onNextPage/onLastPage` 信号 | `PageController.h/.cpp` | 只 `emit` 无人 `connect` |
| 11.5 | 删除 `TradeRecord::info()` 注释代码 | `TradeRecord.h:21-30` | 整个方法体注释 |

### 验证方式

| 范围 | 命令 |
|------|------|
| Step 0-7 | `cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo` + `cmake --build build --config RelWithDebInfo` + `& E:\code\sdk\DataCenterTest.exe` |
| Step 9-11 | 同上 + 启动 BKPro.exe 手动验证 UX |

## Refactoring Progress

| Phase | Content | Status |
|-------|---------|--------|
| 1 | SQL injection fix, deleteRecord fix, column names, delete UI flow, profit default text | DONE |
| 2 (Step 0) | Fix duplicate function definitions in DataCenter.cpp | DONE |
| 2 (Step 1) | AnalysisWidget real data (connect dataRequested signal) | DONE |
| 2 (Step 2) | Implement filter/search functionality | DONE |
| 2 (Step 3) | Merge updateBalanceCards redundant queries | DONE |
| 3 (Step 4) | DBHelper async model simplification | DONE |
| 3 (Step 5) | PageController integration | DONE |
| 3 (Step 6) | Introduce Repository layer | DONE |
| 3 (Step 7) | CMake configuration cleanup | DONE |
| 3 (Step 8) | Alipay CSV import | ON HOLD |
| 4 (Step 9) | Fix: update record does not refresh balance cards | DONE |
| 4 (Step 10) | Fix: AnalysisWidget category combo not connected | DONE |
| 4 (Step 11) | Remove dead code (enums, signals, commented code) | DONE |