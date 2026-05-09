# Emoji Dungeon

> 基于 **Qt 6.10 + C++17 + CMake** 的 2D 俯视角地牢战斗游戏

---

## 项目简介

Emoji Dungeon 是一款类 Roguelike 的 2D 战斗游戏。玩家选择职业后进入战斗场景，使用 WASD 移动、鼠标瞄准射击，击败不断生成的敌人。每轮结束后进入升级页面选择特性加成，逐步强化自身能力。

项目采用 **纯 Qt6 Widgets** 体系，基于 `QGraphicsScene` / `QGraphicsView` 构建战斗场景，遵循 **Data-View 分离** 的 MVVM-like 架构。

---

## 快速开始

### 环境要求

| 依赖 | 版本要求 |
|------|---------|
| Qt | **6.10+** (Core / Gui / Widgets / Test) |
| CMake | 3.16+ |
| 编译器 | 支持 C++17 (MSVC 2019+ / MinGW 8.1+ / GCC 8+ / Clang 10+) |

### 构建 & 运行

```bash
# 1. 进入项目目录
cd emoji_dungeon

# 2. 创建构建目录并生成
cmake -B build -DCMAKE_PREFIX_PATH=<你的Qt6安装路径>

# 3. 编译
cmake --build build

# 4. 运行游戏
./build/emoji_dungeon

# 5. (可选) 运行系统测试
./build/stage1_system_test
```

> **Windows Qt Creator 用户**：直接用 Qt Creator 打开 `CMakeLists.txt`，选择 Kit 后点击运行即可。

---

## 操作说明

| 操作 | 按键 |
|------|------|
| 移动 | **W / A / S / D** |
| 瞄准 | **鼠标移动** |
| 射击 | **按住鼠标左键** (松开即停) |
| 打开升级页 | 点击界面按钮 / 轮次结束自动触发 |
| 显示/隐藏网格 | 勾选右侧面板 "显示网格" |

---

## 游戏内容

### 职业

| 职业 | 生命值 | 移速 | 初始武器 | 特点 |
|------|--------|------|---------|------|
| 战士 | 120 | 235 | 豆豆发射器 | 近中距离稳健成长 |
| 游侠 | 95 | 270 | 散射喷发器 | 高机动持续输出 |
| 施法者 | 85 | 220 | 弧光法杖 | 慢速高爆发 |

### 武器

| 武器 | 基础伤害 | 射击间隔 | 弹速 | 弹数 |
|------|---------|---------|------|------|
| 豆豆发射器 | 10 | 500ms | 420 | 1 |
| 散射喷发器 | 7 | 700ms | 380 | 3 (扇形散射) |
| 弧光法杖 | 16 | 900ms | 360 | 1 |

### 敌人

| 敌人 | 生命值 | 移速 | 接触伤害 | 特点 |
|------|--------|------|---------|------|
| 史莱姆 | 25 | 110 | 5 | 基础近战单位 |
| 蝙蝠 | 15 | 180 | 4 | 高速轻量单位 |
| 骷髅 | 40 | 95 | 7 | 耐久型单位 |

---

## 项目结构

```
emoji_dungeon/
├── CMakeLists.txt              # 根构建配置（入口，委派到 src/）
├── README.md                   # 项目说明
├── .gitignore                  # Git 忽略规则
├── LICENSE                     # 许可证
│
├── src/                        # 源代码目录
│   ├── CMakeLists.txt          # 子构建配置（实际编译目标）
│   ├── main.cpp                # 程序入口
│   ├── resources.qrc           # Qt 资源清单
│   │
│   ├── game_data.h             # 全局配置（枚举/结构体/数值表）
│   │
│   ├── emoji_dungeon_window.h/.cpp  # 主窗口 & 页面导航器
│   ├── class_select_page.h/.cpp     # 职业选择页
│   ├── game_main_page.h/.cpp        # 战斗主页（游戏循环核心）
│   ├── upgrade_page.h/.cpp          # 升级选择页
│   ├── game_factory.h/.cpp          # 实体工厂 + 全部具体类实现
│   │
│   ├── player.h               # 玩家抽象接口
│   ├── weapon.h               # 武器抽象接口
│   ├── enemy.h                # 敌人视图抽象接口
│   ├── enemy_data.h           # 敌人数据抽象接口
│   ├── bullet.h               # 子弹视图抽象接口
│   ├── bullet_data.h          # 子弹数据抽象接口
│   ├── game_view.h            # 视图基类
│   ├── trait.h                # 特性（天赋）抽象接口
│   ├── wave_manager.h         # 波次管理抽象接口
│   │
│   └── stage1_system_test.cpp # 系统测试
│
└── docs/                       # 文档目录
    ├── 项目代码分析报告.md
    └── V1到V2更新对比报告.md
```

### 架构层次

```
┌─────────────────────────────┐
│  EmojiDungeonWindow (导航)   │
│  ┌── QStackedWidget ──────┐  │
│  │ Start → ClassSelect     │  │
│  │       → GameMain        │  │
│  │       → Upgrade         │  │
│  └────────────────────────┘  │
├─────────────────────────────┤
│  GameFactory (工厂)          │
│  ├── BasicPlayer             │
│  ├── BasicWeapon             │
│  ├── BasicEnemyData/View     │
│  ├── BasicBulletData/View    │
│  └── BasicTrait              │
├─────────────────────────────┤
│  抽象接口层 (纯虚类)         │
│  Player / Weapon / EnemyData │
│  BulletData / Trait / ...    │
│  GameView / EnemyView / ...  │
└─────────────────────────────┘
```

---

## 设计特点

- **Data-View 完全分离**：逻辑层（QObject 派生）与渲染层（QGraphicsObject 派生）通过纯虚接口隔离，信号槽绑定同步
- **配置驱动**：全部游戏数值集中在 `game_data.h`，新增职业/武器/敌人只需添加一行配置
- **工厂模式**：`GameFactory` 统一管理所有实体创建，调用方不感知具体类
- **时间驱动帧循环**：基于 `deltaTime` 的位移和冷却计算，帧率无关
- **暗色主题**：内联 QSS 样式表，自定义 QPainter 绘制外观

---

## 测试

项目包含 5 个系统级测试用例（`stage1_system_test.cpp`）：

| 测试 | 覆盖内容 |
|------|---------|
| `wasdMovementMovesPlayer` | WASD 键盘移动 |
| `mouseAimAndLeftClickCreateBullets` | 鼠标瞄准 & 射击 |
| `enemiesSpawnOverTimeAndStayWithinCap` | 敌人生成 & 上限控制 |
| `combatCollisionDealsDamage` | 子弹-敌人 & 敌人-玩家双向碰撞伤害 |
| `leavingGamePagePausesBattleTimer` | 离开战斗页后计时器停止 |

---

## 许可证

MIT License
