# Emoji Dungeon 代码架构完整分析

> 生成日期: 2026-05-08\
> 构建: CMake + Qt 6.10.2 MinGW\
> 总文件数: \~40 个源文件(.h + .cpp)\
> 入口: main.cpp

***

## 一、项目概览

Emoji Dungeon 是一款基于 **Qt 6 / C++** 的 2D 俯视角 Roguelike 射击游戏。玩家选择职业后进入竞技场，完成 10 波怪物进攻，每波结束后获得升级机会（5选1：3个角色强化 + 2个特殊子弹）。

***

## 二、架构分层

```
┌──────────────────────────────────────────────────────────┐
│                    页面层 (Pages)                         │
│  EmojiDungeonWindow ─── QStackedWidget 管理 4 页面        │
│    ├── StartPage (开始页)                                 │
│    ├── ClassSelectPage (职业选择)                          │
│    ├── GameMainPage (游戏主页面) ← 约 2000 行，核心战斗    │
│    └── UpgradePage (升级页面)                              │
├──────────────────────────────────────────────────────────┤
│                   子模块层 (Modules) ← Phase 1 Plan B 拆分  │
│    ├── UpgradeResolver   (升级逻辑管理)                    │
│    ├── CombatCoordinator (战斗交互协调)                    │
│    └── EnemyDirector     (敌人生成与 Boss 管理)            │
├──────────────────────────────────────────────────────────┤
│                 数据层 (Data / Model)                      │
│    GameObjectData 抽象基类体系:                            │
│    ├── Player / BasicPlayer                               │
│    ├── Weapon / BasicWeapon                               │
│    ├── EnemyData / BasicEnemyData                         │
│    ├── BulletData / BasicBulletData / SpecialBulletData   │
│    └── Trait / BasicTrait                                 │
├──────────────────────────────────────────────────────────┤
│                 视图层 (View / QGraphicsItem)               │
│    GameView 抽象基类体系:                                  │
│    ├── PlayerAvatarItem (玩家图形)                         │
│    ├── EnemyView / BasicEnemyView                         │
│    └── BulletView / BasicBulletView / SpecialBulletView   │
├──────────────────────────────────────────────────────────┤
│                 配置层 (Config / Tables)                    │
│    ├── game_enums.h   (所有枚举: 职业/武器/敌人/特性/子弹) │
│    ├── game_structs.h (所有配置结构体)                     │
│    ├── game_tables.h  (所有静态数据表 + 查询函数)           │
│    └── game_data.h    (统一聚合头文件)                     │
├──────────────────────────────────────────────────────────┤
│                 基础设施层                                  │
│    ├── GameFactory    (工厂类，创建所有游戏对象)            │
│    ├── WaveManager    (波次管理，经验/等级系统)             │
│    ├── combat_utils   (战斗工具函数: 颜色/方向/伤害类型)    │
│    └── BattleArenaView (QGraphicsView，网格背景渲染)       │
├──────────────────────────────────────────────────────────┤
│                   特殊子弹系统                              │
│    ├── special_bullet_config.h (12 种子弹配置表)           │
│    ├── special_bullet_data.h/cpp (特殊子弹数据+特效逻辑)   │
│    └── special_bullet_view.h/cpp (emoji 贴图渲染)          │
├──────────────────────────────────────────────────────────┤
│                   UI 组件                                  │
│    ├── DashCooldownWidget (冲刺 CD 环形指示器)             │
│    └── player_avatar_item (玩家角色图形)                   │
└──────────────────────────────────────────────────────────┘
```

***

## 三、文件逐一分析

### 3.1 页面层

| 文件                            | 作用                                                                                                                                                                                                                                    |
| ----------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| emoji\_dungeon\_window\.h/cpp | **主窗口**，继承 QMainWindow。管理 QStackedWidget 的 4 个页面切换。持有 GameFactory 引用，负责页面间信号串联。`connectNavigation()` 是核心——连接所有页面之间的导航和升级信号。持有 `BulletStyle m_activeBulletStyle` 跟踪当前激活的特殊子弹。                                                          |
| class\_select\_page.h/cpp     | **职业选择页**，3 张职业卡片（战士/游侠/施法者）。每张卡片展示职业图标、特性列表、选中动效。带"确认"和"返回"按钮。发出 `classSelected(PlayerClassId)` 信号。                                                                                                                                  |
| game\_main\_page.h/cpp        | **游戏主页面**（最核心文件）。管理 QGraphicsScene/View 战斗场景、WASD 输入、游戏循环 timer。持有三个子模块引用（UpgradeResolver / CombatCoordinator / EnemyDirector）。`rebuildBattleScene()` 重建战斗场景，`handleBattleTick()` 每帧驱动所有逻辑。`updateStatusText()` 刷新右侧状态面板（12 个 Label）。 |
| upgrade\_page.h/cpp           | **升级页面**，左侧卡片式选项列表 + 右侧预览面板。支持 3 种选项类型：Trait（角色特性）、Weapon（武器强化）、Stat（特殊子弹）。点击确认发出 `traitSelected` / `weaponUpgradeSelected` / `bulletStyleSelected` 信号。                                                                               |

### 3.2 子模块层 (Phase 1 Plan B 拆分)

| 文件                        | 作用                                                                                                                                                                                                                       |
| ------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| upgrade\_resolver.h/cpp   | **升级逻辑管理**。`applyTrait()` 应用角色特性（追踪特性等级、计算属性变化、生成 HTML 摘要），`applyWeaponUpgrade()` 应用武器强化（额外弹道/射程/穿透/子弹大小/连击）。持有 `m_traitCounts` 和 `m_ownedTraits`。发出 `statsChanged()` 信号。                                                |
| combat\_coordinator.h/cpp | **战斗交互协调**。`handleWeaponFireRequested()` 创建子弹（含扇形扩散计算、武器升级参数叠加、特殊子弹路由），`resolveCombatCollisions()` 处理玩家子弹-敌人碰撞、敌人子弹-玩家碰撞、接触伤害，`cleanupExpiredBullets()` / `cleanupDefeatedEnemies()` 清理过期对象并发放经验。含冰冻减速/穿透/吸血/肾上腺等特性判定逻辑。 |
| enemy\_director.h/cpp     | **敌人生成与 Boss 管理**。`spawnTestEnemy()` 根据波次系列随机生成敌人（Boss 激活时只生成同系列杂兵），`spawnBossIfPending()` Boss 生成管道（含 AlienPilot 1 秒延迟），`onBossEntityCreated()` 完整的 Boss 生命周期管理（血条绑定、射击信号、召唤杂兵）。管理 Boss 血条 UI 面板。                       |

### 3.3 数据层 (Model)

#### Player 系列

| 文件                  | 类             | 继承      | 作用                                                                                                                                              |
| ------------------- | ------------- | ------- | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| player.h            | `Player`      | QObject | 玩家抽象接口。纯虚方法：`applyTrait()`, `dash()`, `dashCooldownRemaining()`, `dashCooldownTotal()` 等。信号：`dashCooldownChanged`, `healthChanged`, `defeated`。 |
| basic\_player.h/cpp | `BasicPlayer` | Player  | 玩家具体实现。管理 WASD 移动、空格冲刺（3 秒 CD）、血量/伤害/攻速/防御/最大生命属性、10 种特性叠加。`updateDash()` 追踪冷却并在变化时发射 `dashCooldownChanged` 信号。                                 |

#### Weapon 系列

| 文件                  | 类             | 继承      | 作用                                                                                           |
| ------------------- | ------------- | ------- | -------------------------------------------------------------------------------------------- |
| weapon.h            | `Weapon`      | QObject | 武器抽象接口。纯虚方法：开火/瞄准/冷却推进/特性/强化应用。信号：`fireRequested(WeaponId, origin, direction)`。              |
| basic\_weapon.h/cpp | `BasicWeapon` | Weapon  | 武器具体实现。管理冷却计时、弹道增幅/射程倍率/穿透次数/子弹大小缩放/连击计数。`advanceCooldown()` 减少冷却并在归零时发射 `fireRequested` 信号。 |

#### EnemyData 系列

| 文件                       | 类                | 继承        | 作用                                                                                                                                                          |
| ------------------------ | ---------------- | --------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| enemy\_data.h            | `EnemyData`      | QObject   | 敌人数据抽象接口。血量/速度/伤害/AI 行为/减速/蓄力/Boss 阶段查询。信号：`requestShoot`（敌人开火）、`defeated`、`requestSpawnMinion`（Boss 召唤杂兵）、`requestSuicideExplosion`。                       |
| basic\_enemy\_data.h/cpp | `BasicEnemyData` | EnemyData | 敌人具体实现。支持 5 种 AI 行为模式：**Chase**（追逐玩家）、**KeepDistance**（保持距离游走）、**Charge**（蓄力 → 高速冲刺）、**ShootAndMove**（边射击边移动）、**Boss**（多阶段 + 召唤杂兵 + 圆弧弹幕）。内含减速系统、蓄力计时、射击冷却。 |

#### BulletData 系列

| 文件                          | 类                   | 继承              | 作用                                                                                                                                   |
| --------------------------- | ------------------- | --------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| bullet\_data.h              | `BulletData`        | QObject         | 子弹数据抽象接口。方向/速度/伤害/碰撞半径/穿透/生命值/过期判定。信号：`expired`, `positionChanged`。                                                                  |
| basic\_bullet\_data.h/cpp   | `BasicBulletData`   | BulletData      | 普通子弹实现。线性飞行，超出 `射程 × 范围倍率` 后自动过期。                                                                                                    |
| special\_bullet\_data.h/cpp | `SpecialBulletData` | BasicBulletData | 特殊子弹实现。新增 3 种特效：`TrackAndAttach`（追踪 → 挂身 → 持续伤害 + 减速）、`Stun`（命中眩晕敌人 N 秒）、`KnockbackWithBonus`（击退 + 撞墙额外伤害 + 攻速加成）。支持 Hacimi 多图片循环机制。 |

#### Trait 系列

| 文件                 | 类            | 继承      | 作用                                                              |
| ------------------ | ------------ | ------- | --------------------------------------------------------------- |
| trait.h            | `Trait`      | QObject | 特性抽象接口。`applyToPlayer()` 纯虚方法。                                  |
| basic\_trait.h/cpp | `BasicTrait` | Trait   | 特性具体实现。绑定 TraitConfig 配置，委托 `player->applyTrait(traitId)` 应用效果。 |

### 3.4 视图层 (View)

#### 视图基类

| 文件            | 类          | 继承              | 作用                                                                           |
| ------------- | ---------- | --------------- | ---------------------------------------------------------------------------- |
| game\_view\.h | `GameView` | QGraphicsObject | 所有视图的抽象基类。`syncFromData()` 纯虚方法（每帧从数据模型同步）。信号：`removalRequested()`（请求从场景移除）。 |

#### 玩家视图

| 文件                         | 类                  | 继承            | 作用                                                                     |
| -------------------------- | ------------------ | ------------- | ---------------------------------------------------------------------- |
| player\_avatar\_item.h/cpp | `PlayerAvatarItem` | QGraphicsItem | 玩家角色图形。QPainter 绘制：圆形身体（渐变色填充）+ 三角瞄准指示器（朝向鼠标）+ 受击闪白效果。半径 16px，Z 值 5.0。 |

#### 敌人视图

| 文件                        | 类                | 继承        | 作用                                                                              |
| ------------------------- | ---------------- | --------- | ------------------------------------------------------------------------------- |
| enemy.h                   | `EnemyView`      | GameView  | 敌人视图抽象接口。`bindModel(EnemyData*)` 绑定数据模型。                                        |
| basic\_enemy\_view\.h/cpp | `BasicEnemyView` | EnemyView | 敌人视图实现。渲染 PNG 贴图 + 血量条（绿/黄/红三色渐变）+ QTimer 受击闪白动画。`syncFromData()` 同步位置、血量、击败状态。 |

#### 子弹视图

| 文件                           | 类                   | 继承         | 作用                                                                                                        |
| ---------------------------- | ------------------- | ---------- | --------------------------------------------------------------------------------------------------------- |
| bullet.h                     | `BulletView`        | GameView   | 子弹视图抽象接口。`bindModel(BulletData*)` 绑定数据模型。                                                                 |
| basic\_bullet\_view\.h/cpp   | `BasicBulletView`   | BulletView | 普通子弹渲染。QPainter 绘制：彩色圆形（根据武器类型） + 拖尾残影 + 前端三角箭头。                                                          |
| special\_bullet\_view\.h/cpp | `SpecialBulletView` | BulletView | 特殊子弹渲染。使用 QPixmap 加载 emoji PNG 贴图替代彩色圆形。支持 `altImagePaths` 循环切换（Hacimi 每帧随机换图）。draw 时优先渲染贴图，fallback 彩色圆。 |

#### 场景视图

| 文件                         | 类                 | 继承            | 作用                                                                                            |
| -------------------------- | ----------------- | ------------- | --------------------------------------------------------------------------------------------- |
| battle\_arena\_view\.h/cpp | `BattleArenaView` | QGraphicsView | 战斗场景视图。深色 `#2a2a2a` 背景 + `drawBackground()` 绘制网格（小格 40px 浅灰线，大格 80px 中灰线）。启用抗锯齿，禁用滚动条，居中变换锚点。 |

### 3.5 配置层

| 文件              | 作用                                                                                                                                                                                                                                                                                                                                                              |
| --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| game\_enums.h   | **所有枚举定义**。`PageId`(4 页), `BattleFlowState`(3 态), `PlayerClassId`(3 职业), `WeaponId`(3 武器), `EnemyId`(10 敌人), `EnemyBehavior`(6 种 AI 行为), `TraitId`(10 特性), `WeaponUpgradeId`(5 武器强化), `UpgradeOptionKind`(3 类: Trait/Weapon/Stat), `UpgradeRarity`(3 稀有度), `DamageVisualType`(3 伤害类型), `BulletStyle`(13 种子弹), `SpecialEffect`(4 种特效)。含 `Q_DECLARE_METATYPE` 注册。 |
| game\_structs.h | **所有配置结构体**。`WaveConfig`(波次参数), `PlayerClassConfig`(职业属性), `WeaponConfig`(武器属性), `WeaponUpgradeConfig`(武器强化属性), `BulletConfig`(子弹属性), `EnemyConfig`(敌人全部属性含 AI), `TraitConfig`(特性含 14 个浮点字段), `UpgradeOption`(升级选项), `UpgradePreviewContext`, `BulletSpecialTemplate`(特殊子弹模板)。                                                                                  |
| game\_tables.h  | **核心配置表**（约 600 行）。包含：16 级经验阈值（`10·n²` 累加公式）、`waveExpMultiplier()` 波次经验倍率（波 4+→1.1x, 波 8+→1.2x, 波 10+→1.5x）、3 职业/3 武器/5 武器强化/3 子弹配置/10 敌人/10 特性完整数据表、`kUpgradeOptions`(3 个基础选项) 和 `kAllUpgradeOptions`(15 个完整选项池)、所有 `find*Config()` 查询函数、`levelForExperience()` / `experienceToNextLevel()` 等级计算函数。                                                            |
| game\_data.h    | **统一聚合头文件**。`#include "game_enums.h"` + `"game_structs.h"` + `"game_tables.h"`，其余文件只需 `#include "game_data.h"` 即可。                                                                                                                                                                                                                                              |

### 3.6 特殊子弹系统

| 文件                        | 作用                                                                                                                                                                                                                                                                                                                                      |
| ------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| special\_bullet\_config.h | **13 种子弹模板配置表**（Normal + 12 特殊）。每种子弹绑定：`displayName`（如"哈基米"、"不是哥们！？"、"带派不老铁"）、`baseDamage/baseSpeed`、`effect`（特效枚举）+ `effectParam1/2`、`emojiText`（emoji 文字）、`defaultImagePath`（主图片资源路径）、`altImagePaths`（Hacimi 用 5 张猫图循环）、`maxLevel`。`BulletTemplateConfig::generateSpecialBulletUpgradeOptions()` 生成 12 个升级选项（kind=Stat, rarity=Epic）。 |

#### 图片-子弹映射

| 子弹     | Emoji | 图片文件                                                                                                  |
| ------ | ----- | ----------------------------------------------------------------------------------------------------- |
| Normal | —     | 无图（BasicBulletView 彩色圆）                                                                               |
| 飞刀     | 🔪    | dagger\_3d.png                                                                                        |
| 太阳法球   | ☀️    | sun\_3d.png                                                                                           |
| 月亮法球   | 🌙    | new\_moon\_3d.png                                                                                     |
| 哈基米    | 🐱    | grinning\_cat / cat\_with\_wry\_smile / pouting\_cat / weary\_cat / cat\_with\_tears\_of\_joy （5 张循环） |
| 雷霆之矛   | ⚡     | high\_voltage\_3d.png                                                                                 |
| 回力镖    | 🪃    | boomerang\_3d.png                                                                                     |
| 血之箭    | 🩸    | drop\_of\_blood\_3d.png                                                                               |
| 不是哥们！？ | 🦐    | shrimp\_3d.png                                                                                        |
| 随机礼物   | 🎁    | wrapped\_gift\_3d.png                                                                                 |
| 彗星     | ☄️    | comet\_3d.png                                                                                         |
| 带派不老铁  | 👣    | footprints\_3d.png                                                                                    |
| 火箭     | 🚀    | rocket\_3d.png                                                                                        |

#### 特效详解

| 子弹     | 特效类型               | 参数1          | 参数2        | 效果                      |
| ------ | ------------------ | ------------ | ---------- | ----------------------- |
| 哈基米    | TrackAndAttach     | 3.0 秒（挂身时长）  | 0.15（减速比例） | 追踪最近敌人→贴身后挂身持续伤害→减速 15% |
| 不是哥们！？ | Stun               | 1.5 秒（眩晕时长）  | —          | 命中眩晕敌人                  |
| 带派不老铁  | KnockbackWithBonus | 150 px（击退距离） | 0.3（攻速加成）  | 强力击退+撞墙额外伤害+攻速+30%      |

### 3.7 基础设施

| 文件                           | 作用                                                                                                                                                                                                                                                                                                                                                                                                                                              |
| ---------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| game\_factory.h/cpp          | **工厂类**。统一创建所有游戏对象：`createPlayer(PlayerClassId)` → BasicPlayer, `createStarterWeapon(PlayerClassId, owner)` → BasicWeapon + 绑定玩家, `createEnemyEntity(EnemyId, pos)` → BasicEnemyData + BasicEnemyView, `createBulletEntity(WeaponId, pos, dir)` → BasicBulletData + BasicBulletView, `createSpecialBulletData(tmpl, pos, dir)` → SpecialBulletData + SpecialBulletView, `createTrait(TraitId)` → BasicTrait。封装了 Basic\* 实现类与配置表的绑定，是配置层→实例层的桥梁。 |
| wave\_manager.h/cpp          | **波次管理器**。`BasicWaveManager`（在 .cpp 匿名命名空间中，final 实现类）管理：战斗流程状态机（Inactive→Battle→Upgrade 循环）、波次推进（10 波）、经验累积+等级判定（查表 16 级阈值）、怪物系列划分（Bizarre 1-4 波 / Undead 4-7 波 / Alien 7-10 波）、动态难度倍率（`enemyHpMultiplier = 1.05^wave`, `enemyDmgMultiplier = 1.03^wave`）、Boss 波次调度（第 4/7/9-10 波）、5 选项升级池生成（加权随机选 3 特性 + 随机选 2 特殊子弹）。信号：`battleStateChanged`, `roundChanged`, `experienceChanged`, `upgradeRequested`。                                         |
| combat\_utils.h/cpp          | **战斗工具函数**：`normalizedDirection()` 向量归一化（长度 < 0.001 退回 fallback）、`enemyBaseColor()` 统一敌人底色、`enemyHealthBarColor()` 血量条三色分段、`damageVisualTypeForWeapon()` 武器→伤害类型映射、`damageFlashColor()` 伤害闪白颜色、`bulletBaseColor()` 武器→子弹颜色映射。                                                                                                                                                                                                                   |
| dash\_cooldown\_widget.h/cpp | **冲刺 CD 可视化指示器**。52×52 自定义 QWidget，使用 QPainter + AntiAliasing 绘制环形冷却进度。**冷却中**：灰色弧形按 `remaining/total` 比例递减（<0.5s 变黄警告），⚡ 图标灰色，底部显示秒数（如"2.1"）。**就绪**：蓝色实心圆 + ⚡ 图标亮白 + QTimer 闪烁 3 次（167ms 间隔 × 6 次翻转）。通过 `WA_TransparentForMouseEvents` 不拦截鼠标事件。                                                                                                                                                                                                |

***

## 四、数据流总览

### 4.1 页面导航流

```
StartPage ──"开始游戏"──▶ ClassSelectPage ──选择职业──▶ GameMainPage
                                                        │
                                              ┌─ 战斗 ──┤
                                              │         │ 升级触发
                                              │    UpgradePage
                                              │         │ 确认选择 (5选1)
                                              └─ 继续战斗◀┘
                        战斗结束 (10波完成/死亡) → 返回 StartPage (显示结算)
```

### 4.2 主循环 (handleBattleTick, 每 16ms 一帧)

```
handleBattleTick()
  │
  ├── EnemyDirector::spawnBossIfPending(waveManager)
  │     └── 第 4/7/9 波触发，创建 BossData + BossView + 绑定信号
  │
  ├── EnemyDirector::spawnTestEnemy(maxConcurrent, bossActive, bossId)
  │     └── 随机敌人生成 → connectEnemyShootSignals (区分普通/Boss弹幕)
  │
  ├── updatePlayerMovement(delta)
  │     └── WASD 键→m_inputDirection → Player::move() → 场景边界约束
  │
  ├── updateWeaponAim()
  │     └── 鼠标位置 → 计算朝向 → Weapon::setAimDirection()
  │
  ├── Weapon::advanceCooldown(delta) → fireRequested 信号触发 →
  │     CombatCoordinator::handleWeaponFireRequested()
  │     ├── m_activeBulletStyle == Normal ?
  │     │     └── createBulletData + createBulletView → 普通子弹
  │     └── m_activeBulletStyle != Normal ?
  │           └── createSpecialBulletData + createSpecialBulletView → 特殊子弹
  │
  ├── foreach bullet: advanceFrame(dt) + advanceSpecialFrame(enemyPositions)
  ├── foreach enemy: advanceFrame(dt) + updateAI(playerPos)
  │
  ├── CombatCoordinator::resolveCombatCollisions(bullets, enemies, playerMarker)
  │     ├── 玩家子弹 vs 敌人: 伤害计算+穿透判定+冰冻减速+弹射弹查找
  │     └── 敌人子弹 vs 玩家: 伤害计算+冷却判定
  │     └── 敌人接触伤害 vs 玩家
  │
  ├── CombatCoordinator::cleanupExpiredBullets() → 移除+deleteLater
  │
  ├── CombatCoordinator::cleanupDefeatedEnemies(..., waveManager)
  │     └── 移除敌人 → addExperience(exp × expMultiplier × waveExpMultiplier)
  │                    → 吸血光环回复 → 肾上腺加速
  │                    → 如果 Boss 被击败: notifyBossDefeated()
  │
  ├── WaveManager::advanceFrame(dt)
  │     └── 波次计时 → 经验检查 → 升级触发 → upgradeRequested 信号
  │
  └── updateStatusText() → 刷新 12 个状态 Label
```

### 4.3 升级选择流

```
WaveManager::prepareUpgradeOptions()
  ├── 从 kAllUpgradeOptions 中筛选 kind==Trait → 加权随机选 3 个
  │     └── 权重: Common=60, Rare=30, Epic=10
  └── 从 BulletTemplateConfig::generateSpecialBulletUpgradeOptions() → 随机选 2 个
  └── 合并为 5 个 UpgradeOption

  ↓ upgradeRequested → EmojiDungeonWindow

UpgradePage 用户浏览 5 个卡片 → 点击确认:

  ├── kind == Trait:
  │     └── EmojiDungeonWindow: m_selectedTraits.push_back(traitId)
  │         → GameMainPage::applyTrait(traitId)
  │             → UpgradeResolver::applyTrait(traitId)
  │                 → Player::applyTrait(traitId)  (属性叠加)
  │                 → Weapon::applyTrait(traitId)  (fireRateScale)
  │
  ├── kind == Weapon:
  │     └── EmojiDungeonWindow → GameMainPage::applyWeaponUpgrade(id)
  │         → UpgradeResolver::applyWeaponUpgrade(id)
  │             → Weapon 叠加: 弹道数/射程/穿透/大小/连击
  │
  └── kind == Stat:  (特殊子弹)
        └── parse "bullet.X" → BulletStyle
            → EmojiDungeonWindow → GameMainPage::setActiveBulletStyle(style)
                → CombatCoordinator::setActiveBulletStyle(style)
                    (后续开火时路由到 SpecialBulletData/SpecialBulletView)
```

### 4.4 冲刺 CD 可视化流

```
用户按空格 → GameMainPage::eventFilter (Qt::Key_Space)
  → m_player->dash()
    → BasicPlayer::dash()
        → 设置 m_dashCooldownRemaining = 3.0
        → emit dashCooldownChanged(3.0, 3.0) ─────────┐
                                                        │
每帧 GameMainPage::handleBattleTick()                    │
  → m_player->updateDash(delta)                         │
    → BasicPlayer::updateDash()                         │
        → m_dashCooldownRemaining -= dt                 │
        → emit dashCooldownChanged(remaining, 3.0) ─────┤
                                                        ↓
                              DashCooldownWidget::updateCooldown(remaining, total)
                                → m_remainingSeconds = remaining
                                → 冷却中: 重新绘制弧形比例 + 秒数
                                → 就绪触发: 启动 QTimer 闪烁动画
                                → update() → paintEvent()
```

***

## 五、继承体系总图

```
QObject
  ├── Player (抽象)
  │     └── BasicPlayer (final)
  ├── Weapon (抽象)
  │     └── BasicWeapon (final)
  ├── EnemyData (抽象)
  │     └── BasicEnemyData (final)
  ├── BulletData (抽象)
  │     └── BasicBulletData
  │           └── SpecialBulletData (final) ← 特殊子弹扩展
  ├── Trait (抽象)
  │     └── BasicTrait (final)
  ├── WaveManager (抽象)
  │     └── BasicWaveManager (匿名, final) ← 在 wave_manager.cpp 中
  ├── GameFactory
  ├── UpgradeResolver
  ├── CombatCoordinator
  └── EnemyDirector

QGraphicsObject
  └── GameView (抽象)
        ├── EnemyView (抽象)
        │     └── BasicEnemyView (final)
        └── BulletView (抽象)
              ├── BasicBulletView (final)
              └── SpecialBulletView (final)

QGraphicsItem
  └── PlayerAvatarItem (final)

QWidget
  ├── ClassSelectPage
  ├── GameMainPage
  ├── UpgradePage
  ├── DashCooldownWidget (final)
  └── QGraphicsView
        └── BattleArenaView (final)

QMainWindow
  └── EmojiDungeonWindow (final)
```

***

## 六、关键数值汇总

### 6.1 职业属性

| 职业  | 武器    | 生命  | 移速  | 特色        |
| --- | ----- | --- | --- | --------- |
| 战士  | 豆豆发射器 | 120 | 235 | 近中距离，攻防兼备 |
| 游侠  | 散射喷发器 | 95  | 270 | 远距离输出，高机动 |
| 施法者 | 弧光法杖  | 85  | 220 | 远程高爆发，慢速  |

### 6.2 武器属性

| 武器    | 单发伤害 | 攻速(ms) | 弹速  | 弹数 | 职业  |
| ----- | ---- | ------ | --- | -- | --- |
| 豆豆发射器 | 10   | 500    | 420 | 2（平行偏移） | 战士  |
| 散射喷发器 | 7    | 700    | 380 | 3  | 游侠  |
| 弧光法杖  | 16   | 900    | 360 | 1  | 施法者 |

### 6.3 敌人属性 (10 种)

| ID          | 名称   | 生命  | 移速  | 接触伤害 | AI 行为              | 波次系列      |
| ----------- | ---- | --- | --- | ---- | ------------------ | --------- |
| Ogre        | 食人魔  | 80  | 110 | 12   | Charge(蓄力冲刺)       | -         |
| Jester      | 小丑脸  | 35  | 90  | 6    | ShootAndMove(边射边移) | -         |
| DemonLord   | 恶魔领主 | 350 | 45  | 18   | Boss(多阶段圆弧弹幕)      | 波 4 Boss  |
| SkeletonNew | 骷髅兵  | 25  | 70  | 8    | Chase(追逐)          | Undead    |
| Ghost       | 幽灵   | 20  | 85  | 5    | ShootAndMove       | Undead    |
| BoneLord    | 骨王   | 420 | 40  | 22   | Boss(蓄力冲刺+弹幕)      | 波 7 Boss  |
| Robot       | 机器人  | 45  | 58  | 8    | ShootAndMove       | Alien     |
| XenoBeast   | 异形兽  | 55  | 105 | 10   | Charge             | Alien     |
| UFO         | 飞碟   | 500 | 35  | 8    | Boss(多弹幕)          | 波 9 Boss  |
| AlienPilot  | 外星人  | 300 | 78  | 14   | Boss(二阶段)          | 波 10 Boss |

### 6.4 角色特性 (10 种)

| ID              | 名称   | 效果                      | 稀有度    |
| --------------- | ---- | ----------------------- | ------ |
| QuickHands      | 快手   | 伤害×1.15，移速×1.05         | Common |
| ThickSkin       | 厚皮   | 防御×1.20                 | Common |
| LuckyShot       | 幸运射击 | 伤害×1.25                 | Common |
| VampiricAura    | 吸血光环 | 击杀回复 10% 最大生命           | Common |
| BouncingBullet  | 弹射弹  | 命中后弹射到 180px 内敌人(70%伤害) | Rare   |
| Frostbite       | 冰冻减速 | 命中减速 40%，持续 2 秒         | Common |
| CriticalStrike  | 暴击专精 | 15% 概率 1.5 倍伤害          | Rare   |
| ExperienceBoost | 经验加成 | 击败经验+20%                | Common |
| Vitality        | 生命提升 | 最大生命+30                 | Common |
| Adrenaline      | 肾上腺  | 击杀后移速+30%，持续 3 秒        | Common |

### 6.5 武器强化 (5 种)

| ID               | 名称   | 效果             | 稀有度    |
| ---------------- | ---- | -------------- | ------ |
| ExtraProjectiles | 弹道增幅 | +1 额外弹道        | Rare   |
| RangeBoost       | 射程提升 | 飞行距离×1.30      | Common |
| Pierce           | 穿透   | 子弹穿透 1 个敌人     | Rare   |
| BulletSize       | 子弹增幅 | 碰撞半径×1.40      | Common |
| Combo            | 连击   | 每第 3 发×1.50 伤害 | Rare   |

### 6.6 等级系统 (15 级)

| 等级   | 累积经验   | 升下一级需要 |
| ---- | ------ | ------ |
| Lv1  | 0      | 10     |
| Lv2  | 10     | 40     |
| Lv3  | 50     | 90     |
| Lv4  | 140    | 160    |
| Lv5  | 300    | 250    |
| Lv6  | 550    | 360    |
| Lv7  | 910    | 490    |
| Lv8  | 1,400  | 640    |
| Lv9  | 2,040  | 810    |
| Lv10 | 2,850  | 1,000  |
| Lv11 | 3,850  | 1,210  |
| Lv12 | 5,060  | 1,440  |
| Lv13 | 6,500  | 1,690  |
| Lv14 | 8,190  | 1,960  |
| Lv15 | 10,150 | —      |

公式: 累积经验(N) = Σ(10 × i²), i=1..N-1\
波次经验倍率: 波 4+ → 1.1x, 波 8+ → 1.2x, 波 10+ → 1.5x

### 6.7 波次系统 (10 波)

| 波次   | 怪物系列         | Boss                    |
| ---- | ------------ | ----------------------- |
| 1-4  | Bizarre (怪异) | 第 4 波: 恶魔领主             |
| 4-7  | Undead (亡灵)  | 第 7 波: 骨王               |
| 7-10 | Alien (外星)   | 第 9 波: 飞碟 → 第 10 波: 外星人 |

动态难度: `enemyHP × 1.05^wave`, `enemyDMG × 1.03^wave`

***

## 七、CMake 构建结构

```
emoji_dungeon (主可执行文件)
  ├── main.cpp
  ├── 枚举/配置: game_enums.h, game_structs.h, game_tables.h, game_data.h
  ├── 抽象接口: player.h, weapon.h, enemy_data.h, enemy.h, bullet_data.h, bullet.h, trait.h, game_view.h
  ├── 基础实现: basic_player, basic_weapon, basic_enemy_data, basic_enemy_view, basic_bullet_data, basic_bullet_view, basic_trait
  ├── 页面: emoji_dungeon_window, class_select_page, game_main_page, upgrade_page
  ├── 子模块: upgrade_resolver, combat_coordinator, enemy_director
  ├── 特殊子弹: special_bullet_data, special_bullet_view, special_bullet_config
  ├── 基础设施: game_factory, wave_manager, combat_utils
  ├── UI组件: battle_arena_view, player_avatar_item, dash_cooldown_widget
  └── 资源: rescource.qrc, resources.qrc

stage1_system_test (单元测试)
  └── 包含所有生产代码 + stage1_system_test.cpp
```

***

## 八、设计模式与架构要点

1. **MV 分离**: 每个游戏实体都有 Data（数据+逻辑）和 View（渲染）两组类，通过 `bindModel()` / `syncFromData()` 连接
2. **工厂模式**: `GameFactory` 统一创建所有对象，封装配置表查询 + Basic\* 实例化
3. **策略模式**: `EnemyConfig.behavior` 字段决定 AI 行为（Chase/Charge/ShootAndMove/Boss），在 `BasicEnemyData::updateAI()` 中分支执行
4. **配置驱动**: 所有游戏数据（属性/名称/特效参数）都在 `game_tables.h` 的内联配置表中，代码只做运行时逻辑
5. **信号槽解耦**: 页面间通过 Qt Signal/Slot 通信（如 `upgradeRequested`），子模块通过 `statsChanged` 通知 UI 刷新
6. **Plan B 模块化**: 将原 1930 行的 `game_main_page.cpp` 拆分为 3 个独立子模块（UpgradeResolver / CombatCoordinator / EnemyDirector），各自管理独立职责

