# Emoji Dungeon V1 → V2 更新日志

> 版本跨度: V1 (2026-04-28) → V2 (2026-05-09)\
> 构建: CMake + Qt 6.10.2 MinGW\
> V1 路径: `F:\QT projects\emji_vibe_coding_V1`\
> V2 路径: `f:\QT projects\emoji_vibe_coding`

---

## 一、版本号信息

| 项目 | V1 | V2 |
| ---- | -- | -- |
| **版本标识** | emoji_vibe_coding_V1 | emoji_vibe_coding |
| **最后修改日期** | 2026-04-28 | 2026-05-09 |
| **总源文件数 (.h)** | 15 | 35 |
| **总源文件数 (.cpp)** | 7 | 22 |
| **总代码行数 (估算)** | ~2,700 | ~4,500 |

---

## 二、文件结构变更

### 2.1 新增文件

| 文件 | 行数 (.h/.cpp) | 说明 |
| ---- | ------------- | ---- |
| `game_enums.h` | 106 / -- | 从 `game_data.h` 拆出的所有枚举定义，新增 `EnemyBehavior`, `WeaponUpgradeId`, `UpgradeRarity`, `DamageVisualType`, `BulletStyle`, `SpecialEffect` |
| `game_structs.h` | 137 / -- | 从 `game_data.h` 拆出的所有配置结构体，新增 `WeaponUpgradeConfig`, `BulletSpecialTemplate`, `UpgradePreviewContext`, `UpgradeAttributePreview` |
| `game_tables.h` | 568 / -- | 从 `game_data.h` 拆出的所有静态数据表，包含经验阈值表、波次经验倍率函数、10 敌人/10 特性/5 武器强化/15 升级选项的完整配置 |
| `game_data.h` | 4 / -- | 统一聚合头文件，聚合 `game_enums.h` + `game_structs.h` + `game_tables.h` |
| `player.h` | 47 / -- | 玩家抽象基类，声明 `dash()`、`dashCooldownRemaining()` 等冲刺接口，信号 `dashCooldownChanged` |
| `basic_player.h` / `basic_player.cpp` | 60 / 214 | 玩家具体实现，包含冲刺系统（空格键、3 秒 CD、0.15 秒硬直窗口） |
| `weapon.h` | 39 / -- | 武器抽象基类，新增 `applyWeaponUpgrade()` 及各武器升级接口（弹道增幅/穿透/连击等） |
| `basic_weapon.h` / `basic_weapon.cpp` | 54 / 175 | 武器具体实现，支持弹道增幅、穿透计数、连击系统、子弹大小缩放 |
| `enemy.h` | 13 / -- | 敌人视图抽象接口 |
| `enemy_data.h` | 45 / -- | 敌人数据抽象接口，信号：`requestShoot`、`requestSpawnMinion`、`requestSuicideExplosion` |
| `basic_enemy_data.h` / `basic_enemy_data.cpp` | 77 / 339 | 敌人数据具体实现，支持 6 种 AI 行为（Chase/KeepDistance/Charge/ShootAndMove/SuicideBomb/Boss） |
| `basic_enemy_view.h` / `basic_enemy_view.cpp` | 29 / 168 | 敌人视图实现，PNG 贴图渲染 + 血量条 + 受击闪白 |
| `bullet.h` | 13 / -- | 子弹视图抽象接口 |
| `bullet_data.h` | 39 / -- | 子弹数据抽象接口，支持穿透计数、生命值 |
| `basic_bullet_data.h` / `basic_bullet_data.cpp` | 52 / 79 | 普通子弹实现，线性飞行 + 范围倍率过期判定 |
| `basic_bullet_view.h` / `basic_bullet_view.cpp` | 20 / 168 | 普通子弹渲染，彩色圆形 + 拖尾残影 |
| `special_bullet_data.h` / `special_bullet_data.cpp` | 38 / 110 | 特殊子弹数据实现，3 种特效：TrackAndAttach / Stun / KnockbackWithBonus |
| `special_bullet_view.h` / `special_bullet_view.cpp` | 22 / 125 | 特殊子弹视图，QPixmap 加载 emoji PNG 贴图替代彩色圆形 |
| `special_bullet_config.h` | 48 / -- | 12 种特殊子弹模板配置表 + `generateSpecialBulletUpgradeOptions()` |
| `trait.h` | 17 / -- | 特性抽象接口 |
| `basic_trait.h` / `basic_trait.cpp` | 16 / 30 | 特性具体实现，绑定 TraitConfig 委托 player->applyTrait() |
| `game_view.h` | 13 / -- | 所有视图的抽象基类，`syncFromData()` 纯虚方法 |
| `combat_coordinator.h` / `combat_coordinator.cpp` | 41 / 343 | Plan B 拆分：战斗交互协调模块——子弹创建、碰撞解决、清理逻辑 |
| `enemy_director.h` / `enemy_director.cpp` | 42 / 331 | Plan B 拆分：敌人生成与 Boss 管理模块——波次系列划分、Boss 管道、血条 UI |
| `upgrade_resolver.h` / `upgrade_resolver.cpp` | 33 / 171 | Plan B 拆分：升级逻辑管理模块——特性追踪、武器升级应用、属性变化摘要 |
| `combat_utils.h` / `combat_utils.cpp` | 11 / 71 | 战斗工具函数：方向归一化、颜色查询、伤害类型判定 |
| `dash_cooldown_widget.h` / `dash_cooldown_widget.cpp` | 23 / 108 | 冲刺 CD 环形指示器 UI 组件（QPainter 绘制扇形进度 + 就绪闪烁动画） |
| `battle_arena_view.h` / `battle_arena_view.cpp` | 12 / 61 | 战斗场景视图，深色网格背景渲染（小格 40px + 大格 80px） |
| `player_avatar_item.h` / `player_avatar_item.cpp` | 14 / 85 | 玩家角色图形项，圆形身体 + 三角瞄准指示器 + 受击闪白 |

### 2.2 修改文件

| 文件 | V1 行数 | V2 行数 | 变更说明 |
| ---- | ------ | ------ | -------- |
| `game_main_page.h` | 93 | 123 | 新增 3 个子模块指针（UpgradeResolver / CombatCoordinator / EnemyDirector），DashCooldownWidget 引用，BulletStyle 跟踪，Boss 相关状态字段 |
| `game_main_page.cpp` | 1,183 | 1,282 | 核心战斗逻辑大幅重构：引入 `rebuildBattleScene()`、大量逻辑委托给子模块，Boss 管理流程、冲刺集成、状态面板扩展 |
| `game_factory.h` | 53 | 59 | 新增 `BulletEntity`/`EnemyEntity` 结构体，`createSpecialBulletData()`/`createSpecialBulletView()` 工厂方法 |
| `game_factory.cpp` | 859 | 205 | 大幅精简：匿名命名空间中的具体类实现拆分到 `basic_*.cpp` 独立文件 |
| `wave_manager.h` | 34 | 65 | 从 V1 简单接口扩展为完整抽象基类：波次系列 `MonsterSeries`、Boss 管道 `pendingBosses()`、动态难度倍率、`requestSpawnMinion` 信号 |
| `wave_manager.cpp` | -- | 430 | 完全重写：支持 3 大怪物系列（Bizarre/Undead/Alien）、Boss 波次判定、动态难度 (1.05^wave HP, 1.03^wave DMG) |
| `upgrade_page.h` | 22 | 69 | 大幅扩展：支持 5 选 1（3 trait + 2 特殊子弹），属性变化预览面板，稀有度标记 |
| `upgrade_page.cpp` | 56 | -- | 重写，配合新升级系统 |
| `emoji_dungeon_window.h` | 31 | 34 | 新增 `m_activeBulletStyle` 状态跟踪 |
| `class_select_page.h` | 13 | 40 | 扩展职业卡片 UI |
| `emoji_dungeon_window.cpp` | 122 | -- | 导航信号增加特殊子弹路由 |

### 2.3 保留不变文件

| 文件 | 说明 |
| ---- | ---- |
| `main.cpp` | 12 行，入口文件基本不变 |
| `stage1_system_test.cpp` | 测试文件，跟随架构适配 |
| `CMakeLists.txt` | 构建配置，新增源文件编译项 |

### 2.4 V1 特有（V2 已不存在/合并）

| 文件 | 行数 | V2 中的处理 |
| ---- | ---- | ---------- |
| `game_data.h` (V1 单文件版) | 393 | 拆分为 `game_enums.h`(106) + `game_structs.h`(137) + `game_tables.h`(568) + `game_data.h`(4) 聚合头 |

---

## 三、枚举系统详细对比

### 3.1 枚举变更总览

| 枚举 | V1 | V2 | 变更 |
| ---- | -- | -- | ---- |
| `PageId` | 4 值 | 4 值 | 不变 |
| `BattleFlowState` | 3 值 | 3 值 | 不变 |
| `PlayerClassId` | 3 值 | 3 值 | 不变 |
| `WeaponId` | 3 值 | 3 值 | 不变 |
| `EnemyId` | **3 值** | **10 值** | 从 Slime/Bat/Skeleton → Ogre/Jester/DemonLord/SkeletonNew/Ghost/BoneLord/Robot/XenoBeast/UFO/AlienPilot |
| `TraitId` | **3 值** | **10 值** | 从 QuickHands/ThickSkin/LuckyShot → 新增 VampiricAura/BouncingBullet/Frostbite/CriticalStrike/ExperienceBoost/Vitality/Adrenaline |
| `UpgradeOptionKind` | 3 值 | 3 值 | 不变（Trait/Weapon/Stat） |

### 3.2 V2 新增枚举

| 枚举 | 值数量 | 说明 |
| ---- | ----- | ---- |
| `EnemyBehavior` | **6 值** | Chase（追逐）/ KeepDistance（保持距离）/ Charge（冲锋）/ ShootAndMove（射击移动）/ SuicideBomb（自爆）/ Boss（多阶段首领） |
| `UpgradeRarity` | **3 值** | Common（普通）/ Rare（稀有）/ Epic（史诗），控制升级选项品质边框 |
| `WeaponUpgradeId` | **5 值** | ExtraProjectiles（弹道增幅）/ RangeBoost（射程提升）/ Pierce（穿透）/ BulletSize（子弹增幅）/ Combo（连击） |
| `DamageVisualType` | **3 值** | Neutral（中性）/ Rapid（快速）/ Arcane（奥术），控制伤害受击闪色 |
| `BulletStyle` | **13 值** | Normal / Dagger / SunOrb / MoonOrb / Hacimi / ThunderSpear / Boomerang / BloodArrow / StunBullet / RandomGift / Comet / PushBullet / Rocket |
| `SpecialEffect` | **4 值** | None / TrackAndAttach（追踪挂身）/ Stun（眩晕）/ KnockbackWithBonus（击退+撞墙奖励） |

---

## 四、配置结构体详细对比

### 4.1 EnemyConfig 扩展

| 字段 | V1 | V2 | 说明 |
| ---- | -- | -- | ---- |
| `experienceValue` | 无 | + | 每个敌人的经验值独立配置（而非统一 10 点） |
| `imagePath` | 无 | + | PNG 贴图资源路径 |
| `behavior` | 无 | + | AI 行为枚举 |
| `keepDistanceMin` | 无 | + | KeepDistance 行为的最小保持距离 |
| `chargeUpMs` / `chargeDurationMs` / `chargeSpeedMult` / `chargeCooldownMs` | 无 | + | Charge 行为的蓄力/冲锋/冷却参数 |
| `shootIntervalMs` / `shootBulletCount` / `bulletSpeed` / `bulletDamage` / `shootTriggerDistance` | 无 | + | ShootAndMove/Boss 行为的射击参数 |
| `slowImmune` / `knockbackImmune` | 无 | + | Boss 的减速/击退免疫标志 |
| `suicideRadius` / `suicideDamage` | 无 | + | SuicideBomb 行为的爆炸范围/伤害 |

### 4.2 TraitConfig 扩展

| 字段 | V1 | V2 | 说明 |
| ---- | -- | -- | ---- |
| `damageMultiplier` / `defenseMultiplier` / `speedMultiplier` | 3 字段 | 3 字段 | 保留 V1 基础三属性 |
| `extraMaxHealth` | 无 | + | 固定生命增加（Vitality: +30） |
| `criticalChance` / `criticalMultiplier` | 无 | + | 暴击概率/倍率（CriticalStrike: 15%/1.5x） |
| `healOnKillPercent` | 无 | + | 击杀回血比例（VampiricAura: 10%） |
| `expMultiplier` | 无 | + | 经验获取倍率（ExperienceBoost: 1.20x） |
| `slowFactor` / `slowDuration` | 无 | + | 减速因子/持续秒数（Frostbite: 0.60/2 秒） |
| `killSpeedBuff` / `killSpeedBuffDuration` | 无 | + | 击杀移速加成（Adrenaline: 0.30/3 秒） |
| `bounceSearchRadius` / `bounceDamageRetention` | 无 | + | 弹射搜索范围/伤害保留率（BouncingBullet: 180px/70%） |

### 4.3 V2 新增结构体

| 结构体 | 说明 |
| ------ | ---- |
| `WeaponUpgradeConfig` | 武器升级配置：额外弹道数、射程倍率、穿透计数、子弹大小缩放、连击间隔/倍率、属性倍率 |
| `BulletSpecialTemplate` | 特殊子弹模板：`BulletStyle` 样式、显示名称、基础伤害/速度、`SpecialEffect` 特效类型及参数、emoji 文字、图片路径及备用图列表、最大等级 |
| `UpgradePreviewContext` | 升级预览上下文：当前职业、武器、已拥有特性列表 |
| `UpgradeAttributePreview` / `UpgradeAttributePreviews` | 属性变化预览数据：标签、当前值、下级值、增量文本、变化标志、正负面标志 |
| `UpgradeOption` 新增字段 | `weaponUpgradeId`, `rarity`(UpgradeRarity), `level`, `iconPath` |
| `WaveConfig` 数值调整 | `maxConcurrentEnemies`: 10→15, `initialEnemyCount`: 3→5, `enemySpawnIntervalMs`: 3000→2200, `upgradeSelectionCount`: 3→5 |

---

## 五、敌人系统详细对比

### 5.1 敌人列表

| # | V1 EnemyId | V1 名称 | V2 EnemyId | V2 名称 | AI 行为 | HP | 速度 | 伤害 | 经验 | 波次系列 |
| - | ---------- | ------ | ---------- | ------ | ------ | -- | ---- | ---- | ---- | ------- |
| 1 | Slime | 史莱姆 | -- | -- | -- | -- | -- | -- | -- | -- |
| 2 | Bat | 蝙蝠 | -- | -- | -- | -- | -- | -- | -- | -- |
| 3 | Skeleton | 骷髅 | -- | -- | -- | -- | -- | -- | -- | -- |
| -- | -- | -- | Ogre | 食人魔 | Charge | 80 | 110 | 12 | 15 | Bizarre (波 1-3) |
| -- | -- | -- | Jester | 小丑脸 | ShootAndMove | 35 | 90 | 6 | 12 | Bizarre (波 1-3) |
| -- | -- | -- | DemonLord | 恶魔领主 | Boss | 350 | 45 | 18 | 80 | Bizarre (波 4 Boss) |
| -- | -- | -- | SkeletonNew | 骷髅兵 | Chase | 25 | 70 | 8 | 12 | Undead (波 4-6) |
| -- | -- | -- | Ghost | 幽灵 | ShootAndMove | 20 | 85 | 5 | 12 | Undead (波 4-6) |
| -- | -- | -- | BoneLord | 骨王 | Boss | 420 | 40 | 22 | 100 | Undead (波 7 Boss) |
| -- | -- | -- | Robot | 机器人 | ShootAndMove | 45 | 58 | 8 | 15 | Alien (波 7-9) |
| -- | -- | -- | XenoBeast | 异形兽 | Charge | 55 | 105 | 10 | 15 | Alien (波 7-9) |
| -- | -- | -- | UFO | 飞碟 | Boss | 500 | 35 | 8 | 120 | Alien (波 10 Boss 一阶段) |
| -- | -- | -- | AlienPilot | 外星人 | Boss | 300 | 78 | 14 | 60 | Alien (波 10 Boss 二阶段) |

### 5.2 AI 行为系统

V1 所有敌人统一为简单追逐玩家，V2 引入 6 种差异化 AI：

| AI 行为 | 机制描述 | 使用者 |
| ------- | -------- | ------ |
| **Chase** | 始终向玩家移动 | SkeletonNew |
| **KeepDistance** | 与玩家保持最小距离，过近则后退 | （预留，未分配） |
| **Charge** | 蓄力后高速向玩家冲刺，有冷却时间 | Ogre（蓄力 800ms / 冲锋 600ms / CD 3s）、XenoBeast（蓄力 500ms / 冲锋 700ms / CD 2.5s） |
| **ShootAndMove** | 边射击边移动，子弹以玩家为目标 | Jester（2s 间隔 / 1 发 / 170 速）、Ghost（1.8s / 3 发 / 170 速）、Robot（2.2s / 1 发 / 190 速） |
| **SuicideBomb** | 接近玩家后自爆造成范围伤害 | （预留，未分配） |
| **Boss** | 多阶段机制：圆弧弹幕射击 + 召唤杂兵 | DemonLord（8 弹 / 2.5s 间隔）、BoneLord（10 弹 + 蓄力冲锋）、UFO（12 弹 + 召唤 Robot/XenoBeast）、AlienPilot（10 弹 + UFO 隐藏） |

### 5.3 波次系列系统

V2 将 10 波分为 3 个怪物系列，每系列包含 2 种普通敌人 + 1 个 Boss：

| 系列 | 波次范围 | 普通敌人 | Boss | Boss 波次 |
| ---- | ------- | -------- | --- | -------- |
| **Bizarre**（怪异） | 1-3 | Ogre / Jester | DemonLord | 波 4 |
| **Undead**（亡灵） | 4-6 | SkeletonNew / Ghost | BoneLord | 波 7 |
| **Alien**（外星） | 7-9 | Robot / XenoBeast | UFO → AlienPilot | 波 10 |

### 5.4 动态难度系统

V2 引入波次动态难度倍率：

- **敌人 HP 倍率** = 1.05 ^ 当前波次
- **敌人伤害倍率** = 1.03 ^ 当前波次

---

## 六、特性与升级系统详细对比

### 6.1 特性对比

| # | V1 TraitId | V1 效果 | V2 TraitId | V2 效果 | 稀有度 |
| - | ---------- | ------- | ---------- | ------- | ----- |
| 1 | QuickHands | 伤害 1.15x / 速度 1.05x | QuickHands | 同 V1 | Common |
| 2 | ThickSkin | 防御 1.20x | ThickSkin | 同 V1 | Common |
| 3 | LuckyShot | 伤害 1.25x | LuckyShot | 同 V1 | Common |
| -- | -- | -- | VampiricAura | 击败敌人回复 10% 最大生命 | Common |
| -- | -- | -- | BouncingBullet | 子弹命中弹射到 180px 内最近敌人，保留 70% 伤害 | Rare |
| -- | -- | -- | Frostbite | 子弹命中使敌人减速 40%，持续 2 秒 | Common |
| -- | -- | -- | CriticalStrike | 15% 概率造成 1.5 倍伤害 | Rare |
| -- | -- | -- | ExperienceBoost | 击败敌人经验 +20% | Common |
| -- | -- | -- | Vitality | 最大生命值 +30 点 | Common |
| -- | -- | -- | Adrenaline | 击杀后移速 +30%，持续 3 秒 | Common |

### 6.2 武器升级系统（V2 新增）

V1 没有武器升级概念，V2 新增 5 种：

| 武器升级 ID | 名称 | 效果 | 稀有度 |
| ---------- | ---- | ---- | ----- |
| ExtraProjectiles | 弹道增幅 | 增加 1 条额外弹道 | Rare |
| RangeBoost | 射程提升 | 子弹飞行距离 +30% | Common |
| Pierce | 穿透 | 子弹穿透 1 个敌人后继续飞行 | Rare |
| BulletSize | 子弹增幅 | 子弹碰撞半径 +40% | Common |
| Combo | 连击 | 每第 3 发子弹造成额外 50% 伤害 | Rare |

### 6.3 升级选项系统

| 维度 | V1 | V2 |
| ---- | -- | -- |
| **每轮选项数** | 3 | **5**（3 个 Trait + 2 个特殊子弹） |
| **选项类型** | Trait / Weapon / Stat | Trait / Weapon / Stat |
| **稀有度系统** | 无 | Common / Rare / Epic 三档 |
| **选项池** | 3 个基础选项 | 15 个完整选项（10 trait + 5 weapon upgrade） |
| **特殊子弹选项** | 无 | 12 个特殊子弹作为 kind=Stat 选项（稀有度 Epic） |
| **属性预览** | 无 | `UpgradeAttributePreview` 体系，支持当前值/下级值/增量变化文本 |
| **图标** | 无 | 每个选项有 `iconPath` 指向 `:/images/upgrade_icons/*.png` |

---

## 七、子弹系统详细对比

### 7.1 基础子弹对比

| 维度 | V1 | V2 |
| ---- | -- | -- |
| **子弹类型数** | 3（每种武器 1 种） | 3（基础）+ 12（特殊）= 15 种 |
| **PeaShooter 弹道数** | **1** | **2**（平行双发） |
| **SpreadBlaster 弹道数** | 3 | 3（不变） |
| **ArcWand 弹道数** | 1 | 1（不变） |
| **子弹视图渲染** | 纯色圆形 QPainter | 基础子弹保持圆形渲染；特殊子弹使用 QPixmap emoji PNG |
| **特殊效果** | 无 | TrackAndAttach / Stun / KnockbackWithBonus |
| **子弹生命值系统** | 无 | 支持穿透计数：子弹有生命值，穿透 1 个消耗 1 点 |
| **范围倍率** | 无 | 武器升级"射程提升"通过 `rangeMultiplier` 影响子弹存活距离 |

### 7.2 12 种特殊子弹一览

| # | BulletStyle | 名称 | 伤害 | 速度 | 特效 | 图片资源 |
| - | ----------- | ---- | ---- | ---- | ---- | -------- |
| 1 | Dagger | 飞刀 | 12 | 550 | None | `dagger_3d.png` |
| 2 | SunOrb | 太阳法球 | 18 | 320 | None | `sun_3d.png` |
| 3 | MoonOrb | 月亮法球 | 14 | 340 | None | `new_moon_3d.png` |
| 4 | Hacimi | 哈基米 | 8 | 300 | TrackAndAttach (追踪 3 秒 + 0.15x 持续伤害) | `grinning_cat_3d.png` + 4 张备用猫图循环 |
| 5 | ThunderSpear | 雷霆之矛 | 22 | 600 | None | `high_voltage_3d.png` |
| 6 | Boomerang | 回力镖 | 10 | 250 | None | `boomerang_3d.png` |
| 7 | BloodArrow | 血之箭 | 16 | 450 | None | `drop_of_blood_3d.png` |
| 8 | StunBullet | 不是哥们！？ | 15 | 200 | **Stun**（命中眩晕 1.5 秒） | `shrimp_3d.png` |
| 9 | RandomGift | 随机礼物 | 5 | 350 | None | `wrapped_gift_3d.png` |
| 10 | Comet | 彗星 | 20 | 280 | None | `comet_3d.png` |
| 11 | PushBullet | 带派不老铁 | 5 | 400 | **KnockbackWithBonus**（击退 150 像素 + 撞墙额外 0.3x 伤害） | `footprints_3d.png` |
| 12 | Rocket | 火箭 | 25 | 180 | None | `rocket_3d.png` |

### 7.3 特殊效果机制详解

| 特效 | 机制 | 示例 |
| ---- | ---- | ---- |
| **TrackAndAttach** | 发射后追踪最近敌人 → 命中后挂身 → 每帧对挂身目标造成持续伤害 + 减速效果 | 哈基米：追踪 3 秒，每帧 0.15 倍基础伤害 |
| **Stun** | 命中后目标进入眩晕状态 `param1` 秒（无法移动/攻击） | 不是哥们！？：眩晕 1.5 秒 |
| **KnockbackWithBonus** | 命中后将敌人向子弹方向击退 `param1` 像素，若击退途中碰到墙壁则额外造成 `param2` 倍总伤害 | 带派不老铁：击退 150px，撞墙 +30% |

---

## 八、架构演进

### 8.1 V1 架构（单体）

```
game_data.h (393 行，所有枚举+结构体+配置表一体)
    ↓
game_main_page.cpp (~1,183 行，所有逻辑在一个文件中)
    ├── 玩家移动/输入处理
    ├── 武器攻击/子弹创建
    ├── 敌人 AI/生成/碰撞检测
    ├── 特性应用/伤害计算
    └── 波次推进/经验系统
    ↓
game_factory.cpp (~859 行，创建所有对象)
```

### 8.2 V2 架构（分层 + 模块化）

```
                        game_data.h (聚合头, 4 行)
                       ┌───────┼───────┐
                  game_enums.h  game_structs.h  game_tables.h
                  (106 行)      (137 行)        (568 行)

game_main_page.cpp (~1,282 行，编排层)
    ├── UpgradeResolver (171 行)        ← 升级逻辑独立
    ├── CombatCoordinator (343 行)       ← 战斗交互独立
    └── EnemyDirector (331 行)           ← 敌人生成/Boss 独立

数据层 (Model):
    Player(47) → BasicPlayer(60+214)
    Weapon(39) → BasicWeapon(54+175)
    EnemyData(45) → BasicEnemyData(77+339)
    BulletData(39) → BasicBulletData(52+79) → SpecialBulletData(38+110)
    Trait(17) → BasicTrait(16+30)

视图层 (View):
    GameView(13)
    ├── PlayerAvatarItem(14+85)
    ├── EnemyView(13) → BasicEnemyView(29+168)
    ├── BulletView(13) → BasicBulletView(20+168)
    └── BulletView(13) → SpecialBulletView(22+125)  [并行继承]

基础设施:
    GameFactory(59+205)    ← 工厂类，创建所有游戏对象
    WaveManager(65+430)    ← 波次管理，经验/等级/动态难度
    combat_utils(11+71)    ← 方向归一化/颜色查询工具
    BattleArenaView(12+61) ← 网格背景战斗场景

特殊子弹系统:
    special_bullet_config.h(48)  ← 12 种子弹模板配置表
    special_bullet_data.h/cpp    ← 特殊子弹数据+特效逻辑
    special_bullet_view.h/cpp    ← emoji PNG 渲染

新增 UI 组件:
    DashCooldownWidget(23+108)   ← 冲刺 CD 环形指示器
```

### 8.3 Plan B 拆分前后对比

| 指标 | 拆分前 (V1) | 拆分后 (V2) |
| ---- | ---------- | ---------- |
| `game_main_page.cpp` 行数 | 1,183 | 1,282 |
| 子模块独立文件 | 0 | 3（UpgradeResolver + CombatCoordinator + EnemyDirector） |
| 子模块总行数 | 0 | 845 |
| `game_factory.cpp` 行数 | 859 | 205（拆分到 basic_*.cpp） |
| 数据类独立文件 | 0 | 10+（player / weapon / enemy / bullet / trait 各有抽象+实现） |

---

## 九、UI 系统变更

### 9.1 新增 UI 组件

| 组件 | 说明 |
| ---- | ---- |
| **DashCooldownWidget** | 冲刺 CD 环形指示器。QPainter 绘制：灰色底环 + 蓝色进度扇区（从 3 秒倒计时），就绪时显示完整蓝色环 + 短暂闪烁动画。`sizeHint` = 48x48，放置于战斗场景左上角。 |
| **BattleArenaView** | 继承 QGraphicsView，重写 `drawBackground()`。绘制深色 `#2a2a2a` 背景 + 双层网格：小格 40px（浅灰 `#3a3a3a`）+ 大格 80px（中灰 `#4a4a4a`）。启用 `setRenderHint(Antialiasing)`。 |
| **Boss 血条面板** | `EnemyDirector` 管理：当 Boss 登场时，在战斗场景顶部居中动态创建 `QFrame` + `QProgressBar` + `QLabel`，显示 Boss 名称和血条，每秒更新一次。 |
| **玩家角色图形** | `PlayerAvatarItem`：QPainter 绘制圆形身体（从职业配置色渐变）+ 三角瞄准指示器（朝向鼠标方向）+ 受击闪白（QTimer 衰减 α 通道）。 |

### 9.2 升级页面变更

| 维度 | V1 | V2 |
| ---- | -- | -- |
| 选项数量 | 3 | **5**（3 trait + 2 特殊子弹） |
| 选项卡片 | 简单文字 | 带图标 + 稀有度边框 + 属性变化预览 |
| 稀有度标识 | 无 | Common（灰）/ Rare（蓝）/ Epic（紫）三色边框 |
| 特殊子弹选项 | 无 | 12 种可选，显示 emoji PNG 预览图 |
| 特性已有标记 | 无 | 已拥有的特性标记"已拥有"状态 |

### 9.3 玩家信号

V2 新增 `dashCooldownChanged(float remainingSeconds, float totalSeconds)` 信号：
- 由 `BasicPlayer::updateDashState()` 在冲刺冷却变化时发射
- `GameMainPage` 连接该信号到 `DashCooldownWidget::updateCooldown()`
- 冲刺键：**空格键**，冷却时间 **3 秒**，硬直窗口 0.15 秒

---

## 十、经验与等级系统详细对比

### 10.1 经验阈值表

| 等级 | V1 累计经验 | V2 累计经验 | V2 各级增量 (Δ) |
| ---- | --------- | --------- | ------------- |
| 0 | 0 | 0 | -- |
| 1 | 20 | **0** | 0 |
| 2 | 45 | **10** | 10 |
| 3 | 75 | **50** | 40 |
| 4 | 110 | **140** | 90 |
| 5 | 150 | **300** | 160 |
| 6 | 195 | **550** | 250 |
| 7 | 245 | **910** | 360 |
| 8 | 300 | **1,400** | 490 |
| 9 | 360 | **2,040** | 640 |
| 10 | --（V1 最高 10 级） | **2,850** | 810 |
| 11 | -- | **3,850** | 1,000 |
| 12 | -- | **5,060** | 1,210 |
| 13 | -- | **6,500** | 1,440 |
| 14 | -- | **8,190** | 1,690 |
| 15 | -- | **10,150** | 1,960 |

- V1: 10 级，最大累计经验 **360**
- V2: **15 级**，最大累计经验 **10,150**（约 V1 的 **28 倍**）
- V2 公式: `threshold[n] = threshold[n-1] + 10 × (n-1)^2`（即 `10·n²` 累加）

### 10.2 波次经验倍率

V1 无此系统，V2 新增 `waveExpMultiplier(waveNumber)`：

| 波次范围 | 倍率 |
| ------- | ---- |
| 1-3 | 1.0x |
| 4-7 | 1.1x |
| 8-9 | 1.2x |
| 10+ | 1.5x |

### 10.3 单个敌人经验值

V1: 统一 10 点（`experiencePerEnemyDefeat=10`）
V2: 敌人独立经验值（Ogre=15, Jester=12, DemonLord=80, BoneLord=100, UFO=120, AlienPilot=60 等）

---

## 十一、关键数值变化总结

| 参数 | V1 | V2 | 变化 |
| ---- | -- | -- | ---- |
| **最大波次** | 10 | 10 | 不变 |
| **每波时长** | 30 秒 | 30 秒 | 不变 |
| **升级选项数** | 3 | **5** | +66% |
| **等待升级数** | -- | 每级 1 次（15 级 = 15 次升级） | 新增 |
| **最大同时敌人** | 10 | **15** | +50% |
| **初始敌人数量** | 3 | **5** | +66% |
| **敌人生成间隔** | 3,000ms | **2,200ms** | -27% |
| **PeaShooter 弹道数** | 1 | **2** | +100% |
| **最低敌人速度** | Bat(95) | BoneLord(40) | -58% |
| **最高敌人速度** | Bat(180) | Ogre(110) | -39%（但有 Charge 冲锋速度补偿） |
| **Boss 最大生命** | Skeleton(40) | UFO(500) | +1,150% |
| **最高单敌经验** | 10 | 120 (UFO) | +1,100% |
| **武器强化种类** | 0 | 5 | 全新系统 |
| **特殊子弹种类** | 0 | 12 | 全新系统 |
| **AI 行为种类** | 1（追逐） | 6 | +500% |
| **怪物系列** | 0 | 3 (Bizarre/Undead/Alien) | 全新系统 |
| **动态难度** | 无 | HP×1.05^wave, DMG×1.03^wave | 全新系统 |
| **Boss 波次** | 无 | 3 (波4/波7/波10) | 全新系统 |
| **冲刺系统** | 无 | 空格键，3 秒 CD | 全新系统 |
| **经验上限** | 360 (Lv10) | 10,150 (Lv15) | +2,719% |

---

## 十二、QRC 资源注册对比

V1 没有 `resources.qrc` 注册 emoji 图片（敌人使用纯色圆形 QPainter 绘制）。

V2 注册 **47 个 PNG 图片资源**：

| 类别 | 数量 | 示例 |
| ---- | ---- | ---- |
| UI 基础 | 4 | `emoji_dungeon.png`, `class_bg.png`, `warrior_icon.png`, `ranger_icon.png`, `caster_icon.png` |
| 敌人贴图 | 10 | `ogre_3d.png`, `ghost_3d.png`, `skull_3d.png`, `robot_3d.png`, `alien_3d.png`, `flying_saucer_3d.png` 等 |
| 升级图标 | 15 | `quick_hands.png`, `bouncing_bullet.png`, `frostbite.png`, `extra_projectiles.png`, `combo.png` 等 |
| 特殊子弹贴图 | 18 | `dagger_3d.png`, `sun_3d.png`, `grinning_cat_3d.png`（+4 张猫图备用）, `shrimp_3d.png`, `rocket_3d.png` 等 |

---

## 十三、数据模型抽象层级

### V1 继承体系

```
QObject
  └── game_factory.cpp 匿名命名空间中的具体类（无抽象接口分离）
```

### V2 继承体系

```
QObject
  ├── Player (抽象) → BasicPlayer (实现)
  ├── Weapon (抽象) → BasicWeapon (实现)
  ├── EnemyData (抽象) → BasicEnemyData (实现)
  ├── BulletData (抽象) → BasicBulletData (实现) → SpecialBulletData (扩展)
  └── Trait (抽象) → BasicTrait (实现)

QGraphicsObject → GameView (抽象)
  ├── EnemyView (抽象) → BasicEnemyView (实现)
  ├── BulletView (抽象) → BasicBulletView (实现)
  └── BulletView (抽象) → SpecialBulletView (实现) [并行继承，不继承 BasicBulletView]

QGraphicsItem → PlayerAvatarItem (直接实现)
QGraphicsView → BattleArenaView (实现)
QWidget
  ├── GameMainPage
  ├── DashCooldownWidget
  └── ...其他页面
```

---

*更新日志结束。更多架构细节请参阅 CODE_ANALYSIS.md。*