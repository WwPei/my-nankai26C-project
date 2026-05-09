# Emoji Dungeon V1 → V3 版本演进全记录

> 版本跨度: V1 (2026-04-28) → V2 (2026-05-08) → V3 (2026-05-09)\
> 构建: CMake + Qt 6.10.2 MinGW / C++17\
> V1 路径: `F:\QT projects\emji_vibe_coding_V1`\
> V3 路径: `F:\QT projects\emoji_vibe_coding`

---

## 一、版本概览

| 项目 | V1 | V3 |
| ---- | -- | -- |
| **版本标识** | emoji_vibe_coding_V1 | emoji_vibe_coding |
| **最后修改日期** | 2026-04-28 | 2026-05-09 |
| **总头文件数 (.h)** | 15 | 35 |
| **总源文件数 (.cpp)** | 7 | 22 |
| **总代码行数 (估算)** | ~2,700 | ~4,500 |
| **QRC 注册资源数** | 0 | 47 张 PNG |
| **目录层级** | 1 层（全部在 src/） | 3 层（10 个子目录） |

---

## 二、文件结构重组（V2 → V3 核心变更）

### 2.1 V1 结构（扁平单层）

```
emji_vibe_coding_V1/
├── .gitignore
├── CMakeLists.txt          (5 行，仅调用 add_subdirectory)
├── LICENSE
├── README.md
├── 项目架构设计文档.md
├── docs/
│   ├── V1到V2更新对比报告.md
│   └── 项目代码分析报告.md
└── src/                    (全部 20 个源文件平铺在一起)
    ├── CMakeLists.txt
    ├── main.cpp
    ├── bullet.h
    ├── bullet_data.h
    ├── class_select_page.cpp / .h
    ├── emoji_dungeon_window.cpp / .h
    ├── enemy.h
    ├── enemy_data.h
    ├── game_data.h          (393 行，所有配置单体文件)
    ├── game_factory.cpp / .h
    ├── game_main_page.cpp / .h
    ├── game_view.h
    ├── player.h
    ├── resources.qrc        (空文件，无注册资源)
    ├── stage1_system_test.cpp
    ├── trait.h
    ├── upgrade_page.cpp / .h
    ├── wave_manager.h
    └── weapon.h
```

### 2.2 V3 结构（分层分模块，10 个子目录）

```
emoji_vibe_coding/
├── .gitignore
├── CMakeLists.txt              (147 行，Strategy A: target_include_directories)
├── resources/
│   ├── resources.qrc           (47 个 PNG 资源注册)
│   ├── images/                 (32 张图片: 敌人贴图/子弹贴图/职业图标/背景)
│   └── icons/                  (15 张图标: 升级选项 UI 图标)
├── tests/
│   └── stage1_system_test.cpp
├── docs/
│   ├── CHANGELOG_V2.md         (V1→V2 详细对比)
│   ├── CHANGELOG_V3.md         (本文档)
│   └── CODE_ANALYSIS.md        (完整代码架构分析)
└── src/
    ├── main.cpp
    ├── core/                   (6 个文件: 配置与工具层)
    │   ├── game_enums.h        (所有枚举: 106 行)
    │   ├── game_structs.h      (所有配置结构体: 137 行)
    │   ├── game_tables.h       (所有静态数据表: 568 行)
    │   ├── game_data.h         (聚合头: 4 行)
    │   ├── combat_utils.h
    │   └── combat_utils.cpp
    ├── models/
    │   ├── interfaces/         (6 个抽象基类)
    │   │   ├── player.h
    │   │   ├── weapon.h
    │   │   ├── enemy_data.h
    │   │   ├── bullet_data.h
    │   │   ├── trait.h
    │   │   └── game_view.h
    │   └── basic/              (10 个实现文件)
    │       ├── basic_player.h / .cpp
    │       ├── basic_weapon.h / .cpp
    │       ├── basic_enemy_data.h / .cpp
    │       ├── basic_bullet_data.h / .cpp
    │       └── basic_trait.h / .cpp
    ├── views/
    │   ├── interfaces/         (2 个抽象视图)
    │   │   ├── bullet.h
    │   │   └── enemy.h
    │   ├── basic_bullet_view.h / .cpp
    │   ├── basic_enemy_view.h / .cpp
    │   ├── battle_arena_view.h / .cpp
    │   ├── dash_cooldown_widget.h / .cpp
    │   └── player_avatar_item.h / .cpp
    ├── pages/                  (4 个页面, 8 个文件)
    │   ├── emoji_dungeon_window.h / .cpp
    │   ├── class_select_page.h / .cpp
    │   ├── game_main_page.h / .cpp
    │   └── upgrade_page.h / .cpp
    ├── systems/                (5 个子系统, 10 个文件)
    │   ├── game_factory.h / .cpp
    │   ├── wave_manager.h / .cpp
    │   ├── upgrade_resolver.h / .cpp
    │   ├── combat_coordinator.h / .cpp
    │   └── enemy_director.h / .cpp
    └── bullets/                (特殊子弹系统, 5 个文件)
        ├── special_bullet_config.h
        ├── special_bullet_data.h / .cpp
        └── special_bullet_view.h / .cpp
```

### 2.3 目录分类汇总

| 目录 | 文件数 | 职责 |
| ---- | ------ | ---- |
| `src/core/` | 6 | 枚举、结构体、配置表、工具函数 |
| `src/models/interfaces/` | 6 | 数据模型抽象基类 |
| `src/models/basic/` | 10 | 数据模型具体实现 |
| `src/views/interfaces/` | 2 | 视图抽象基类 |
| `src/views/` | 10 | 视图实现 + 场景 + UI 组件 |
| `src/pages/` | 8 | 4 个游戏页面 |
| `src/systems/` | 10 | 5 个游戏子系统 |
| `src/bullets/` | 5 | 特殊子弹系统 |
| `resources/images/` | 32 | 游戏贴图资源 |
| `resources/icons/` | 15 | UI 图标资源 |
| `tests/` | 1 | 单元测试 |
| `docs/` | 3 | 项目文档 |

---

## 三、枚举系统演进

### 3.1 枚举数量对比

| 枚举 | V1 | V3 | 说明 |
| ---- | -- | -- | ---- |
| `PageId` | 4 | 4 | 不变 |
| `BattleFlowState` | 3 | 3 | 不变 |
| `PlayerClassId` | 3 | 3 | 不变 |
| `WeaponId` | 3 | 3 | 不变 |
| `EnemyId` | **3** | **10** | +7 种敌人 |
| `TraitId` | **3** | **10** | +7 种特性 |
| `UpgradeOptionKind` | 3 | 3 | 不变 |
| `EnemyBehavior` | 无 | **6** | 全新 AI 行为系统 |
| `UpgradeRarity` | 无 | **3** | 全新稀有度系统 |
| `WeaponUpgradeId` | 无 | **5** | 全新武器强化系统 |
| `DamageVisualType` | 无 | **3** | 全新伤害类型系统 |
| `BulletStyle` | 无 | **13** | 全新子弹风格系统 |
| `SpecialEffect` | 无 | **4** | 全新特殊效果系统 |

### 3.2 敌人 ID 变更

```
V1 (3 种):                              V3 (10 种):
  Slime    ─── (已移除)                    Ogre          食人魔
  Bat      ─── (已移除)                    Jester        小丑脸
  Skeleton ─── (已移除)                    DemonLord     恶魔领主 (Boss)
                                           SkeletonNew   骷髅兵
                                           Ghost         幽灵
                                           BoneLord      骨王 (Boss)
                                           Robot         机器人
                                           XenoBeast     异形兽
                                           UFO           飞碟 (Boss)
                                           AlienPilot    外星人 (Boss 二阶段)
```

### 3.3 特性 ID 变更

```
V1 (3 种):                              V3 (10 种):
  QuickHands ─── 保留                      QuickHands    快手 (+伤害+移速)
  ThickSkin  ─── 保留                      ThickSkin     厚皮 (+防御)
  LuckyShot  ─── 保留                      LuckyShot     幸运射击 (+伤害)
                                           VampiricAura  吸血光环 (击杀回血)
                                           BouncingBullet 弹射弹 (命中弹射)
                                           Frostbite     冰冻减速 (命中减速)
                                           CriticalStrike 暴击专精 (概率暴击)
                                           ExperienceBoost 经验加成
                                           Vitality      生命提升 (+最大生命)
                                           Adrenaline    肾上腺 (击杀加速)
```

### 3.4 V3 新增枚举

| 枚举 | 值数量 | 说明 |
| ---- | ----- | ---- |
| `EnemyBehavior` | 6 | Chase / KeepDistance / Charge / ShootAndMove / SuicideBomb / Boss |
| `UpgradeRarity` | 3 | Common (灰) / Rare (蓝) / Epic (紫) |
| `WeaponUpgradeId` | 5 | ExtraProjectiles / RangeBoost / Pierce / BulletSize / Combo |
| `DamageVisualType` | 3 | Neutral / Rapid / Arcane |
| `BulletStyle` | 13 | Normal + 12 种特殊子弹 |
| `SpecialEffect` | 4 | None / TrackAndAttach / Stun / KnockbackWithBonus |

---

## 四、架构演进

### 4.1 V1 架构（单体文件）

```
game_data.h (393 行，枚举+结构体+配置表一体)
    ↓
game_main_page.cpp (~1,183 行，所有战斗逻辑)
    ├── 玩家移动/输入处理
    ├── 武器攻击/子弹创建
    ├── 敌人 AI/生成/碰撞检测
    ├── 特性应用/伤害计算
    └── 波次推进/经验系统
    ↓
game_factory.cpp (~859 行，创建所有对象)
```

### 4.2 V3 架构（分层 + 模块化 + 目录组织）

```
                        game_data.h (聚合头, 4 行)
                       ┌───────┼───────┐
                  game_enums.h  game_structs.h  game_tables.h
                  (106 行)      (137 行)        (568 行)
                     ↑ src/core/ 目录 ↑

game_main_page.cpp (~1,282 行，编排层)
    ├── UpgradeResolver (171 行)        ← src/systems/
    ├── CombatCoordinator (343 行)       ← src/systems/
    └── EnemyDirector (331 行)           ← src/systems/

数据层 (Model) — src/models/:
    ├── interfaces/ — 6 个抽象基类
    │   Player / Weapon / EnemyData / BulletData / Trait / GameView
    └── basic/ — 5 个具体实现（10 个文件）
        BasicPlayer / BasicWeapon / BasicEnemyData / BasicBulletData / BasicTrait

视图层 (View) — src/views/:
    ├── interfaces/ — 2 个抽象视图
    │   EnemyView / BulletView
    └── 5 个具体实现 + 2 个 UI 组件
        BasicEnemyView / BasicBulletView / BattleArenaView /
        DashCooldownWidget / PlayerAvatarItem

特殊子弹系统 — src/bullets/:
    SpecialBulletData (继承 BasicBulletData)
    SpecialBulletView (继承 BulletView，并行于 BasicBulletView)
    special_bullet_config.h (12 种子弹模板)

页面层 — src/pages/:
    EmojiDungeonWindow / ClassSelectPage / GameMainPage / UpgradePage
```

### 4.3 关键架构指标

| 指标 | V1 | V3 |
| ---- | -- | -- |
| `game_data.h` 行数 | 393 (单体) | 4 (聚合头, 实际代码在 3 个独立文件) |
| `game_main_page.cpp` 行数 | ~1,183 | ~1,282 (编排层, 逻辑委托给子模块) |
| `game_factory.cpp` 行数 | ~859 | ~205 (实现拆分到 basic_*.cpp) |
| Plan B 子模块数 | 0 | 3 (UpgradeResolver + CombatCoordinator + EnemyDirector) |
| 子模块总行数 | 0 | 845 |
| 数据类独立文件 | 0 | 10+ (各有抽象接口 + 具体实现) |
| 抽象接口文件 | 3 | 8 |
| MV 分离程度 | 无（视图逻辑混在 Factory 中） | 完整（Data + View 独立类） |
| 目录层级 | 1 层 | 3 层 |
| 子目录数 | 1 (src/) | 10 |

---

## 五、敌人系统演进

### 5.1 敌人对比

| # | V1 | V3 |
| - | -- | -- |
| 1 | Slime (史莱姆, HP 25, 速 110) | 已移除 |
| 2 | Bat (蝙蝠, HP 15, 速 180) | 已移除 |
| 3 | Skeleton (骷髅, HP 40, 速 95) | 已移除 |
| 4 | — | Ogre (食人魔, HP 80, 速 110, Charge AI) |
| 5 | — | Jester (小丑脸, HP 35, 速 90, ShootAndMove AI) |
| 6 | — | DemonLord (恶魔领主, HP 350, 速 45, Boss, 波 4 Boss) |
| 7 | — | SkeletonNew (骷髅兵, HP 25, 速 70, Chase AI) |
| 8 | — | Ghost (幽灵, HP 20, 速 85, ShootAndMove AI) |
| 9 | — | BoneLord (骨王, HP 420, 速 40, Boss, 波 7 Boss) |
| 10 | — | Robot (机器人, HP 45, 速 58, ShootAndMove AI) |
| 11 | — | XenoBeast (异形兽, HP 55, 速 105, Charge AI) |
| 12 | — | UFO (飞碟, HP 500, 速 35, Boss, 波 9 Boss) |
| 13 | — | AlienPilot (外星人, HP 300, 速 78, Boss 二阶段) |

### 5.2 AI 行为系统

| AI 行为 | V1 | V3 |
| ------- | -- | -- |
| Chase (追逐) | 所有敌人统一 | SkeletonNew |
| KeepDistance (保持距离) | 无 | 预留, 未分配 |
| Charge (蓄力冲锋) | 无 | Ogre (蓄 800ms/冲 600ms/CD 3s), XenoBeast |
| ShootAndMove (射击移动) | 无 | Jester / Ghost / Robot |
| SuicideBomb (自爆) | 无 | 预留, 未分配 |
| Boss (多阶段) | 无 | 圆弧弹幕 + 召唤杂兵 + 二阶段 |

### 5.3 波次系列系统

| 系列 | 波次 | 杂兵 | Boss | Boss 波次 |
| ---- | ---- | ---- | ---- | --------- |
| **Bizarre** (怪异) | 1-3 | Ogre / Jester | DemonLord | 波 4 |
| **Undead** (亡灵) | 4-6 | SkeletonNew / Ghost | BoneLord | 波 7 |
| **Alien** (外星) | 7-9 | Robot / XenoBeast | UFO → AlienPilot | 波 9-10 |

### 5.4 动态难度

V1: 无任何动态难度系统

V3: 每波提升难度
- 敌人 HP 倍率 = 1.05 ^ 当前波次
- 敌人伤害倍率 = 1.03 ^ 当前波次

---

## 六、升级系统演进

### 6.1 升级选项

| 维度 | V1 | V3 |
| ---- | -- | -- |
| **每轮选项数** | 3 (全 Trait) | **5** (3 Trait + 2 特殊子弹) |
| **选项类型** | Trait 一种 | Trait / Weapon / Stat (特殊子弹) |
| **稀有度系统** | 无 | Common (灰) / Rare (蓝) / Epic (紫) |
| **总选项池** | 3 个 | 15 个 Trait+Weapon 选项 + 12 种特殊子弹 |
| **武器升级** | 无 | ExtraProjectiles / RangeBoost / Pierce / BulletSize / Combo |
| **属性预览** | 无 | UpgradeAttributePreview 体系（当前值/下级值/增量变化） |
| **选项图标** | 无 | 15 个专用 PNG 图标 |

### 6.2 角色特性

| # | ID | 名称 | 效果 | 稀有度 |
| - | -- | ---- | ---- | ----- |
| 1 | QuickHands | 快手 | 伤害×1.15, 移速×1.05 | Common |
| 2 | ThickSkin | 厚皮 | 防御×1.20 | Common |
| 3 | LuckyShot | 幸运射击 | 伤害×1.25 | Common |
| 4 | VampiricAura | 吸血光环 | 击杀回复 10% 最大生命 | Common |
| 5 | BouncingBullet | 弹射弹 | 命中弹射 180px 内敌人, 70% 伤害 | Rare |
| 6 | Frostbite | 冰冻减速 | 命中减速 40%, 持续 2s | Common |
| 7 | CriticalStrike | 暴击专精 | 15% 概率 1.5x 伤害 | Rare |
| 8 | ExperienceBoost | 经验加成 | 击败经验 +20% | Common |
| 9 | Vitality | 生命提升 | 最大生命 +30 | Common |
| 10 | Adrenaline | 肾上腺 | 击杀后移速 +30%, 持续 3s | Common |

### 6.3 武器升级

| ID | 名称 | 效果 | 稀有度 |
| -- | ---- | ---- | ----- |
| ExtraProjectiles | 弹道增幅 | +1 额外弹道 | Rare |
| RangeBoost | 射程提升 | 飞行距离×1.30 | Common |
| Pierce | 穿透 | 穿透 1 个敌人 | Rare |
| BulletSize | 子弹增幅 | 碰撞半径×1.40 | Common |
| Combo | 连击 | 每第 3 发×1.50 伤害 | Rare |

---

## 七、子弹系统演进

### 7.1 基础子弹对比

| 维度 | V1 | V3 |
| ---- | -- | -- |
| **总子弹类型** | 3 (每种武器 1 种) | 15 (3 基础 + 12 特殊) |
| **PeaShooter 弹道数** | 1 | **2** (平行双发偏移 10px) |
| **子弹渲染** | 纯色圆形 QPainter | 基础子弹: 彩色圆形 + 拖尾 + 三角箭头; 特殊子弹: QPixmap emoji PNG |
| **特殊效果** | 无 | TrackAndAttach / Stun / KnockbackWithBonus |
| **穿透系统** | 无 | 子弹生命值, 穿透 1 消耗 1 点 |
| **范围倍率** | 无 | 武器升级 RangeBoost 通过 rangeMultiplier 影响 |

### 7.2 12 种特殊子弹

| # | BulletStyle | 名称 | 伤害 | 速度 | 特效 | 图片 |
| - | ----------- | ---- | ---- | ---- | ---- | ---- |
| 1 | Dagger | 飞刀 | 12 | 550 | None | dagger_3d.png |
| 2 | SunOrb | 太阳法球 | 18 | 320 | None | sun_3d.png |
| 3 | MoonOrb | 月亮法球 | 14 | 340 | None | new_moon_3d.png |
| 4 | Hacimi | 哈基米 | 8 | 300 | TrackAndAttach | grinning_cat_3d.png + 4 张备用 |
| 5 | ThunderSpear | 雷霆之矛 | 22 | 600 | None | high_voltage_3d.png |
| 6 | Boomerang | 回力镖 | 10 | 250 | None | boomerang_3d.png |
| 7 | BloodArrow | 血之箭 | 16 | 450 | None | drop_of_blood_3d.png |
| 8 | StunBullet | 不是哥们！？ | 15 | 200 | **Stun** (1.5s) | shrimp_3d.png |
| 9 | RandomGift | 随机礼物 | 5 | 350 | None | wrapped_gift_3d.png |
| 10 | Comet | 彗星 | 20 | 280 | None | comet_3d.png |
| 11 | PushBullet | 带派不老铁 | 5 | 400 | **KnockbackWithBonus** | footprints_3d.png |
| 12 | Rocket | 火箭 | 25 | 180 | None | rocket_3d.png |

### 7.3 特殊效果详解

| 特效 | 机制 | 适用子弹 |
| ---- | ---- | -------- |
| **TrackAndAttach** | 发射后追踪最近敌人 → 命中挂身 → 每帧持续伤害 + 减速 | 哈基米 (3s 挂身, 15% 减速) |
| **Stun** | 命中后目标眩晕 N 秒 (无法移动/攻击) | 不是哥们！？ (1.5s 眩晕) |
| **KnockbackWithBonus** | 命中击退 N 像素, 撞墙额外 M 倍伤害 | 带派不老铁 (150px 击退, +30%) |

---

## 八、经验与等级系统演进

### 8.1 经验阈值

| 等级 | V1 累计经验 | V3 累计经验 | V3 升下一级需要 |
| ---- | ---------- | --------- | ------------- |
| 0 | 0 | 0 | — |
| 1 | 20 | **0** | 10 |
| 2 | 45 | **10** | 40 |
| 3 | 75 | **50** | 90 |
| 4 | 110 | **140** | 160 |
| 5 | 150 | **300** | 250 |
| 6 | 195 | **550** | 360 |
| 7 | 245 | **910** | 490 |
| 8 | 300 | **1,400** | 640 |
| 9 | 360 | **2,040** | 810 |
| 10 | — (V1 封顶) | **2,850** | 1,000 |
| 11 | — | **3,850** | 1,210 |
| 12 | — | **5,060** | 1,440 |
| 13 | — | **6,500** | 1,690 |
| 14 | — | **8,190** | 1,960 |
| 15 | — | **10,150** | — |

- V1: **10 级封顶**, 最大累计经验 **360**
- V3: **15 级封顶**, 最大累计经验 **10,150** (约 V1 的 28 倍)
- V3 公式: `threshold[n] = Σ(10 × i²)`, i = 1..n-1

### 8.2 波次经验倍率

V1: 固定经验

V3: 按波次递增
- 波 1-3: 1.0x
- 波 4-7: 1.1x
- 波 8-9: 1.2x
- 波 10+: 1.5x

### 8.3 敌人独立经验值

V1: 统一 10 点

V3: 各敌人独立配置 (Ogre=15, DemonLord=80, BoneLord=100, UFO=120, AlienPilot=60 等)

---

## 九、UI 系统演进

### 9.1 新增 UI 组件

| 组件 | V1 | V3 |
| ---- | -- | -- |
| **DashCooldownWidget** | 无 | 52×52 环形 CD 指示器 (QPainter 扇形进度 + 闪烁动画) |
| **BattleArenaView** | 无 | QGraphicsView 子类, 深色 `#2a2a2a` 网格背景 (40px+80px) |
| **Boss 血条面板** | 无 | EnemyDirector 动态创建 QProgressBar + QLabel |
| **PlayerAvatarItem** | 无 | QPainter 绘制圆形角色 + 三角瞄准指示器 |
| **升级属性预览** | 无 | UpgradeAttributePreview 体系, 当前值/下级值/增量文本 |

### 9.2 升级页面

| 维度 | V1 | V3 |
| ---- | -- | -- |
| 选项数量 | 3 | **5** (3 Trait + 2 特殊子弹) |
| 卡片样式 | 简单文字 | 图标 + 稀有度边框 + 属性预览 |
| 稀有度标识 | 无 | Common (灰) / Rare (蓝) / Epic (紫) 三色边框 |
| 已拥有标记 | 无 | 已拥有特性标记"已拥有"状态 |
| 特殊子弹卡片 | 无 | emoji PNG 预览图 |

### 9.3 冲刺系统

| 维度 | V1 | V3 |
| ---- | -- | -- |
| 冲刺机制 | 无 | 空格键触发, CD 3 秒, 硬直窗口 0.15 秒 |
| 冷却可视化 | 无 | DashCooldownWidget: 蓝色扇形进度 + 剩余秒数 + 就绪闪烁 |
| CD 警告 | 无 | <0.5 秒变黄色警告色 |

---

## 十、QRC 资源系统对比

| 维度 | V1 | V3 |
| ---- | -- | -- |
| **注册资源数** | 0 (空 QRC) | **47 张 PNG** |
| **敌人贴图** | 无 (QPainter 绘制) | 10 张 (ogre/ghost/skull/robot/alien/saucer 等) |
| **子弹贴图** | 无 | 18 张 (dagger/sun/moon/cat/high_voltage/shrimp/rocket 等) |
| **升级图标** | 无 | 15 张 (quick_hands/thick_skin/extra_projectiles/combo 等) |
| **UI 资源** | 无 | 5 张 (emoji_dungeon.png/class_bg/职业图标) |

### 资源目录规划

```
resources/
├── images/    (32 PNG) ← 游戏贴图: 敌人/子弹/职业图标/背景
└── icons/     (15 PNG) ← UI 图标: 升级选项/特性/武器强化
```

---

## 十一、关键数值变化总览

| 参数 | V1 | V3 | 变化 |
| ---- | -- | -- | ---- |
| **最大波次** | 10 | 10 | 不变 |
| **每波时长** | 30s | 30s | 不变 |
| **升级选项数** | 3 | **5** | +67% |
| **最高等级** | 10 | **15** | +50% |
| **经验上限** | 360 | **10,150** | +2,719% |
| **最大同时敌人** | 10 | **15** | +50% |
| **初始敌人数量** | 3 | **5** | +67% |
| **敌人生成间隔** | 3,000ms | **2,200ms** | -27% |
| **敌人种类** | 3 | **10** | +233% |
| **Boss 波次** | 0 | **4** (波 4/7/9/10) | 全新 |
| **最高 Boss HP** | 40 (Skeleton) | **500** (UFO) | +1,150% |
| **最高单敌经验** | 10 | **120** (UFO) | +1,100% |
| **特性种类** | 3 | **10** | +233% |
| **武器强化种类** | 0 | **5** | 全新 |
| **特殊子弹种类** | 0 | **12** | 全新 |
| **AI 行为种类** | 1 (Chase) | **6** | +500% |
| **怪物系列** | 0 | **3** (Bizarre/Undead/Alien) | 全新 |
| **动态难度** | 无 | HP×1.05^wave, DMG×1.03^wave | 全新 |
| **冲刺系统** | 无 | 空格键, 3s CD | 全新 |
| **PeaShooter 弹道** | 1 | **2** (平行双发) | +100% |
| **QRC 资源** | 0 | **47 PNG** | 全新 |
| **子目录数** | 1 | **10** | +900% |
| **抽象基类** | 3 | **8** | +167% |

---

## 十二、CMake 构建系统演进

### V1 CMake

```
CMakeLists.txt (根, 5 行) → add_subdirectory(src)
  └── src/CMakeLists.txt (直接列出 15 个源文件)
```

- 简陋的 flat 结构
- 无 `AUTOMOC`/`AUTORCC` 显式配置
- 无测试目标构建

### V3 CMake

```
CMakeLists.txt (根, 147 行)
  ├── set(CMAKE_AUTOMOC ON)       ← 自动 MOC 生成
  ├── set(CMAKE_AUTORCC ON)       ← 自动 QRC 编译
  ├── set(CMAKE_CXX_STANDARD 17)  ← C++17 标准
  ├── SHARED_SOURCES 变量          ← 共享源文件列表 (76 个条目)
  ├── qt_add_executable(emoji_dungeon)
  ├── qt_add_executable(stage1_system_test)    ← 测试目标
  ├── target_include_directories (Strategy A)  ← 多目录 include 搜索
  └── target_link_libraries
```

**Strategy A: `target_include_directories`** — 通过 `target_include_directories` 注册所有子目录为 include 搜索路径，避免修改任何 `#include` 语句，保持代码兼容性。

```cmake
target_include_directories(emoji_dungeon PRIVATE
    src/core
    src/models/interfaces
    src/models/basic
    src/views/interfaces
    src/views
    src/pages
    src/systems
    src/bullets
)
```

---

## 十三、数据模型抽象层级

### V1 (无抽象接口分离)

```
QObject
  └── game_factory.cpp 匿名命名空间中的具体类
```

### V3 (完整 MV 分层)

```
QObject
  ├── Player (抽象)     → BasicPlayer (final)
  ├── Weapon (抽象)     → BasicWeapon (final)
  ├── EnemyData (抽象)  → BasicEnemyData (final)
  ├── BulletData (抽象) → BasicBulletData → SpecialBulletData (final)
  └── Trait (抽象)      → BasicTrait (final)

QGraphicsObject
  └── GameView (抽象)
        ├── EnemyView (抽象)    → BasicEnemyView (final)
        └── BulletView (抽象)   → BasicBulletView (final)
                                → SpecialBulletView (final)

QGraphicsItem → PlayerAvatarItem
QGraphicsView → BattleArenaView
QWidget → GameMainPage / DashCooldownWidget / ClassSelectPage / UpgradePage
QMainWindow → EmojiDungeonWindow
```

---

## 十四、第三方资源使用声明

### Fluent Emoji 图标资源

本项目使用了来自 [Microsoft Fluent Emoji](https://github.com/microsoft/fluentui-emoji) 的 emoji 3D 风格图标资源。

**许可证：MIT License**

```
MIT License

Copyright (c) Microsoft Corporation

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
```

**使用说明：**

- 本项目为**学术大作业**，**绝不会用于任何商业用途**
- 仅会上传至**视频网站进行游戏展示**（如 Bilibili/YouTube）
- 本项目中使用的 Fluent Emoji 资源遵守 MIT 许可证条款
- 所有资源文件保留原始版权信息
- 资源经过 PNG 格式转换后嵌入 Qt QRC 资源系统

**使用的 Fluent Emoji 资源清单：**

| 资源名称 | 文件名 | 用途 |
| -------- | ------ | ---- |
| Ogre (食人魔) | ogre_3d.png | 敌人贴图 |
| Ghost (幽灵) | ghost_3d.png | 敌人贴图 |
| Skull (骷髅) | skull_3d.png | 敌人贴图 |
| Skull and Crossbones (骷髅旗) | skull_and_crossbones_3d.png | 敌人贴图 |
| Smiling Face with Horns (恶魔) | smiling_face_with_horns_3d.png | 敌人贴图 |
| Clown Face (小丑) | clown_face_3d.png | 敌人贴图 |
| Alien (外星人) | alien_3d.png | 敌人贴图 |
| Alien Monster (异形) | alien_monster_3d.png | 敌人贴图 |
| Flying Saucer (飞碟) | flying_saucer_3d.png | 敌人贴图 |
| Robot (机器人) | robot_3d.png | 敌人贴图 |
| Dagger (飞刀) | dagger_3d.png | 特殊子弹贴图 |
| Sun (太阳) | sun_3d.png | 特殊子弹贴图 |
| New Moon (月亮) | new_moon_3d.png | 特殊子弹贴图 |
| Grinning Cat (哈基米) | grinning_cat_3d.png | 特殊子弹贴图 |
| Cat with Wry Smile | cat_with_wry_smile_3d.png | 哈基米备用图 |
| Pouting Cat | pouting_cat_3d.png | 哈基米备用图 |
| Weary Cat | weary_cat_3d.png | 哈基米备用图 |
| Cat with Tears of Joy | cat_with_tears_of_joy_3d.png | 哈基米备用图 |
| High Voltage (雷霆) | high_voltage_3d.png | 特殊子弹贴图 |
| Boomerang (回力镖) | boomerang_3d.png | 特殊子弹贴图 |
| Drop of Blood (血滴) | drop_of_blood_3d.png | 特殊子弹贴图 |
| Shrimp (虾) | shrimp_3d.png | 特殊子弹贴图 |
| Wrapped Gift (礼物) | wrapped_gift_3d.png | 特殊子弹贴图 |
| Comet (彗星) | comet_3d.png | 特殊子弹贴图 |
| Footprints (脚印) | footprints_3d.png | 特殊子弹贴图 |
| Rocket (火箭) | rocket_3d.png | 特殊子弹贴图 |
| Goblin (哥布林) | goblin_3d.png | 预留资源 |

### 其他资源

- **职业图标** (warrior_icon.png / ranger_icon.png / caster_icon.png): Fluent Emoji 人物类图标
- **背景图** (class_bg.png / emoji_dungeon.png): 自定义游戏素材

---

## 十五、测试系统演进

| 维度 | V1 | V3 |
| ---- | -- | -- |
| **测试文件** | stage1_system_test.cpp (位于 src/) | tests/stage1_system_test.cpp (独立目录) |
| **测试编译** | 与主程序混编 | 独立 `qt_add_executable` 目标 |
| **测试 Link** | 无 | `Qt6::Test` |
| **CTest 支持** | 无 | `include(CTest)` + `add_test()` |

---

## 十六、V3 新增项目文件

V3 相比 V1 新增以下项目级文件：

| 文件 | 说明 |
| ---- | ---- |
| `.gitignore` | 排除 build/、*.user、.qtcreator/ 等编译/IDE 临时文件 |
| `docs/CHANGELOG_V2.md` | V1→V2 详细变更记录 (13 章节) |
| `docs/CHANGELOG_V3.md` | 本文档 (V1→V3 全记录) |
| `docs/CODE_ANALYSIS.md` | 完整代码架构分析 (8 章节) |

V1 的 `LICENSE`、`README.md`、`项目架构设计文档.md` 为 V1 独有，V3 中不再保留（被 V3 文档体系取代）。

---

## 十七、总结

从 V1 到 V3，项目经历了两个阶段的演进：

### V1 → V2 (功能扩充)
- 敌人: 3 → 10 种 (含 4 个 Boss)
- 特性: 3 → 10 种
- 新增: 5 种武器升级 + 12 种特殊子弹 + 6 种 AI 行为 + 冲刺系统
- 等级上限: 10 → 15 级
- 架构: 单体 → MV 分离 + Plan B 三模块拆分
- 资源: 0 → 47 张 PNG (来自 Fluent Emoji)
- 代码量: ~2,700 → ~4,500 行

### V2 → V3 (工程化重组)
- 扁平单目录 (1 层) → 分层子目录 (3 层, 10 个子目录)
- CMake: Strategy A `target_include_directories` 实现零 `#include` 修改
- 35 .h + 22 .cpp 文件按职责分为 8 个代码目录
- 47 个 PNG 资源分为 `images/` 和 `icons/` 两个资源目录
- 测试文件独立到 `tests/` 目录
- 文档集中到 `docs/` 目录

V3 代表了项目的**最终形态**：功能完整、架构清晰、目录规范 —— 适合作为大作业提交和视频展示。

---

*更新日志结束。更多架构细节请参阅 [CODE_ANALYSIS.md](CODE_ANALYSIS.md) 和 [CHANGELOG_V2.md](CHANGELOG_V2.md)。*