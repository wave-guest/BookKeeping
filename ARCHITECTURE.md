# BookKeeping 架构评审与重构计划

## 项目概述

基于 Qt6 + C++17 的桌面记账应用，采用三层架构（UI 层 MyWindow、数据层 DataCenter、模型层 TradeRecord.h），通过 CMake 构建，使用 SQLite 存储。

---

## 一、项目目录结构

```
BookKeeping/
├── CMakeLists.txt                  # 顶层 CMake：包含 3 个子模块
├── QtConfig.cmake                  # 自定义 CMake 函数（setup_qt_library/setup_qt_exe/拷贝头文件）
├── build.bat                       # 构建脚本：cmake -S . -B build
├── build/                          # 构建产物（VS2022 项目 + .sln）
│
├── MainApp/                        # 可执行程序入口
│   ├── CMakeLists.txt
│   └── main.cpp                    # main() 入口，启动 QApplication
│
├── MyWindow/                       # 动态库：UI 层
│   ├── CMakeLists.txt              # 依赖 DataCenter + Qt Charts
│   ├── MainWindow.h / .cpp         # 主窗口（导航栏 + QStackedWidget 页面切换）
│   ├── AccountingWidget.h / .cpp   # 记账管理页面（表单 + 表格 + 增删改查）
│   ├── AnalysisWidget.h / .cpp     # 收支分析页面（饼图/折线图）
│   └── PageController.h / .cpp     # 分页控件（当前未集成使用）
│
├── DataCenter/                     # 动态库：数据层
│   ├── CMakeLists.txt              # 依赖 sqlite3
│   ├── TradeRecord.h               # 纯数据结构（TradeRecord, Statistics, 枚举）
│   ├── DataCenter.h / .cpp         # 外观模式 Facade：统计 + CRUD 接口
│   └── DBHelper.h / .cpp           # SQLite 数据库封装（异步任务队列）
│
└── 账单/支付宝/                    # 支付宝导出的 CSV 账单源文件（6 个年度文件）
    ├── 支付宝交易明细(20200901-20210831).csv
    ├── 支付宝交易明细(20210901-20220831).csv
    ├── 支付宝交易明细(20220901-20230831).csv
    ├── 支付宝交易明细(20230901-20240831).csv
    ├── 支付宝交易明细(20240901-20250831).csv
    └── 支付宝交易明细(20250901-20260107).csv
```

---

## 二、模块职责

| 模块 | 类型 | 职责 |
|------|------|------|
| **MainApp** | 可执行文件 (.exe) | 启动 QApplication，创建 MainWidget，设置全局字体和 HighDPI |
| **MyWindow** | 动态库 (.dll) | 所有 UI 组件：主窗口框架、导航菜单、记账表单/表格、图表分析、分页控件 |
| **DataCenter** | 动态库 (.dll) | 所有数据逻辑：SQLite 数据库初始化、交易记录增删改查、按时间范围统计查询 |
| **TradeRecord.h** | 纯头文件（数据模型） | 仅定义数据结构与枚举，无任何业务逻辑 |

---

## 三、程序启动流程

```
main.cpp
  ├─ 设置 HighDPI 属性
  ├─ 创建 QApplication
  ├─ 设置全局字体 SimHei
  ├─ 创建 MainWidget
  │     └─ 构造函数
  │           ├─ initLayout() → initLeftNav() + initRightContent()
  │           │     ├─ 左侧：导航栏（主界面 / 记账管理 / 收支分析 / 系统设置）
  │           │     └─ 右侧：QStackedWidget
  │           │           ├─ page 0: homeWidget（收支汇总卡片：总/年/月/日）
  │           │           ├─ page 1: AccountingWidget（记账表单 + 表格）
  │           │           ├─ page 2: settingsWidget（占位）
  │           │           └─ page 3: AnalysisWidget（图表）
  │           ├─ 连接导航按钮 → onSwitchContent（切换 stackedWidget 页面）
  │           ├─ dataCenter.initTables() → 创建/打开 SQLite 数据库
  │           ├─ 连接 AccountingWidget::addRecord → dataCenter.addRecord + 刷新表格
  │           ├─ 连接 AccountingWidget::updateRecord → dataCenter.updateRecord
  │           ├─ 加载所有记录到表格
  │           └─ updateBalanceCards() → 填充首页收支卡片
  └─ a.exec() → 进入事件循环
```

---

## 四、数据流

```
用户操作 UI（记账表单/筛选/搜索）
        │
        ▼
AccountingWidget（信号 addRecord/updateRecord）
        │
        ▼ 信号-槽（MainWindow 中连接）
DataCenter（外观 Facade）
        │
        ├─ addRecord()       → DBHelper::exec(INSERT SQL)
        ├─ deleteRecord()    → DBHelper::exec(DELETE SQL)
        ├─ updateRecord()    → DBHelper::exec(UPDATE SQL)
        ├─ getAllRecords()   → DBHelper::query(SELECT *)
        ├─ getStatistics()   → DBHelper::query(SUM + CASE WHEN + WHERE)
        └─ getNewRecord()    → DBHelper::query(SELECT * ORDER BY id DESC LIMIT 1)
                │
                ▼
        DBHelper（异步任务队列 + SQLite3）
                │
                ▼
        sqlite3（通过 sqlite_modern_cpp 库访问）
```

DBHelper 内部：所有数据库操作通过 `submit()` 投递到后台线程任务队列，调用方通过 `future.get()` 同步等待结果（伪异步）。

---

## 五、UI 和业务关联方式

通过 Qt 信号-槽，MainWindow 充当**中介者（Mediator）**：

- 导航按钮 clicked → onSwitchContent → 页面切换 + updateBalanceCards
- AccountingWidget::addRecord → MainWindow 槽 → dataCenter.addRecord + 刷新表格
- AccountingWidget::updateRecord → MainWindow 槽 → dataCenter.updateRecord
- updateBalanceCards → 调用 DataCenter 统计接口 → 更新首页卡片 Label

关键点：AnalysisWidget 使用硬编码模拟数据，未接入 DataCenter；PageController 未集成到任何组件。

---

## 六、应保持的设计

| 设计决策 | 理由 |
|----------|------|
| 分层架构（UI ↔ Data ↔ Model） | 三层分离清晰，TradeRecord.h 纯数据结构零依赖 |
| Facade 模式（DataCenter） | UI 层只面对一个接口，不直接接触 SQL |
| PIMPL 惯用法 | 接口与实现分离，编译隔离好 |
| 信号-槽中介通信 | 组件间松耦合，可测试性较好 |
| DBHelper 的 PRAGMA 配置 | WAL + NORMAL + busy_timeout 5000，成熟生产配置 |
| 支付宝 CSV 账单独立目录 | 数据文件与代码分离，便于扩展 |

---

## 七、应重构的设计

| 需要重构 | 原因与方向 |
|----------|------------|
| DataCenter 直接拼接 SQL | 字符串拼接是安全风险，改为参数化查询 + Repository 封装 |
| MainWindow 承担过多中介逻辑 | 信号连接、页面切换、数据刷新全部堆在构造中，引入 Controller 层或抽取连接函数 |
| AnalysisWidget 与 DataCenter 脱节 | 硬编码 mock 数据，通过信号或依赖注入接入真实统计接口 |
| AccountingWidget 筛选/搜索是空壳 | 只弹提示框，实现真实查询并打通 DataCenter |
| DBHelper 伪异步 | 后台线程 + future.get() 阻塞 = 无并发收益的复杂同步，改为纯同步或真正异步（回调/signal） |
| PageController 孤立未集成 | 完整的 UI 组件无人使用，集成到 AccountingWidget 配合分页查询 |
| updateBalanceCards 重复查询 | 12 次独立 SQL 调用，改为一次 getStatistics 返回完整 Statistics |

---

## 八、未来的维护成本

| 维护痛点 | 原因 |
|----------|------|
| SQL 注入漏洞 | 金额/备注/来源/去向字段拼接 SQL，属安全债务，必须优先修复 |
| deleteRecord 传空字符串 | exec("") 无效果，功能缺口 |
| AnalysisWidget 与数据断层 | mock 数据与实际偏差越来越大，新开发者会困惑 |
| DBHelper 线程模型难以调试 | 四重并发原语叠加，未来复杂事务/多表联查时出现死锁和竞态 |
| 数据库列名不一致 | 建表用 frome（拼写错误），查询用 from，to 是保留字 |
| 未实现分页/搜索/筛选 | 数据量增长后 UI 卡顿，用户体验急剧下降 |
| SQL 分散在各函数 | 无集中管理，后续维护者需逐函数阅读才能理解数据库结构 |
| export 符号过于宽泛 | WINDOWS_EXPORT_ALL_SYMBOLS 导出 DLL 所有符号，重构困难 |

---

## 九、重构计划

### 第一阶段：修复安全漏洞 + 功能缺口

**目标**：消除最严重的生产风险，补齐缺失的核心功能

| 任务 | 说明 |
|------|------|
| 1.1 SQL 注入修复 | 将所有 QString::arg() 拼接改为 sqlite_modern_cpp 的 << 参数化语法 |
| 1.2 补全 deleteRecord | 实现 DELETE SQL，修复 int id 与数据库主键的映射 |
| 1.3 修复数据库列名不一致 | 统一 frome → from，处理 to 保留字问题 |
| 1.4 删除记录 UI 联动 | AccountingWidget 发射删除信号 → MainWindow 调用 deleteRecord |
| 1.5 修复 profitValue 无初始文本 | 构造函数中设置默认值 ¥0.00 |

**风险**：
- 低：参数化查询需要理解 sqlite_modern_cpp 绑定语法，学习成本低
- 低：列名统一需确认无其他代码依赖旧字段
- 无：其余为纯补全，不涉及重构

---

### 第二阶段：打通数据流 + 消除伪功能

**目标**：让所有 UI 功能真实对接数据层，消除占位和 mock

| 任务 | 说明 |
|------|------|
| 2.1 AnalysisWidget 接入 DataCenter | 移除 generateMockData()，接入真实统计数据进行图表渲染 |
| 2.2 实现筛选/搜索功能 | 新增 filterRecords() 接口，AccountingWidget 信号驱动 |
| 2.3 合并 updateBalanceCards 冗余查询 | 12 次调用改为 4 次 getStatistics |
| 2.4 修复当日数据冗余调用 | 复用已有 getStatistics 结果，不重复调用 getIncome/getExpense |

**风险**：
- 中：AnalysisWidget 接真实数据后图表呈现可能变化，需视觉验证
- 中：filterRecords 涉及 SQL WHERE 动态组装，防止回退到字符串拼接
- 低：查询合并不改变语义，回归测试验证卡片数值不变

---

### 第三阶段：架构简化 + 工程化

**目标**：降低长期维护成本，统一风格，移除不必要的复杂度

| 任务 | 说明 |
|------|------|
| 3.1 DBHelper 异步模型简化 | 移除后台线程 + 任务队列，改为纯同步 + Qt 信号通知 |
| 3.2 PageController 集成 | 实现完整翻页逻辑，新增分页查询接口 getRecords(page, pageSize) |
| 3.3 引入 Repository 层 | 将 SQL 抽取到 RecordRepository，DataCenter 专注业务编排 |
| 3.4 统一 Impl 风格 | 统一使用 unique_ptr + Impl 模式 |
| 3.5 CMake 配置清理 | 移除 WINDOWS_EXPORT_ALL_SYMBOLS，显式标记公开接口 |
| 3.6 支付宝 CSV 导入模块 | 实现 CSV 解析 + 批量入库 + 去重逻辑 |

**风险**：
- 高：DBHelper 改造涉及整个数据访问层调用链，回归影响面最大
- 中：Repository 接口边界定义需谨慎，避免过度设计
- 中：分页查询改变 getAllRecords 行为，前后端需同步修改
- 低：风格统一和 CMake 清理是纯机械变更
- 中：CSV 导入涉及文件编码、日期格式、去重逻辑，需额外领域知识

---

### 阶段依赖关系

```
第一阶段（安全 + 补全）
    │
    ▼
第二阶段（打通数据流）  ← 依赖第一阶段 SQL 注入修复 + 列名统一
    │
    ▼
第三阶段（架构简化）    ← 依赖第二阶段数据流稳定
```

不建议并行执行：第三阶段对 DBHelper 的改造会波及所有调用方，必须在第一、二阶段功能验证通过后进行。

---

## 十、Git 提交规范

本项目采用 [Conventional Commits](https://www.conventionalcommits.org/) 规范管理提交信息。

### 提交格式

```
<type>: <简短描述>

<可选：详细描述正文>
```

### 类型说明

| 类型 | 使用场景 |
|------|----------|
| `feat` | 新增功能、接口、模块 |
| `fix` | 修复 bug |
| `refactor` | 重构代码（不改变外部行为） |
| `perf` | 性能优化 |
| `test` | 添加或修改测试 |
| `docs` | 文档变更（ARCHITECTURE.md 等） |
| `chore` | 构建配置、工具链、依赖变更 |
| `style` | 代码格式化、命名调整（不改变逻辑） |

### 提交粒度

- 每个逻辑变更独立提交，不混杂多个不相关的修改
- 重构类提交应分阶段进行，每个阶段一个提交
- 提交覆盖范围：仅包含源码文件（`.cpp` / `.h` / `CMakeLists.txt` / `build.bat` / `.gitignore`）

### 排除规则（不纳入提交）

| 目录/文件 | 原因 |
|-----------|------|
| `build/` | CMake 构建产物，自动生成 |
| `账单/` | 原始 CSV 数据文件，属外部数据源 |
| `*.md` | 项目文档独立管理 |

### 已提交记录参考

```
fe5773e Initial commit                              # 仅 README.md
f694f4f feat: 初始化项目核心源代码                     # 首次导入全部源码
```