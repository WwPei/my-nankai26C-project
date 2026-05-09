# Emoji Dungeon 代码类型化分析

> 生成日期: 2026-05-09\
> 构建: CMake + Qt 6.10.2 MinGW / C++17\
> 总源文件: 57 个 (.h + .cpp)\
> 归纳类型: 7 种

---

## 目录

- [一、类型概览](#一类型概览)
- [二、类型一：配置层（Config / Core）](#二类型一配置层config--core)
- [三、类型二：数据模型抽象接口（Model Interfaces）](#三类型二数据模型抽象接口model-interfaces)
- [四、类型三：数据模型具体实现（Model Implementations）](#四类型三数据模型具体实现model-implementations)
- [五、类型四：视图层（View）](#五类型四视图层view)
- [六、类型五：页面层（Pages）](#六类型五页面层pages)
- [七、类型六：子系统层（Systems）](#七类型六子系统层systems)
- [八、类型七：特殊子弹系统（Bullets）](#八类型七特殊子弹系统bullets)
- [九、跨类型共同模式总结](#九跨类型共同模式总结)

---

## 一、类型概览

| 类型编号 | 类型名称 | 文件数量 | 目录位置 | 核心职责 |
| -------- | -------- | -------- | -------- | -------- |
| **类型一** | 配置层 (Config/Core) | 6 | `src/core/` | 枚举、结构体、配置表、工具函数 |
| **类型二** | 数据模型抽象接口 (Model Interfaces) | 6 | `src/models/interfaces/` | 声明实体契约（纯虚类） |
| **类型三** | 数据模型具体实现 (Model Implementations) | 10 | `src/models/basic/` | 填充接口逻辑（final 类） |
| **类型四** | 视图层 (View) | 10 | `src/views/` | 可视化渲染（MV 配对 + 独立组件） |
| **类型五** | 页面层 (Pages) | 8 | `src/pages/` | 用户界面与页面导航 |
| **类型六** | 子系统层 (Systems) | 10 | `src/systems/` | 游戏逻辑模块封装 |
| **类型七** | 特殊子弹系统 (Bullets) | 5 | `src/bullets/` | 12 种特殊子弹的配置/数据/视图 |

### 7 种类型的依赖关系

```
┌────────────────────────────────────────────────────────────┐
│                   类型五：页面层 (Pages)                     │
│   EmojiDungeonWindow  ── 管理 4 个 QWidget 页面             │
│       ├── ClassSelectPage                                   │
│       ├── GameMainPage ── 编排器（持有类型六的 3 个子系统）    │
│       └── UpgradePage                                       │
├────────────────────────────────────────────────────────────┤
│             类型六：子系统层 (Systems) ← 依赖类型三           │
│   UpgradeResolver / CombatCoordinator / EnemyDirector       │
│   GameFactory / WaveManager                                 │
├────────────────────────────────────────────────────────────┤
│      类型四：视图层 (View) ← 依赖类型二（数据接口）            │
│   BasicBulletView / BasicEnemyView                          │
│   BattleArenaView / DashCooldownWidget / PlayerAvatarItem   │
├────────────────────────────────────────────────────────────┤
│      类型三：数据模型实现 (Basic) ← 实现类型二                │
│   BasicPlayer / BasicWeapon / BasicEnemyData                │
│   BasicBulletData / BasicTrait                              │
├────────────────────────────────────────────────────────────┤
│  类型七：特殊子弹 (Bullets) ← 扩展类型三 + 类型四             │
│   SpecialBulletData → BasicBulletData                       │
│   SpecialBulletView → BulletView（并行继承）                  │
├────────────────────────────────────────────────────────────┤
│     类型二：数据模型接口 (Interfaces) ← 纯虚类，无实现        │
│   Player / Weapon / EnemyData / BulletData / Trait / GameView│
├────────────────────────────────────────────────────────────┤
│      类型一：配置层 (Core) ← 被所有其他类型依赖               │
│   game_enums / game_structs / game_tables / game_data       │
│   combat_utils                                              │
└────────────────────────────────────────────────────────────┘
```

---

## 二、类型一：配置层（Config / Core）

**文件列表：**

| 文件 | 行数 | 内容 |
| ---- | ---- | ---- |
| [game_enums.h](file:///F:/QT%20projects/emoji_vibe_coding/src/core/game_enums.h) | 121 | 13 个枚举定义 |
| [game_structs.h](file:///F:/QT%20projects/emoji_vibe_coding/src/core/game_structs.h) | 153 | 10 个结构体定义 |
| [game_tables.h](file:///F:/QT%20projects/emoji_vibe_coding/src/core/game_tables.h) | 596 | 所有实例配置表 |
| [game_data.h](file:///F:/QT%20projects/emoji_vibe_coding/src/core/game_data.h) | 4 | 聚合头文件 |
| [combat_utils.h](file:///F:/QT%20projects/emoji_vibe_coding/src/core/combat_utils.h) | 18 | 工具函数声明 |
| [combat_utils.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/core/combat_utils.cpp) | 71 | 工具函数实现 |

### 2.1 代码作用

为整个游戏提供**数据基础**——定义所有游戏对象的 ID 枚举、数据结构、静态配置表和通用工具函数。**其他所有类型（Model / View / System / Page）都直接依赖这一层**，它是整个项目的基石。

### 2.2 内容分析

#### game_enums.h —— 13 个枚举

```cpp
enum class PageId             { Start, ClassSelect, GameMain, Upgrade };         // 4 页面
enum class BattleFlowState    { Inactive, Battle, Upgrade };                     // 3 战斗状态
enum class PlayerClassId      { Warrior, Ranger, Caster };                       // 3 职业
enum class WeaponId           { PeaShooter, SpreadBlaster, ArcWand };            // 3 武器
enum class EnemyId            { Ogre, Jester, DemonLord, SkeletonNew, Ghost,     // 10 敌人
                                BoneLord, Robot, XenoBeast, UFO, AlienPilot };
enum class EnemyBehavior      { Chase, KeepDistance, Charge,                     // 6 AI 行为
                                ShootAndMove, SuicideBomb, Boss };
enum class TraitId            { QuickHands, ThickSkin, LuckyShot, VampiricAura,  // 10 特性
                                BouncingBullet, Frostbite, CriticalStrike,
                                ExperienceBoost, Vitality, Adrenaline };
enum class UpgradeOptionKind  { Trait, Weapon, Stat };                           // 3 升级类型
enum class UpgradeRarity      { Common, Rare, Epic };                            // 3 稀有度
enum class WeaponUpgradeId    { ExtraProjectiles, RangeBoost,                    // 5 武器强化
                                Pierce, BulletSize, Combo };
enum class DamageVisualType   { Neutral, Rapid, Arcane };                        // 3 伤害视觉
enum class BulletStyle        { Normal, Dagger, SunOrb, MoonOrb, Hacimi,        // 13 子弹
                                ThunderSpear, Boomerang, BloodArrow, StunBullet,
                                RandomGift, Comet, PushBullet, Rocket };
enum class SpecialEffect      { None, TrackAndAttach, Stun, KnockbackWithBonus };// 4 特效
```

#### game_structs.h —— 10 个关键结构体

| 结构体 | 关键字段 | 用途 |
| ------ | -------- | ---- |
| `WaveConfig` | `roundDurationMs`, `maxConcurrentEnemies`, `enemySpawnIntervalMs` | 波次参数 |
| `PlayerClassConfig` | `id`, `displayName`, `starterWeaponId`, `maxHealth`, `moveSpeed` | 职业属性 |
| `WeaponConfig` | `id`, `baseDamage`, `fireIntervalMs`, `projectileCount` | 武器属性 |
| `WeaponUpgradeConfig` | `extraProjectiles`, `rangeMultiplier`, `pierceCount`, `comboInterval` | 武器强化属性 |
| `BulletConfig` | `weaponId`, `damage`, `speed`, `collisionRadius` | 子弹属性 |
| `EnemyConfig` | `maxHealth`, `moveSpeed`, `behavior`, `imagePath`, 射击/冲锋/自爆参数 | 敌人全部属性含 AI |
| `TraitConfig` | `damageMultiplier` ~ `bounceDamageRetention`（14 个浮点字段） | 特性全部效果参数 |
| `UpgradeOption` | `optionId`, `kind`, `displayName`, `rarity`, `iconPath` | 升级选项 |
| `UpgradeAttributePreview` | `label`, `currentValue`, `nextValue`, `deltaText` | 属性预览条目 |
| `UpgradePreviewContext` | `classId`, `weaponId`, `ownedTraits` | 升级预览上下文 |
| `BulletSpecialTemplate` | `style`, `baseDamage`, `effect`, `defaultImagePath`, `altImagePaths` | 特殊子弹模板 |

#### game_tables.h —— 所有实例数据（约 600 行）

```
namespace GameConfig {
    inline const WaveConfig kWaveConfig {};
    inline const QList<int> kExperienceThresholds {0, 0, 10, 50, 140, ..., 10150};
    inline const QList<PlayerClassConfig> kPlayerClasses { 3 个职业 };
    inline const QList<WeaponConfig> kWeapons { 3 种武器 };
    inline const QList<WeaponUpgradeConfig> kWeaponUpgrades { 5 种强化 };
    inline const QList<BulletConfig> kBullets { 3 种子弹 };
    inline const QList<EnemyConfig> kEnemies { 10 种敌人 };
    inline const QList<TraitConfig> kTraits { 10 种特性 };
    inline const UpgradeOptions kUpgradeOptions { 15 个升级选项 };
    inline const UpgradeOptions kAllUpgradeOptions { 完整选项池 };

    [[nodiscard]] inline float waveExpMultiplier(int waveNumber);
    [[nodiscard]] inline int levelForExperience(int experience);
    [[nodiscard]] inline int experienceToNextLevel(int experience);
    [[nodiscard]] inline const EnemyConfig *findEnemyConfig(EnemyId id);
    // ... 每个配置表都有对应的 find*Config() 查询函数
}
```

### 2.3 共同模式

```
#pragma once + #include + 纯定义（无 .cpp，除 combat_utils）
```

#### 模式 A：纯头文件驱动

除了 `combat_utils.cpp`，所有配置都是 `.h` 文件中的 **`inline const` 变量**或**内联函数**：

```cpp
inline const QList<int> kExperienceThresholds { 0, 0, 10, 50, ... };
[[nodiscard]] inline float waveExpMultiplier(int waveNumber) {
    if (waveNumber >= 10) return 1.5F;
    if (waveNumber >= 8) return 1.2F;
    ...
}
```

- **零运行时开销**：`inline` 确保每个编译单元只有一份定义
- **所有数据在编译期确定**：无动态初始化依赖

#### 模式 B：命名空间聚合

所有配置集中在 `namespace GameConfig { }` 中：

```cpp
namespace GameConfig {
    inline const WaveConfig kWaveConfig {};
    inline const QList<int> kExperienceThresholds { ... };
    [[nodiscard]] inline int levelForExperience(int experience) { ... }
}
```

使用时通过 `GameConfig::kExperienceThresholds` 访问，命名清晰，避免全局命名空间污染。

#### 模式 C：三段递进定义

```
枚举 (enums) ───→ "是什么"
   ↓
结构体 (structs) ───→ "含什么字段"
   ↓
配置表 (tables) ───→ "具体值是多少"
```

例如敌人系统：

```
game_enums.h:  enum class EnemyId { Ogre, Jester, ... };
game_structs.h: struct EnemyConfig { EnemyId id; float maxHealth; ... };
game_tables.h:  { EnemyId::Ogre, "食人魔", ..., 80.0F, 110.0F, ... };
```

#### 模式 D：`[[nodiscard]]` 查询函数

每个配置表都有对应的查询函数：

```cpp
[[nodiscard]] inline const EnemyConfig *findEnemyConfig(EnemyId id) {
    for (const auto &c : kEnemies) {
        if (c.id == id) return &c;
    }
    return nullptr;
}
```

#### 模式 E：Q_DECLARE_METATYPE 注册

所有需要在信号/槽中传递的枚举和结构体都在文件末尾注册：

```cpp
Q_DECLARE_METATYPE(BattleFlowState)
Q_DECLARE_METATYPE(UpgradeOptionKind)
Q_DECLARE_METATYPE(DamageVisualType)
Q_DECLARE_METATYPE(UpgradeRarity)
Q_DECLARE_METATYPE(WeaponUpgradeId)
Q_DECLARE_METATYPE(EnemyBehavior)
Q_DECLARE_METATYPE(BulletStyle)
Q_DECLARE_METATYPE(SpecialEffect)
Q_DECLARE_METATYPE(UpgradeOption)
Q_DECLARE_METATYPE(UpgradeOptions)
Q_DECLARE_METATYPE(BulletSpecialTemplate)
```

#### 对比：类型一中唯一的 .cpp 文件

`combat_utils.cpp` 是配置层唯一的 `.cpp` 文件，因为它包含**非内联的运行时函数**：

```cpp
// combat_utils.cpp
QColor enemyBaseColor(EnemyId id) {
    switch (id) {
        case EnemyId::Ogre: return QColor("#8B0000"); // 暗红
        case EnemyId::Ghost: return QColor("#C0C0C0"); // 银白
        // ...
    }
}
```

---

## 三、类型二：数据模型抽象接口（Model Interfaces）

**文件列表：**

| 文件 | 行数 | 声明内容 |
| ---- | ---- | -------- |
| [player.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/interfaces/player.h) | 57 | 13 属性 + 8 Slot + 5 冲刺方法 + 5 信号 |
| [weapon.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/interfaces/weapon.h) | 47 | 4 属性 + 14 Slot + 5 查询 + 2 信号 |
| [enemy_data.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/interfaces/enemy_data.h) | 53 | 10 属性 + 8 AI 方法 + 7 信号 |
| [bullet_data.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/interfaces/bullet_data.h) | 46 | 10 属性 + 10 Slot + 2 信号 |
| [trait.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/interfaces/trait.h) | 24 | 3 属性 + 1 Slot |
| [game_view.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/interfaces/game_view.h) | 18 | 1 Slot + 1 信号 |

### 3.1 代码作用

定义游戏实体的**契约/规范**——声明每个数据对象"能做什么"、"有什么属性"、"发出什么信号"，但不提供任何实现。这是 **MV (Model-View) 分离架构**中 Model 侧的**抽象层**。

### 3.2 内容分析

#### player.h —— 玩家抽象接口

```cpp
class Player : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    ~Player() override = default;

    // ① 查询属性（13 个 getter）
    [[nodiscard]] virtual PlayerClassId classId() const = 0;
    [[nodiscard]] virtual QPointF worldPosition() const = 0;
    [[nodiscard]] virtual float currentHealth() const = 0;
    [[nodiscard]] virtual float maxHealth() const = 0;
    [[nodiscard]] virtual float moveSpeed() const = 0;
    [[nodiscard]] virtual WeaponId weaponId() const = 0;
    [[nodiscard]] virtual float damageMultiplier() const = 0;
    [[nodiscard]] virtual float defenseMultiplier() const = 0;
    [[nodiscard]] virtual float speedMultiplier() const = 0;
    [[nodiscard]] virtual float expMultiplier() const = 0;
    [[nodiscard]] virtual QMap<TraitId, int> traitLevels() const = 0;

    // ② 操作方法（8 个 public slot）
public slots:
    virtual void setWorldPosition(const QPointF &position) = 0;
    virtual void setMoveDirection(const QPointF &direction) = 0;
    virtual void equipWeapon(WeaponId weaponId) = 0;
    virtual void applyTrait(TraitId traitId) = 0;
    virtual void receiveDamage(float amount) = 0;
    virtual void applyWeaponUpgrade(WeaponUpgradeId upgradeId) = 0;
    virtual void heal(float amount) = 0;
    virtual void applySpeedBuff(float multiplier, float durationSeconds) = 0;

    // ③ 特殊系统（冲刺）
    virtual void dash() = 0;
    [[nodiscard]] virtual bool isDashing() const = 0;
    [[nodiscard]] virtual float dashCooldownRemaining() const = 0;
    [[nodiscard]] virtual float dashCooldownTotal() const = 0;
    [[nodiscard]] virtual QPointF dashDirection() const = 0;
    virtual void updateDash(float deltaSeconds) = 0;

    // ④ 信号
signals:
    void moved(const QPointF &position);
    void healthChanged(float currentHealth, float maxHealth);
    void weaponChanged(WeaponId weaponId);
    void defeated();
    void dashCooldownChanged(float remainingSeconds, float totalSeconds);
};
```

#### 其他接口文件一览

**weapon.h** 声明：
- 属性：`id()`, `config()`, `owner()`, `isFiring()`
- Slot：`bindOwner()`, `setAimDirection()`, `startFiring()`, `stopFiring()`, `advanceCooldown()`, `applyTrait()`, `addExtraProjectiles()`, `applyRangeMultiplier()`, `applyPierce()`, `applyBulletSizeScale()`, `enableCombo()`
- 查询：`bulletSizeScale()`, `pierceCount()`, `extraProjectiles()`, `rangeMultiplier()`, `comboInterval()`, `comboDamageMultiplier()`, `fireRateScale()`, `comboCounter()`
- 信号：`fireRequested(WeaponId, origin, direction)`, `cooldownChanged(float)`

**enemy_data.h** 声明：
- 属性：`id()`, `config()`, `worldPosition()`, `currentHealth()`, `maxHealth()`, `moveSpeed()`, `contactDamage()`, `collisionRadius()`, `isDefeated()`
- Slot：`setWorldPosition()`, `receiveDamage()`, `setTargetPosition()`, `advanceFrame()`, `applySlow()`
- AI：`currentBehavior()`, `updateAI()`, `enterBossPhase()`, `currentBossPhase()`, `chargeCooldownRemaining()`, `chargeProgress()`, `shootCooldownRemaining()`, `imagePath()`
- 信号：`positionChanged()`, `healthChanged()`, `damageReceived()`, `defeated()`, `requestShoot()`, `requestSuicideExplosion()`, `requestSpawnMinion()`

**bullet_data.h** 声明：
- 属性：`weaponId()`, `config()`, `worldPosition()`, `direction()`, `damage()`, `speed()`, `collisionRadius()`, `isCritical()`, `isExpired()`, `remainingPierceCount()`, `sizeScale()`, `comboMultiplier()`, `isEnemyBullet()`
- Slot：`setWorldPosition()`, `setDirection()`, `advanceFrame()`, `expire()`, `setCritical()`, `setPierceCount()`, `setSizeScale()`, `setComboMultiplier()`, `setDamageMultiplier()`, `setEnemyBullet()`, `setSpeed()`
- 信号：`positionChanged()`, `expired()`

**trait.h** 声明：
- 属性：`id()`, `displayName()`, `description()`
- Slot：`applyToPlayer(Player *player)`

**game_view.h** 声明：
- Slot：`syncFromData()`
- 信号：`removalRequested()`

### 3.3 共同模式

```
class Xxx : public QObject {      // ① 都继承 QObject
    Q_OBJECT
public:
    using QObject::QObject;        // ② 复用父类构造器
    ~Xxx() override = default;     // ③ virtual 析构

    // ④ 查询属性：纯虚 [[nodiscard]] getter
    [[nodiscard]] virtual Type property() const = 0;

    // ⑤ 操作方法：public slot 纯虚
public slots:
    virtual void doSomething(...) = 0;

    // ⑥ 信号：Qt 信号声明
signals:
    void somethingChanged(...);
};
```

#### 6 个文件高度一致的骨架

每个接口文件都精确遵循以下结构：

```
第 1 行:  #pragma once
第 2~N:  #include ...
第 N+1 行: class Xxx : public QObject {
第 N+2 行:     Q_OBJECT
第 N+3 行: public:
第 N+4 行:     using QObject::QObject;
第 N+5 行:     ~Xxx() override = default;
           // 查询 getter...
           // 操作 slot...
           // 信号...
第 N+M 行: };
```

#### 三区段排列

每个接口文件的方法排列顺序完全一致：

```
public:                          → 区段一：查询属性（const getter）
    [[nodiscard]] virtual Type getter() const = 0;

public slots:                    → 区段二：操作方法（setter / action）
    virtual void setter(...) = 0;

signals:                         → 区段三：事件通知
    void signalName(...);
```

#### 纯接口设计

- 所有方法都是 `= 0` 纯虚函数
- **没有**非静态成员变量
- **没有**构造函数体（使用 `using QObject::QObject` 透传）

#### 沉浸式信号声明

敌人和子弹使用信号声明**它需要的交互**，而非直接调用：

```cpp
// enemy_data.h 中声明的信号 —— 它"需要"别人来处理这些请求
signals:
    void requestShoot(QPointF direction, int bulletCount, float bulletSpeed, float bulletDamage);
    void requestSuicideExplosion(float radius, float damage);
    void requestSpawnMinion();
```

子系统通过 `connect()` 监听这些信号，实现完全的解耦：

```cpp
// enemy_director.cpp 中 —— 连接敌人的射击信号
connect(enemyData, &EnemyData::requestShoot,
        this, &EnemyDirector::onEnemyShootRequested);
```

#### game_view.h 的特殊性

`GameView` 虽然命名是 View，但作为**视图的抽象基类**放在 `models/interfaces/` 目录：

```cpp
class GameView : public QGraphicsObject {  // ← 继承 QGraphicsObject，不是 QObject
    Q_OBJECT
public:
    using QGraphicsObject::QGraphicsObject;
    ~GameView() override = default;

public slots:
    virtual void syncFromData() = 0;   // ← 所有视图的核心方法（每帧同步）

signals:
    void removalRequested();           // ← 请求从场景移除自己
};
```

所有视图接口（EnemyView / BulletView）都继承自 GameView。

---

## 四、类型三：数据模型具体实现（Model Implementations）

**文件列表：**

| 文件 | 行数 | 实现类 | 实现接口 |
| ---- | ---- | ------ | -------- |
| [basic_player.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_player.h) / [.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_player.cpp) | 60 + 214 | `BasicPlayer final` | `Player` |
| [basic_weapon.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_weapon.h) / [.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_weapon.cpp) | 54 + 175 | `BasicWeapon final` | `Weapon` |
| [basic_enemy_data.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_enemy_data.h) / [.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_enemy_data.cpp) | 77 + 339 | `BasicEnemyData final` | `EnemyData` |
| [basic_bullet_data.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_bullet_data.h) / [.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_bullet_data.cpp) | 52 + 79 | `BasicBulletData` | `BulletData` |
| [basic_trait.h](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_trait.h) / [.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/models/basic/basic_trait.cpp) | 16 + 30 | `BasicTrait final` | `Trait` |

### 4.1 代码作用

**Interface 的实现者** —— 将抽象基类的所有纯虚方法填充为真实逻辑。每个实现类都**持有配置指针**，从配置表中读取初始值，运行时状态通过成员变量追踪。

### 4.2 内容分析

#### BasicPlayer —— 玩家具体实现

**核心私有成员：**

```cpp
class BasicPlayer final : public Player {
    // ...
private:
    const PlayerClassConfig *m_config{nullptr};   // 配置指针（只读）
    QPointF m_worldPosition;                       // 当前位置
    QPointF m_moveDirection;                       // 移动方向
    float m_currentHealth{0.0F};                   // 当前生命
    float m_damageMultiplier{1.0F};                // 伤害倍率
    float m_defenseMultiplier{1.0F};               // 防御倍率
    float m_speedMultiplier{1.0F};                 // 速度倍率
    WeaponId m_weaponId{WeaponId::PeaShooter};     // 当前武器
    float m_expMultiplier{1.0F};                   // 经验倍率
    float m_extraMaxHealth{0.0F};                  // 额外生命
    QMap<TraitId, int> m_traitLevels;               // 特性等级追踪
    bool m_isDashing{false};                        // 冲刺状态
    float m_dashDurationRemaining{0.0F};            // 冲刺剩余时长
    float m_dashCooldownRemaining{0.0F};            // 冲刺冷却
    float m_dashHardStunRemaining{0.0F};            // 冲刺硬直
    QPointF m_lastMoveDirection{0.0F, -1.0F};      // 最后移动方向
};
```

**核心逻辑：**
- WASD 移动 + 边界约束
- 空格冲刺：3 秒 CD，0.15 秒硬直窗口
- 10 种特性叠加：`applyTrait()` 中 switch 分支处理
- 属性管理：`
- 经验倍率/速度倍率/防御倍率乘法叠加

#### BasicWeapon —— 武器具体实现

**核心私有成员：**

```cpp
class BasicWeapon final : public Weapon {
    // ...
private:
    const WeaponConfig *m_config{nullptr};
    Player *m_owner{nullptr};
    QPointF m_aimDirection{0.0, -1.0};
    float m_fireCooldownRemainingMs{0.0F};
    bool m_isFiring{false};
    float m_fireRateScale{1.0F};
    int m_extraProjectiles{0};
    float m_rangeMultiplier{1.0F};
    int m_pierceCount{0};
    float m_bulletSizeScale{1.0F};
    int m_comboInterval{0};
    float m_comboDamageMultiplier{1.0F};
    int m_comboCounter{0};
};
```

**核心逻辑：**
- `advanceCooldown()`: 每帧减少冷却，归零时发射 `fireRequested` 信号
- 冷却 = `fireIntervalMs / fireRateScale`
- 弹道数 = `projectileCount + extraProjectiles`
- 连击计数：每第 N 发附带额外伤害倍率
- `startFiring()` / `stopFiring()`: 持续/停止射击

#### BasicEnemyData —— 敌人具体实现

**核心私有成员：**

```cpp
class BasicEnemyData final : public EnemyData {
    // ...
private:
    const EnemyConfig *m_config{nullptr};
    QPointF m_worldPosition;
    float m_currentHealth;
    float m_slowFactor{1.0F};
    float m_slowRemainingSeconds{0.0F};
    float m_chargeUpTimer{0.0F};
    float m_chargeDurationTimer{0.0F};
    float m_chargeCooldownTimer{0.0F};
    bool m_isCharging{false};
    float m_shootCooldownTimer{0.0F};
    int m_currentBossPhase{0};
};
```

**核心逻辑 —— 5 种 AI 行为状态机：**

```cpp
void BasicEnemyData::updateAI(float deltaSeconds, QPointF playerPosition) {
    switch (m_config->behavior) {
        case EnemyBehavior::Chase:
            // 向玩家移动
            setTargetPosition(playerPosition);
            break;
        case EnemyBehavior::Charge:
            // 蓄力 → 高速冲锋 → 冷却
            if (m_isCharging) { /* 冲锋逻辑 */ }
            else if (chargeCooldownTimer <= 0) { /* 开始蓄力 */ }
            break;
        case EnemyBehavior::ShootAndMove:
            // 边射击边移动
            shootCooldownTimer -= deltaSeconds;
            if (shootCooldownTimer <= 0) emit requestShoot(...);
            setTargetPosition(playerPosition);
            break;
        case EnemyBehavior::Boss:
            // 多阶段 + 弹幕 + 召唤
            if (currentHealth / maxHealth < 0.5F) enterBossPhase(2);
            break;
        // ...
    }
}
```

#### BasicBulletData —— 子弹具体实现

**核心逻辑：**
- `advanceFrame()`: 按方向 + 速度线性飞行
- 距离追踪：`m_traveledDistance += speed * deltaSeconds`
- 过期判定：`traveledDistance > config->range * rangeMultiplier`
- 穿透抵消：`remainingPierceCount > 0` 时减 1 继续飞行

#### BasicTrait —— 特性具体实现

**极简实现**（仅 16+30 行）：

```cpp
class BasicTrait final : public Trait {
    // ...
    const TraitConfig *m_config;
};

void BasicTrait::applyToPlayer(Player *player) {
    if (m_config) {
        player->applyTrait(m_config->id);   // 委托给 Player
    }
}
```

### 4.3 共同模式

```
class BasicXxx final : public Xxx {     // ① final 封闭继承链
    Q_OBJECT
public:
    explicit BasicXxx(const XxxConfig *config, ...);  // ② 构造注入配置指针

    // ③ 全部 override 实现
    [[nodiscard]] Type property() const override { return m_xxx; }
public slots:
    void setXxx(...) override { m_xxx = ...; emit xxxChanged(); }

private:
    const XxxConfig *m_config{nullptr};   // ④ 只读配置指针
    Type m_xxx;                            // ⑤ 运行时状态成员
};
```

#### 模式 A：`final` 封闭继承链

所有实现类都标记为 `final`，不允许进一步派生：

```cpp
class BasicPlayer final : public Player { ... };
class BasicWeapon final : public Weapon { ... };
class BasicEnemyData final : public EnemyData { ... };
class BasicTrait final : public Trait { ... };
```

**唯一例外**：`BasicBulletData` 不是 `final`——允许 `SpecialBulletData` 继承它。

#### 模式 B：构造注入配置指针

每个 BasicXxx 的构造函数都接收 `const XxxConfig *config`：

```cpp
BasicPlayer::BasicPlayer(const PlayerClassConfig *config, QObject *parent)
    : Player(parent), m_config(config) {
    m_currentHealth = config->maxHealth;       // 从配置读取初始值
    m_weaponId = config->starterWeaponId;
}

BasicEnemyData::BasicEnemyData(const EnemyConfig *config, const QPointF &pos, QObject *parent)
    : EnemyData(parent), m_config(config), m_worldPosition(pos) {
    m_currentHealth = config->maxHealth;
}
```

#### 模式 C：Setter 发射信号

每个 setter 在修改值的同时发射通知信号：

```cpp
void BasicPlayer::setWorldPosition(const QPointF &position) {
    m_worldPosition = position;
    emit moved(position);       // ← 通知视图更新
}

void BasicEnemyData::setWorldPosition(const QPointF &position) {
    m_worldPosition = position;
    emit positionChanged(position);
}
```

#### 模式 D：成员变量分两层

```cpp
private:
    const XxxConfig *m_config{nullptr};   // 第 1 层：只读配置（永不修改）
    Type m_field;                          // 第 2 层：运行时可变状态
```

例如 BasicPlayer 有 1 个配置指针 + 14 个运行时成员。

#### 代码量比例

接口 vs 实现的行数比：

| 接口 | 接口行数 | 实现行数 (.h+.cpp) | 比例 |
| ---- | -------- | ------------------ | ---- |
| Player | 57 | 60 + 214 = 274 | 1 : 4.8 |
| Weapon | 47 | 54 + 175 = 229 | 1 : 4.9 |
| EnemyData | 53 | 77 + 339 = 416 | 1 : 7.8 |
| BulletData | 46 | 52 + 79 = 131 | 1 : 2.8 |
| Trait | 24 | 16 + 30 = 46 | 1 : 1.9 |

**平均值：接口 : 实现 ≈ 1 : 4.5**

---

## 五、类型四：视图层（View）

**文件列表：**

| 文件 | 行数 | 类名 | 继承自 | 渲染方式 |
| ---- | ---- | ---- | ------ | -------- |
| [interfaces/bullet.h](file:///F:/QT%20projects/emoji_vibe_coding/src/views/interfaces/bullet.h) | 18 | `BulletView` | `GameView` | —（抽象接口） |
| [interfaces/enemy.h](file:///F:/QT%20projects/emoji_vibe_coding/src/views/interfaces/enemy.h) | 18 | `EnemyView` | `GameView` | —（抽象接口） |
| [basic_bullet_view.h](file:///F:/QT%20projects/emoji_vibe_coding/src/views/basic_bullet_view.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/views/basic_bullet_view.cpp) | 29 + 98 | `BasicBulletView` | `BulletView` | QPainter 彩色圆 + 拖尾 + 三角箭头 |
| [basic_enemy_view.h](file:///F:/QT%20projects/emoji_vibe_coding/src/views/basic_enemy_view.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/views/basic_enemy_view.cpp) | 37 + 168 | `BasicEnemyView` | `EnemyView` | QPainter PNG 贴图 + 血量条 + 闪白 |
| [battle_arena_view.h](file:///F:/QT%20projects/emoji_vibe_coding/src/views/battle_arena_view.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/views/battle_arena_view.cpp) | 22 + 61 | `BattleArenaView` | `QGraphicsView` | drawBackground 网格线 |
| [dash_cooldown_widget.h](file:///F:/QT%20projects/emoji_vibe_coding/src/views/dash_cooldown_widget.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/views/dash_cooldown_widget.cpp) | 25 + 108 | `DashCooldownWidget` | `QWidget` | QPainter 环形扇形进度 |
| [player_avatar_item.h](file:///F:/QT%20projects/emoji_vibe_coding/src/views/player_avatar_item.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/views/player_avatar_item.cpp) | 24 + 85 | `PlayerAvatarItem` | `QGraphicsItem` | QPainter 渐变圆形 + 三角瞄准器 |

### 5.1 代码作用

负责游戏中所有**可视化渲染**。分为两个子类型：

**子类型 A：MV 配对视图** —— 绑定数据模型，每帧同步

- `BulletView` (接口) → `BasicBulletView` (实现)
- `EnemyView` (接口) → `BasicEnemyView` (实现)

**子类型 B：独立 UI 组件** —— 不绑定数据模型，直接通过 setter 接收外部输入

- `BattleArenaView`（网格背景场景）
- `DashCooldownWidget`（冲刺冷却指示器）
- `PlayerAvatarItem`（玩家角色图形）

### 5.2 内容分析

#### MV 配对视图接口

```cpp
// bullet.h —— 子弹视图抽象接口
class BulletView : public GameView {
    Q_OBJECT
public:
    using GameView::GameView;
    ~BulletView() override = default;

    [[nodiscard]] virtual BulletData *model() const = 0;  // 返回模型指针

public slots:
    virtual void bindModel(BulletData *data) = 0;          // 绑定模型
};

// enemy.h —— 敌人视图抽象接口
class EnemyView : public GameView {
    Q_OBJECT
public:
    using GameView::GameView;
    ~EnemyView() override = default;

    [[nodiscard]] virtual EnemyData *model() const = 0;

public slots:
    virtual void bindModel(EnemyData *data) = 0;
};
```

#### BasicBulletView —— 基础子弹渲染

```cpp
class BasicBulletView final : public BulletView {
    Q_OBJECT
public:
    explicit BasicBulletView(QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;       // 边界矩形
    [[nodiscard]] QPainterPath shape() const override;        // 碰撞形状
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;                      // 渲染

    [[nodiscard]] BulletData *model() const override;          // 返回模型
    void bindModel(BulletData *data) override;                 // 绑定模型
    void syncFromData() override;                              // 每帧同步

private:
    QPointer<BulletData> m_model;                               // QPointer 安全指针
    qreal m_radius {6.0};                                      // 子弹半径
};
```

#### BasicEnemyView —— 敌人视图

```cpp
class BasicEnemyView final : public EnemyView {
    Q_OBJECT
public:
    explicit BasicEnemyView(QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    [[nodiscard]] QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    [[nodiscard]] EnemyData *model() const override;
    void bindModel(EnemyData *data) override;
    void syncFromData() override;

private:
    QPointer<EnemyData> m_model;
    QTimer *m_hitFlashTimer{nullptr};                           // 受击闪白定时器
    QPixmap m_pixmap;                                           // PNG 贴图
    bool m_hasImage{false};
    qreal m_radius{18.0};
    qreal m_healthRatio{1.0};
    qreal m_hitFlashIntensity{0.0};
    QColor m_hitFlashColor;
};
```

#### PlayerAvatarItem —— 玩家角色图形

```cpp
class PlayerAvatarItem final : public QGraphicsItem {
public:
    explicit PlayerAvatarItem(QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void setAimDirection(const QPointF &direction);              // 设置瞄准方向
    void setHitFlash(qreal intensity);                           // 设置受击闪白强度

private:
    QPointF m_aimDirection {1.0, 0.0};                          // 瞄准方向
    qreal m_hitFlashIntensity {0.0};                             // 闪白强度
};
```

#### DashCooldownWidget —— 冲刺冷却指示器

```cpp
class DashCooldownWidget final : public QWidget {
    Q_OBJECT
public:
    explicit DashCooldownWidget(QWidget *parent = nullptr);
    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

public slots:
    void updateCooldown(float remainingSeconds, float totalSeconds);  // 外部 setter

protected:
    void paintEvent(QPaintEvent *event) override;                     // 自定义绘制

private:
    float m_remainingSeconds{0.0F};
    float m_totalSeconds{3.0F};
    bool m_isReady{true};
    QTimer *m_flashTimer{nullptr};
    int m_flashCount{0};
    bool m_flashVisible{true};
};
```

#### BattleArenaView —— 战斗场景视图

```cpp
class BattleArenaView final : public QGraphicsView {
public:
    explicit BattleArenaView(QGraphicsScene *scene, QWidget *parent = nullptr);
    void setGridVisible(bool visible);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    bool m_gridVisible {true};
};
```

### 5.3 共同模式

#### MV 配对视图的 3 个核心方法

```
① model()        → 返回数据模型指针（"我是谁的数据视图？"）
② bindModel()    → 建立视图↔数据的连接（"我的数据是谁？"）
③ syncFromData() → 每帧从数据模型同步状态（"更新我的状态"）
```

完整同步流程：

```cpp
// game_main_page.cpp 的主循环中
for (auto &enemy : m_enemies) {
    enemy.data->advanceFrame(dt);           // ① 数据更新位置/状态
    enemy.data->updateAI(dt, playerPos);    // ② AI 逻辑
    enemy.view->syncFromData();             // ③ 视图从数据同步（每帧调用）
}
```

#### QPointer 安全指针

所有视图类使用 `QPointer` 而非裸指针持有模型引用：

```cpp
QPointer<BulletData> m_model;    // 如果 BulletData 被 delete，
                                  // m_model 自动变为 nullptr
```

这防止了模型对象先于视图销毁时的悬空指针崩溃。

#### QGraphicsItem 三件套

每个继承 QGraphicsItem 的视图都重写这 3 个方法：

| 方法 | 作用 | BasicBulletView | BasicEnemyView |
| ---- | ---- | --------------- | -------------- |
| `boundingRect()` | 定义绘制边界（Qt 用此做裁剪优化） | 以子弹位置为中心的方形 | 以敌人位置为中心的方形 |
| `shape()` | 定义碰撞/选中的精确形状 | 圆形 QPainterPath | 圆形 QPainterPath |
| `paint()` | 定义渲染内容 | 彩色圆 + 拖尾 + 三角箭头 | PNG 贴图 + 血量条 + 闪白 |

#### 独立 UI 组件的模式

没有 `model()` 和 `bindModel()`：

```cpp
class XxxWidget final : public QWidget / QGraphicsItem {
public:
    // 通过 setter 接收外部输入
    void setXxx(...);

protected:
    void paintEvent / paint(...);   // 自定义绘制（纯 QPainter，无数据绑定）
};
```

#### 渲染多样性与统一入口

尽管渲染内容差异巨大，但都通过统一的 `paint()` 入口：

| 组件 | 绘制内容 | QPainter 操作 |
| ---- | -------- | ------------- |
| BasicBulletView | 彩色圆形 + 拖尾残影 + 前向三角 | `setBrush()` + `drawEllipse()` + `drawPolygon()` |
| BasicEnemyView | PNG 贴图 + 绿/黄/红血条 | `drawPixmap()` + `drawRect()` |
| PlayerAvatarItem | 渐变色圆形 + 三角瞄准器 | `setBrush(gradient)` + `drawEllipse()` + `drawPolygon()` |
| DashCooldownWidget | 灰色底环 + 蓝色进度扇区 + ⚡图标 + 秒数文字 | `drawArc()` + `drawPixmap()` + `drawText()` |
| BattleArenaView | 深色背景 + 双层网格线 | `fillRect()` + `drawLine()` |

---

## 六、类型五：页面层（Pages）

**文件列表：**

| 文件 | 行数 | 类名 | 继承自 | 负责内容 |
| ---- | ---- | ---- | ------ | -------- |
| [emoji_dungeon_window.h](file:///F:/QT%20projects/emoji_vibe_coding/src/pages/emoji_dungeon_window.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/pages/emoji_dungeon_window.cpp) | 41 | `EmojiDungeonWindow` | `QMainWindow` | 页面导航中枢、信号串联 |
| [class_select_page.h](file:///F:/QT%20projects/emoji_vibe_coding/src/pages/class_select_page.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/pages/class_select_page.cpp) | 50 | `ClassSelectPage` | `QWidget` | 3 职业展示与选择 |
| [game_main_page.h](file:///F:/QT%20projects/emoji_vibe_coding/src/pages/game_main_page.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/pages/game_main_page.cpp) | 134 | `GameMainPage` | `QWidget` | 核心战斗编排器 |
| [upgrade_page.h](file:///F:/QT%20projects/emoji_vibe_coding/src/pages/upgrade_page.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/pages/upgrade_page.cpp) | 84 | `UpgradePage` | `QWidget` | 5 选 1 升级与预览 |

### 6.1 代码作用

**用户可见的界面** —— 4 个页面覆盖完整的游戏流程：

```
EmojiDungeonWindow (QMainWindow)
    └── QStackedWidget
        ├── StartPage (开始页: "开始游戏"按钮)
        ├── ClassSelectPage (职业选择: 战士/游侠/施法者)
        ├── GameMainPage (战斗主页面: 核心游戏循环) ← 最核心
        └── UpgradePage (升级页面: 5 选 1 升级)
```

### 6.2 内容分析

#### EmojiDungeonWindow —— 页面导航中枢

```cpp
class EmojiDungeonWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit EmojiDungeonWindow(QWidget *parent = nullptr);

private:
    void buildPages();          // 创建所有子页面
    void connectNavigation();   // 连接所有页面间信号（核心）
    void setCurrentPage(PageId pageId);
    void updateWindowTitle();

    QStackedWidget *m_stack {nullptr};
    QWidget *m_startPage {nullptr};
    ClassSelectPage *m_classSelectPage {nullptr};
    GameMainPage *m_gameMainPage {nullptr};
    UpgradePage *m_upgradePage {nullptr};
    GameFactory *m_factory {nullptr};              // 共享工厂
    QList<TraitId> m_selectedTraits;               // 已选特性追踪
    PlayerClassId m_currentClassId {PlayerClassId::Warrior};
    BulletStyle m_activeBulletStyle{BulletStyle::Normal};
    PageId m_currentPage {PageId::Start};
};
```

**关键方法** `connectNavigation()` —— 连接页面间的所有信号：

```cpp
void EmojiDungeonWindow::connectNavigation() {
    // 开始页 → 选择页
    connect(m_startPage, &QPushButton::clicked, this, [this]{ setCurrentPage(PageId::ClassSelect); });

    // 选择页 → 主页面
    connect(m_classSelectPage, &ClassSelectPage::classSelected, this, [this](PlayerClassId id) {
        m_currentClassId = id;
        m_gameMainPage->setSelectedClass(id);
        setCurrentPage(PageId::GameMain);
    });

    // 主页面 → 升级页（升级请求）
    connect(m_gameMainPage, &GameMainPage::upgradeRequested, this, [this] {
        UpgradeOptions options = m_gameMainPage->waveManager()->currentUpgradeOptions();
        UpgradePreviewContext ctx;
        ctx.classId = m_currentClassId;
        ctx.ownedTraits = m_selectedTraits;
        m_upgradePage->loadOptions(options, ctx);
        setCurrentPage(PageId::Upgrade);
    });

    // 升级页 → 主页面（确认选择）
    connect(m_upgradePage, &UpgradePage::traitSelected, this, [this](TraitId id) {
        m_selectedTraits.push_back(id);
        m_gameMainPage->applyTrait(id);
        m_gameMainPage->resumeBattleState();
        setCurrentPage(PageId::GameMain);
    });
    // ... weaponUpgradeSelected, bulletStyleSelected 类似
}
```

#### GameMainPage —— 核心编排器

**子模块引用：**

```cpp
class GameMainPage : public QWidget {
    // ...
private:
    UpgradeResolver *m_upgradeResolver{nullptr};     // 升级逻辑模块
    CombatCoordinator *m_combatCoordinator{nullptr}; // 战斗交互模块
    EnemyDirector *m_enemyDirector{nullptr};          // 敌人生成模块

    DashCooldownWidget *m_dashCooldownWidget{nullptr};
    BattleArenaView *m_view {nullptr};
    PlayerAvatarItem *m_playerAvatar {nullptr};

    QVector<GameFactory::EnemyEntity> m_enemies;     // 敌人容器
    QVector<GameFactory::BulletEntity> m_bullets;     // 子弹容器
    // ...
};
```

**核心循环 `handleBattleTick()`：**

```
handleBattleTick()                          每 16ms 调一次
  │
  ├── EnemyDirector::spawnBossIfPending()    检查是否需要生成 Boss
  ├── EnemyDirector::spawnTestEnemy()        随机生杂兵
  ├── updatePlayerMovement()                 WASD 移动
  ├── updateWeaponAim()                      鼠标瞄准
  ├── Weapon::advanceCooldown()              武器冷却 → fireRequested
  │     → CombatCoordinator::handleWeaponFireRequested()  创建子弹
  ├── foreach bullet: advanceFrame()         子弹飞行
  ├── foreach enemy: advanceFrame()+updateAI() 敌人 AI
  ├── CombatCoordinator::resolveCombatCollisions() 碰撞检测
  ├── CombatCoordinator::cleanupExpiredBullets()   清理过期子弹
  ├── CombatCoordinator::cleanupDefeatedEnemies()  清理击败敌人
  ├── WaveManager::advanceFrame()            波次推进 + 经验计算
  └── updateStatusText()                     刷新 12 个状态 Label
```

#### ClassSelectPage —— 职业选择页

```cpp
class ClassSelectPage : public QWidget {
    Q_OBJECT
public:
    explicit ClassSelectPage(QWidget *parent = nullptr);

signals:
    void classSelected(PlayerClassId classId);
    void backRequested();

private:
    void setupUi();
    void setupStyle();
    void setupConnections();
    void selectCard(PlayerClassId id);
    void animateCardSelection(QFrame *card);
    void refreshCardStyle(QFrame *card, bool selected);

    QList<QFrame *> m_cards;       // 3 张职业卡片
    PlayerClassId m_selectedId;
    QPushButton *m_confirmButton{nullptr};
    QPushButton *m_backButton{nullptr};
    QLabel *m_background{nullptr};
    QLabel *m_titleLabel{nullptr};
};
```

#### UpgradePage —— 升级页面

```cpp
class UpgradePage : public QWidget {
    Q_OBJECT
public:
    explicit UpgradePage(QWidget *parent = nullptr);
    void loadOptions(const UpgradeOptions &options, const UpgradePreviewContext &context);

signals:
    void traitSelected(TraitId traitId);
    void weaponUpgradeSelected(WeaponUpgradeId weaponUpgradeId);
    void bulletStyleSelected(BulletStyle style);
    void confirmed();

private:
    void buildOptionCards();       // 创建 5 张可选卡片
    void refreshCardStyles();      // 刷新卡片边框样式
    void selectCard(int index);    // 选中某张卡片
    void rebuildPreviewPanel();    // 更新右侧预览面板
    [[nodiscard]] UpgradeAttributePreviews buildAttributePreviews(const UpgradeOption &option) const;

    UpgradeOptions m_upgradeOptions;
    QList<QFrame *> m_cards;        // 5 张选项卡片
    QFrame *m_cardsContainer{nullptr};
    int m_selectedOptionIndex{-1};
    QPushButton *m_confirmButton{nullptr};
    // ...
};
```

### 6.3 共同模式

```
class XxxPage : public QWidget {     // ① QWidget 子类
    Q_OBJECT
public:
    explicit XxxPage(QWidget *parent = nullptr);

signals:
    void xxxRequested();             // ② 页面输出信号

protected:
    bool eventFilter(QObject *, QEvent *) override; // ③ 事件过滤器

private:
    void setupUi();                  // ④ 三个 setup 方法
    void setupStyle();
    void setupConnections();

    QWidget *m_widget{nullptr};      // ⑤ {} 零值初始化
};
```

#### 模式 A：星型信号拓扑

页面间通过 EmojiDungeonWindow 作为信号中枢，形成星型通信：

```
ClassSelectPage
    │ classSelected(id)
    ▼
EmojiDungeonWindow (connectNavigation 信号中枢)
    │
    ├──▶ GameMainPage::setSelectedClass()
    │         │
    │         │ upgradeRequested()
    │         ▼
    │    EmojiDungeonWindow
    │         │
    │         ├──▶ UpgradePage::loadOptions()
    │         │
    │         │ traitSelected / bulletStyleSelected / weaponUpgradeSelected
    │         ▼
    │    EmojiDungeonWindow
    │         │
    │         ├──▶ GameMainPage::applyTrait()
    │         ├──▶ GameMainPage::resumeBattleState()
    │
    └──▶ setCurrentPage(PageId::xxx)  // 切换 QStackedWidget
```

#### 模式 B：三级 Setup

每个 QWidget 页面都有三个私有方法：

| 方法 | 作用 | 示例 |
| ---- | ---- | ---- |
| `setupUi()` | 创建控件、设置 Layout | `new QLabel`, `new QPushButton`, `QVBoxLayout` |
| `setupStyle()` | QSS 样式表 | `setStyleSheet("background: #333")` |
| `setupConnections()` | 绑定内部信号槽 | `connect(button, &QPushButton::clicked, ...)` |

#### 模式 C：事件过滤器统一入口

```cpp
bool ClassSelectPage::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        for (auto *card : m_cards) {
            if (watched == card) {
                selectCard(m_cards.indexOf(card));
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}
```

#### 模式 D：GameMainPage 的特殊性

GameMainPage 是**唯一一个既是页面又是编排器**的类：

- 作为 QWidget：管理子控件（12 个 Label、3 个 ProgressBar、1 个 CheckBox、1 个 Button）
- 作为编排器：持有 3 个子系统 + 3 个渲染组件的引用，驱动 `handleBattleTick()` 主循环
- 代码量最大：`game_main_page.h` 134 行 + `.cpp` 约 1282 行

---

## 七、类型六：子系统层（Systems）

**文件列表：**

| 文件 | 行数 | 类名 | 构造依赖 | 核心能力 |
| ---- | ---- | ---- | -------- | -------- |
| [game_factory.h](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/game_factory.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/game_factory.cpp) | 68 + 205 | `GameFactory` | QObject | 创建所有 Data/View 对象 |
| [wave_manager.h](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/wave_manager.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/wave_manager.cpp) | 82 + 430 | `WaveManager` (抽象) + `BasicWaveManager` (匿名) | QObject | 波次推进、经验计算、Boss 调度 |
| [upgrade_resolver.h](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/upgrade_resolver.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/upgrade_resolver.cpp) | 41 + 171 | `UpgradeResolver` | Player, Weapon, Factory | 升级逻辑、特性追踪 |
| [combat_coordinator.h](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/combat_coordinator.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/combat_coordinator.cpp) | 48 + 343 | `CombatCoordinator` | Factory, Scene, Player, Weapon | 子弹创建、碰撞、清理 |
| [enemy_director.h](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/enemy_director.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/systems/enemy_director.cpp) | 53 + 331 | `EnemyDirector` | Factory, Scene, ParentWidget | 敌人生成、Boss 管理 |

### 7.1 代码作用

**封装游戏逻辑的模块** —— 每个 System 类负责游戏的一个独立方面。接收外部依赖（构造注入），操作数据模型，不直接处理 UI 渲染。

这些是从 V1 单体 `game_main_page.cpp` 中通过 **Plan B 拆分**出的逻辑单元。

### 7.2 内容分析

#### GameFactory —— 唯一的"真正工厂"

```cpp
class GameFactory : public QObject {
    Q_OBJECT
public:
    // ① 查询包装：委托给 game_tables.h
    [[nodiscard]] const PlayerClassConfig *playerClassConfig(PlayerClassId id) const noexcept;
    [[nodiscard]] const WeaponConfig *weaponConfig(WeaponId id) const noexcept;
    // ...

    // ② 数据对象创建
    [[nodiscard]] Player *createPlayer(PlayerClassId, QObject *parent = nullptr) const;
    [[nodiscard]] Weapon *createStarterWeapon(PlayerClassId, Player *owner, ...) const;
    [[nodiscard]] EnemyData *createEnemyData(EnemyId, const QPointF &, ...) const;
    [[nodiscard]] BulletData *createBulletData(WeaponId, const QPointF &, const QPointF &, ...) const;

    // ③ 视图对象创建
    [[nodiscard]] EnemyView *createEnemyView(EnemyData *data, ...) const;
    [[nodiscard]] BulletView *createBulletView(BulletData *data, ...) const;

    // ④ 复合创建（返回 Entity 结构体）
    [[nodiscard]] EnemyEntity createEnemyEntity(EnemyId, const QPointF &, ...) const;
    [[nodiscard]] BulletEntity createBulletEntity(WeaponId, const QPointF &, const QPointF &, ...) const;

    // ⑤ 特殊子弹
    [[nodiscard]] BulletData *createSpecialBulletData(const BulletSpecialTemplate &, ...) const;
    [[nodiscard]] BulletView *createSpecialBulletView(BulletData *, ...) const;

    // ⑥ 特性
    [[nodiscard]] Trait *createTrait(TraitId, ...) const;

    // ⑦ 实体结构体
    struct EnemyEntity { EnemyData *data; EnemyView *view; };
    struct BulletEntity { BulletData *data; BulletView *view; };
};
```

#### CombatCoordinator —— 战斗交互协调

```cpp
class CombatCoordinator : public QObject {
    Q_OBJECT
public:
    explicit CombatCoordinator(GameFactory *factory, QGraphicsScene *scene,
                               Player *player, Weapon *weapon, QObject *parent = nullptr);

    // ① 开火：创建子弹（含普通/特殊子弹路由 + 扇形扩散）
    void handleWeaponFireRequested(WeaponId, const QPointF &origin,
                                   const QPointF &direction,
                                   QVector<GameFactory::BulletEntity> &bullets);

    // ② 碰撞：玩家子弹 vs 敌人、敌人子弹 vs 玩家、接触伤害
    void resolveCombatCollisions(QVector<GameFactory::BulletEntity> &bullets,
                                 QVector<GameFactory::EnemyEntity> &enemies,
                                 QGraphicsEllipseItem *playerMarker,
                                 float &playerDamageCooldownRemainingMs,
                                 const QMap<TraitId, int> &traitCounts);

    // ③ 清理：过期子弹、击败敌人
    void cleanupExpiredBullets(QVector<GameFactory::BulletEntity> &bullets);
    void cleanupDefeatedEnemies(QVector<GameFactory::EnemyEntity> &enemies,
                                WaveManager *wm,
                                const QMap<TraitId, int> &traitCounts);

    void setActiveBulletStyle(BulletStyle style);
    [[nodiscard]] BulletStyle activeBulletStyle() const noexcept;

private:
    GameFactory *m_factory;
    QGraphicsScene *m_scene;
    Player *m_player;
    Weapon *m_weapon;
    BulletStyle m_activeBulletStyle{BulletStyle::Normal};
};
```

#### EnemyDirector —— 敌人生成与 Boss 管理

```cpp
class EnemyDirector : public QObject {
    Q_OBJECT
public:
    explicit EnemyDirector(GameFactory *factory, QGraphicsScene *scene,
                           QWidget *parentWidget, QObject *parent = nullptr);

    void setStateVectors(QVector<GameFactory::EnemyEntity> *enemies,
                         QVector<GameFactory::BulletEntity> *bullets);

    // 普通敌人生成
    void spawnTestEnemy(int maxConcurrent, bool bossIsActive, EnemyId currentBossId);

    // Boss 生成管道（含延迟、阶段切换）
    void spawnBossIfPending(WaveManager *wm, bool &bossIsActive, EnemyId &currentBossId);
    void onBossEntityCreated(GameFactory::EnemyEntity entity);

    // Boss 血条 UI 管理
    void updateBossHealthBar(EnemyData *boss, QLabel *hpLabel, QProgressBar *hpBar);
    void showBossHealthBar(const QString &name, QFrame *&panel, QLabel *&label, QProgressBar *&bar);
    void hideBossHealthBar(QFrame *&panel, QLabel *&label, QProgressBar *&bar);

    [[nodiscard]] QPointF randomEnemySpawnPosition() const;
    [[nodiscard]] static EnemyId randomEnemyId();

private:
    void connectEnemyShootSignals(EnemyData *enemyData);   // 连接敌人射击信号

    GameFactory *m_factory;
    QGraphicsScene *m_scene;
    QWidget *m_parentWidget;
    QVector<GameFactory::EnemyEntity> *m_enemies{nullptr}; // 外部容器引用
    QVector<GameFactory::BulletEntity> *m_bullets{nullptr};// 外部容器引用
    QFrame *m_bossHpPanel{nullptr};                        // Boss 血条面板
    QLabel *m_bossHpLabel{nullptr};
    QProgressBar *m_bossHpBar{nullptr};
};
```

#### UpgradeResolver —— 升级逻辑管理

```cpp
class UpgradeResolver : public QObject {
    Q_OBJECT
public:
    explicit UpgradeResolver(Player *player, Weapon *weapon, GameFactory *factory,
                             QObject *parent = nullptr);

    void applyTrait(TraitId traitId);                     // 应用特性
    void applyWeaponUpgrade(WeaponUpgradeId upgradeId);   // 应用武器强化
    void setClassId(PlayerClassId classId);
    [[nodiscard]] QString lastUpgradeSummary() const;     // 属性变化摘要（HTML）
    [[nodiscard]] const QMap<TraitId, int> &traitCounts() const;
    [[nodiscard]] const QList<TraitId> &ownedTraits() const;
    void clearTraits();

signals:
    void statsChanged();

private:
    Player *m_player;
    Weapon *m_weapon;
    GameFactory *m_factory;
    PlayerClassId m_classId{PlayerClassId::Warrior};
    QMap<TraitId, int> m_traitCounts;       // 特性等级计数
    QList<TraitId> m_ownedTraits;           // 已拥有特性列表
    QString m_lastUpgradeSummary;           // 上次升级摘要
};
```

### 7.3 共同模式

```
class XxxSystem : public QObject {     // ① QObject 子类
    Q_OBJECT
public:
    // ② 构造注入依赖（依赖倒置原则）
    explicit XxxSystem(GameFactory *factory, QGraphicsScene *scene, ...);

    // ③ 以动词开头的方法名（命令式 API）
    void handleXxx(...);
    void spawnXxx(...);
    void resolveXxx(...);
    void cleanupXxx(...);

    // ④ 只暴露必要信号
signals:
    void stateChanged();

private:
    // ⑤ 通过指针持有外部依赖（不拥有所有权）
    GameFactory *m_factory;
    QGraphicsScene *m_scene;
    // ...
};
```

#### 模式 A：构造注入依赖

所有 System 类在构造函数中接收外部依赖，不自己创建：

```cpp
// UpgradeResolver 依赖 Player 和 Weapon
UpgradeResolver(Player *player, Weapon *weapon, GameFactory *factory, ...);

// CombatCoordinator 依赖 Factory, Scene, Player, Weapon
CombatCoordinator(GameFactory *factory, QGraphicsScene *scene,
                  Player *player, Weapon *weapon, ...);

// EnemyDirector 依赖 Factory, Scene, ParentWidget
EnemyDirector(GameFactory *factory, QGraphicsScene *scene,
              QWidget *parentWidget, ...);
```

#### 模式 B：动词开头的方法命名

| 子系统 | 方法名 | 语义 |
| ------ | ------ | ---- |
| CombatCoordinator | `handleWeaponFireRequested` | 处理"武器开火"请求 |
| CombatCoordinator | `resolveCombatCollisions` | 解决战斗碰撞 |
| CombatCoordinator | `cleanupExpiredBullets` | 清理过期子弹 |
| CombatCoordinator | `cleanupDefeatedEnemies` | 清理击败敌人 |
| EnemyDirector | `spawnTestEnemy` | 生成测试敌人 |
| EnemyDirector | `spawnBossIfPending` | 生成待定 Boss |
| EnemyDirector | `updateBossHealthBar` | 更新 Boss 血条 |
| EnemyDirector | `showBossHealthBar` | 显示 Boss 血条 |
| EnemyDirector | `hideBossHealthBar` | 隐藏 Boss 血条 |
| UpgradeResolver | `applyTrait` | 应用特性 |
| UpgradeResolver | `applyWeaponUpgrade` | 应用武器强化 |

#### 模式 C：操作外部容器

System 类**不自己持有敌人/子弹列表**，而是操作 `QVector<>` 外部容器的引用：

```cpp
void CombatCoordinator::handleWeaponFireRequested(WeaponId, const QPointF &origin,
    const QPointF &direction,
    QVector<GameFactory::BulletEntity> &bullets    // ← 外部引用
) {
    // 创建新子弹，追加到外部容器
    auto entity = m_factory->createBulletEntity(...);
    bullets.append(entity);
}

void CombatCoordinator::cleanupExpiredBullets(
    QVector<GameFactory::BulletEntity> &bullets    // ← 外部引用
) {
    bullets.erase(
        std::remove_if(bullets.begin(), bullets.end(), [](auto &b) {
            return b.data->isExpired();
        }),
        bullets.end()
    );
}
```

#### 模式 D：处理后的数据写回引用

`resolveCombatCollisions` 是典型代表 —— 传入**非 const 引用**，修改后从引用反映到调用者：

```cpp
void CombatCoordinator::resolveCombatCollisions(
    QVector<GameFactory::BulletEntity> &bullets,    // 会被修改（击败的敌人移除）
    QVector<GameFactory::EnemyEntity> &enemies,     // 会被修改（过期子弹移除）
    QGraphicsEllipseItem *playerMarker,             // 只读
    float &playerDamageCooldownRemainingMs,         // 会被修改（冷却计时）
    const QMap<TraitId, int> &traitCounts           // 只读
);
```

## 八、类型七：特殊子弹系统（Bullets）

**文件列表：**

| 文件 | 行数 | 内容 |
| ---- | ---- | ---- |
| [special_bullet_config.h](file:///F:/QT%20projects/emoji_vibe_coding/src/bullets/special_bullet_config.h) | 54 | 13 个 `BulletSpecialTemplate` + 生成升级选项函数 |
| [special_bullet_data.h](file:///F:/QT%20projects/emoji_vibe_coding/src/bullets/special_bullet_data.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/bullets/special_bullet_data.cpp) | 45 + 110 | `SpecialBulletData` — 3 种特效逻辑 |
| [special_bullet_view.h](file:///F:/QT%20projects/emoji_vibe_coding/src/bullets/special_bullet_view.h)/[.cpp](file:///F:/QT%20projects/emoji_vibe_coding/src/bullets/special_bullet_view.cpp) | 24 + 125 | `SpecialBulletView` — emoji PNG 渲染 |

### 8.1 代码作用

在基础子弹系统之上，**扩展出 12 种具有独立逻辑和外观的特殊子弹**。遵循与基础 Model/View 同样的 MV 分离模式，但独立放置于 `src/bullets/` 目录中。

### 8.2 内容分析

#### special_bullet_config.h —— 配置表

```cpp
namespace BulletTemplateConfig {

inline const QList<BulletSpecialTemplate> kAllBulletTemplates {
    // 索引0: 普通弹（预留）
    {BulletStyle::Normal, "普通弹", 0, 0, SpecialEffect::None, ...},
    // 索引1~12: 12 种特殊子弹
    {BulletStyle::Dagger, "飞刀", 12, 550, SpecialEffect::None, ...},
    {BulletStyle::SunOrb, "太阳法球", 18, 320, SpecialEffect::None, ...},
    {BulletStyle::MoonOrb, "月亮法球", 14, 340, SpecialEffect::None, ...},
    {BulletStyle::Hacimi, "哈基米", 8, 300, SpecialEffect::TrackAndAttach, 3.0F, 0.15F,
     "🐱", ":/images/grinning_cat_3d.png",
     {4 张备用猫图}, 3},
    {BulletStyle::ThunderSpear, "雷霆之矛", 22, 600, SpecialEffect::None, ...},
    {BulletStyle::Boomerang, "回力镖", 10, 250, SpecialEffect::None, ...},
    {BulletStyle::BloodArrow, "血之箭", 16, 450, SpecialEffect::None, ...},
    {BulletStyle::StunBullet, "不是哥们！？", 15, 200, SpecialEffect::Stun, 1.5F, ...},
    {BulletStyle::RandomGift, "随机礼物", 5, 350, SpecialEffect::None, ...},
    {BulletStyle::Comet, "彗星", 20, 280, SpecialEffect::None, ...},
    {BulletStyle::PushBullet, "带派不老铁", 5, 400, SpecialEffect::KnockbackWithBonus, 150, 0.3F, ...},
    {BulletStyle::Rocket, "火箭", 25, 180, SpecialEffect::None, ...},
};

[[nodiscard]] inline const BulletSpecialTemplate *findBulletTemplate(BulletStyle style) {
    for (const auto &t : kAllBulletTemplates) {
        if (t.style == style) return &t;
    }
    return nullptr;
}

[[nodiscard]] inline UpgradeOptions generateSpecialBulletUpgradeOptions() {
    // 遍历索引1~12，生成 12 个 UpgradeOption（kind=Stat, rarity=Epic）
}
} // namespace BulletTemplateConfig
```

#### SpecialBulletData —— 特殊子弹数据 + 特效逻辑

```cpp
struct BulletSpecialConfig {
    SpecialEffect effect{SpecialEffect::None};
    float param1{0.0F};
    float param2{0.0F};
    QString emojiIcon;
    QString imagePath;
    QList<QString> altImagePaths;
};

class SpecialBulletData final : public BasicBulletData {
    Q_OBJECT
public:
    SpecialBulletData(WeaponId weaponId, const BulletConfig *config,
                      const QPointF &spawnPos, const QPointF &direction,
                      const BulletSpecialConfig &specialConfig,
                      QObject *parent = nullptr);

    [[nodiscard]] SpecialEffect specialEffect() const;
    [[nodiscard]] const BulletSpecialConfig &specialConfig() const;
    [[nodiscard]] QString currentImagePath() const;
    [[nodiscard]] bool isAttached() const;
    void setAttached(bool attached, int targetIndex);
    [[nodiscard]] int attachedTargetIndex() const;
    [[nodiscard]] float attachTimer() const;
    void advanceAttachTimer(float dt);
    void advanceSpecialFrame(float deltaSeconds, const QVector<QPointF> &enemyPositions);

    void advanceFrame(float deltaSeconds) override;

private:
    BulletSpecialConfig m_specialConfig;
    bool m_attached{false};
    int m_attachedTarget{-1};
    float m_attachTimer{0.0F};
    int m_imageCycleIndex{0};
};
```

#### SpecialBulletView —— 特殊子弹视图

```cpp
class SpecialBulletView final : public BulletView {     // ← 继承 BulletView，不是 BasicBulletView！
    Q_OBJECT
public:
    explicit SpecialBulletView(QGraphicsItem *parent = nullptr);
    [[nodiscard]] QRectF boundingRect() const override;
    [[nodiscard]] QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    [[nodiscard]] BulletData *model() const override;

public slots:
    void bindModel(BulletData *data) override;
    void syncFromData() override;

private:
    QPointer<BulletData> m_model;
    qreal m_radius{6.0};
    QPixmap m_iconPixmap;    // emoji PNG 图片缓存
};
```

**核心渲染逻辑**（paint 方法中）：

```cpp
void SpecialBulletView::paint(...) {
    if (!m_iconPixmap.isNull()) {
        // 有贴图 → 绘制 PNG
        painter->drawPixmap(boundingRect().toRect(), m_iconPixmap);
    } else {
        // 无贴图 → fallback 彩色圆
        painter->setBrush(QColor("#FFD700"));
        painter->drawEllipse(boundingRect());
    }
}
```

### 8.3 共同模式

#### 模式 A：并行继承体系

这是整个项目中**唯一出现并行继承**的地方：

```
BulletView (抽象接口)
    ├── BasicBulletView (几何图形渲染, final)       ← 基础子弹
    └── SpecialBulletView (PNG 贴图渲染, final)     ← 特殊子弹（同级兄弟）
```

SpecialBulletView **不**继承 BasicBulletView，两者是**兄弟关系**，都直接继承 BulletView 抽象接口。

#### 模式 B：Data 链式继承

与 View 不同，Data 侧是链式继承：

```
BulletData → BasicBulletData → SpecialBulletData
```

SpecialBulletData 复用 BasicBulletData 的飞行逻辑（`advanceFrame`），并增加：
- `advanceSpecialFrame()` — 特效逻辑（追踪/眩晕/击退）
- 新增成员：`m_specialConfig`, `m_attached`, `m_attachTimer`

#### 模式 C：配置 + Data + View 三文件

```
special_bullet_config.h           → 配置层：声明 12 种子弹模板
special_bullet_data.h / .cpp      → 数据层：特效逻辑
special_bullet_view.h / .cpp      → 视图层：emoji 渲染
```

职责分离清晰：
- **config**：定义"哪些子弹存在" + "生成升级选项"
- **data**：定义"子弹的特效逻辑是什么"
- **view**：定义"子弹长什么样子"

#### 模式 D：特效用 enum + 参数编码

不通过多态/继承区分特效，而是用枚举 + 浮点参数：

```cpp
// 枚举标识
enum class SpecialEffect { None, TrackAndAttach, Stun, KnockbackWithBonus };

// 参数编码
{BulletStyle::StunBullet, ..., SpecialEffect::Stun, 1.5F, 0.0F, ...};
//                                                     ↑
//                                                param1 = 眩晕时长 1.5 秒

// 运行时 switch 分支
void SpecialBulletData::advanceSpecialFrame(float dt, ...) {
    switch (m_specialConfig.effect) {
        case SpecialEffect::Stun: /* 命中后眩晕敌人 param1 秒 */ break;
        case SpecialEffect::TrackAndAttach: /* 追踪 → 挂身 → 持续伤害 */ break;
        case SpecialEffect::KnockbackWithBonus: /* 击退 + 撞墙奖励 */ break;
    }
}
```

---

## 九、跨类型共同模式总结

### 9.1 继承树的两极分化

整个项目 57 个文件形成三个清晰的继承层次：

```
┌─────────────────────────────────────────────────────────┐
│              QObject 层次（数据/逻辑）                    │
│                                                         │
│  QObject                                                 │
│    ├── Player (抽象) → BasicPlayer (final)               │
│    ├── Weapon (抽象) → BasicWeapon (final)               │
│    ├── EnemyData (抽象) → BasicEnemyData (final)         │
│    ├── BulletData (抽象) → BasicBulletData → SpecialBD   │
│    ├── Trait (抽象) → BasicTrait (final)                 │
│    ├── WaveManager (抽象) → BasicWaveManager (匿名)      │
│    ├── GameFactory                                       │
│    ├── UpgradeResolver / CombatCoordinator / EnemyDirector│
│    └── GameMainPage (通过 QWidget 间接继承)               │
├─────────────────────────────────────────────────────────┤
│            QGraphics 层次（渲染）                         │
│                                                         │
│  QGraphicsObject                                         │
│    └── GameView (抽象)                                   │
│          ├── EnemyView (抽象) → BasicEnemyView (final)   │
│          └── BulletView (抽象)                           │
│                ├── BasicBulletView (final)               │
│                └── SpecialBulletView (final)             │
│                                                         │
│  QGraphicsItem                                           │
│    └── PlayerAvatarItem (final)                          │
│                                                         │
│  QGraphicsView                                           │
│    └── BattleArenaView (final)                           │
├─────────────────────────────────────────────────────────┤
│              QWidget 层次（页面/组件）                    │
│                                                         │
│  QWidget                                                 │
│    ├── ClassSelectPage                                   │
│    ├── GameMainPage (含编排器逻辑)                        │
│    ├── UpgradePage                                       │
│    └── DashCooldownWidget                                │
│                                                         │
│  QMainWindow                                             │
│    └── EmojiDungeonWindow                                │
└─────────────────────────────────────────────────────────┘
```

### 9.2 配置驱动的设计

**所有类的初始值都不是硬编码的** —— 都从 `game_tables.h` 中的配置表读取：

```
game_tables.h
    ├── kPlayerClasses[3]      ──→ BasicPlayer::BasicPlayer(const PlayerClassConfig*)
    ├── kWeapons[3]            ──→ BasicWeapon::BasicWeapon(const WeaponConfig*)
    ├── kEnemies[10]           ──→ BasicEnemyData::BasicEnemyData(const EnemyConfig*)
    ├── kTraits[10]            ──→ BasicTrait::BasicTrait(const TraitConfig*)
    └── kExperienceThresholds  ──→ WaveManager::levelForExperience()
```

```cpp
// 每个 BasicXxx 的构造器都接收 Config 指针
BasicPlayer::BasicPlayer(const PlayerClassConfig *config, ...) : m_config(config) {
    m_currentHealth = config->maxHealth;        // ← 从配置表读取
    m_weaponId = config->starterWeaponId;       // ← 从配置表读取
}
```

### 9.3 QObject 信号/槽三层解耦

整个游戏通过 Qt 信号/槽实现 Model → View → System 三层解耦：

```
Model 层（数据变化时发射信号）
    │
    ├── emit positionChanged()      ──→ View 层 syncFromData()
    ├── emit healthChanged()        ──→ View 层更新血条
    ├── emit fireRequested()        ──→ System 层 handleWeaponFireRequested()
    ├── emit requestShoot()         ──→ System 层 onEnemyShootRequested()
    └── emit defeated()             ──→ System 层 cleanupDefeatedEnemies()
```

```cpp
// GameMainPage 中的信号连接
connect(m_weapon, &Weapon::fireRequested,
        m_combatCoordinator, &CombatCoordinator::handleWeaponFireRequested);

connect(m_player, &Player::dashCooldownChanged,
        m_dashCooldownWidget, &DashCooldownWidget::updateCooldown);
```

### 9.4 构造器三件套

几乎所有类都遵循相同的构造器约定：

```cpp
// 第 1 件：explicit 防止隐式转换
explicit BasicPlayer(const PlayerClassConfig *config, QObject *parent = nullptr);

// 第 2 件：构造时从配置表初始化
BasicPlayer::BasicPlayer(const PlayerClassConfig *config, ...) : Player(parent) {
    m_currentHealth = config->maxHealth;
}

// 第 3 件：成员变量 {} 零值初始化
private:
    const PlayerClassConfig *m_config{nullptr};   // 指针 → nullptr
    float m_currentHealth{0.0F};                   // 浮点 → 0.0
    bool m_isDashing{false};                       // 布尔 → false
```

### 9.5 `[[nodiscard]]` 全覆盖

**所有查询方法都标记 `[[nodiscard]]`** —— 编译器会警告返回值被丢弃：

```cpp
// 类型一（配置层）
[[nodiscard]] inline const EnemyConfig *findEnemyConfig(EnemyId id);

// 类型二（接口层）
[[nodiscard]] virtual float currentHealth() const = 0;

// 类型三（实现层）
[[nodiscard]] float currentHealth() const override;

// 类型六（子系统）
[[nodiscard]] BulletStyle activeBulletStyle() const noexcept;
```

整个代码库中约有 **200+ 个 `[[nodiscard]]` getter**，是贯穿所有类型的一致性要求。

### 9.6 相同的文件头骨架

所有 `.h` 文件的起始结构完全一致：

```cpp
#pragma once

#include <QtCore/QType>
#include "game_data.h"

class ForwardDecl;

class Xxx : public QObject
{
    Q_OBJECT

public:
    explicit Xxx(QObject *parent = nullptr);
    ~Xxx() override = default;
};
```

### 9.7 类型间互操作总结

| 类型一 (Config) | 被所有其他类型依赖 |
| 类型二 (Interfaces) | 被类型三实现 |
| 类型三 (Implementations) | 被类型四 (View) 的模型绑定、类型六 (System) 操作 |
| 类型四 (View) | 绑定类型三的数据对象，按类型一中的配置渲染 |
| 类型五 (Pages) | 编排类型六 (System) 的调用来管理类型三和类型四 |
| 类型六 (Systems) | 通过类型一 (GameFactory) 创建类型三和类型四的对象 |
| 类型七 (Bullets) | 扩展类型三和类型四，使用类型一的配置 |

---

*文档结束。基于对项目 57 个源文件的完整代码阅读和分析。*