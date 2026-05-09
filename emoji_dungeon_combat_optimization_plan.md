# Emoji Dungeon 战斗系统优化开发计划

## 1. 项目概述

本项目旨在对 Emoji Dungeon 游戏的战斗系统进行深度优化，主要包含两大核心功能：
1. **玩家闪避能力**：通过空格键触发短途冲刺，提升操作手感与生存空间
2. **敌人系统重构**：引入三大系列敌人，包含多样化 AI、Boss 弹幕、分档刷新频率及系列锁定规则

## 2. 当前代码架构分析

### 2.1 现有系统结构
- **核心类**：Player、EnemyData、BulletData、WaveManager、GameMainPage
- **数据配置**：GameConfig 命名空间包含所有配置数据
- **UI 层**：GameView 基类，派生 EnemyView、BulletView
- **战斗管理**：WaveManager 处理波次推进与状态切换

### 2.2 现有敌人系统
```cpp
enum class EnemyId {
    Slime,      // 待删除
    Bat,        // 待删除  
    Skeleton    // 待删除
};
```

## 3. 玩家闪避能力开发计划

### 3.1 功能规格
- **触发方式**：空格键触发冲刺
- **冲刺方向**：
  - 优先移动输入方向（WASD）
  - 次选最后瞄准方向（鼠标位置）
- **核心参数**：
  - 冲刺距离：100 像素
  - 持续时间：0.15 秒
  - 冷却时间：2.5 秒
  - 冲刺期间移速倍率：4 倍

### 3.2 实现步骤

#### 阶段 1：Player 类扩展
**文件：player.h / 新实现类**
```cpp
// Player 类新增虚函数
virtual void dash() = 0;
virtual bool isDashing() const = 0;
virtual float dashCooldownRemaining() const = 0;
virtual void updateDash(float deltaSeconds) = 0;
```

#### 阶段 2：BasicPlayer 实现
**新类：basic_player.cpp / basic_player.h**
- 实现冲刺状态管理
- 添加计时器成员变量：
  - `dashDurationRemaining`
  - `dashCooldownRemaining`
  - `dashHardStunRemaining`
- 实现 `dash()` 方法逻辑
- 实现 `updateDash()` 帧更新

#### 阶段 3：GameMainPage 集成
**文件：game_main_page.cpp**
1. **事件处理**：
```cpp
// eventFilter 中捕获空格键
if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Space && m_player) {
        m_player->dash();
        return true;
    }
}
```

2. **移动更新**：
```cpp
// updatePlayerMovement 中检测冲刺状态
if (m_player->isDashing()) {
    // 使用冲刺速度和方向
    QPointF dashVelocity = dashDirection * m_player->moveSpeed() * 4.0f;
    // ...
}
```

3. **碰撞检测**：
```cpp
// resolveCombatCollisions 中跳过接触伤害
if (m_player->isDashing()) {
    // 无敌帧，跳过伤害计算
    continue;
}
```

#### 阶段 4：UI 显示
- 状态栏添加冲刺冷却指示器
- 冲刺期间玩家视觉反馈（透明化/残影效果）

### 3.3 技术要点
1. **方向计算**：优先检查移动输入 `m_moveInput`，否则使用 `m_lastAimDirection`
2. **无敌帧**：冲刺期间 `isDashing() == true` 时免疫接触伤害
3. **硬直时间**：冲刺结束后 0.1 秒内不可再次冲刺
4. **冷却显示**：`dashCooldownRemaining()` 返回剩余秒数

## 4. 敌人系统重构开发计划

### 4.1 数据模型扩展

#### 阶段 1：扩展 EnemyId 枚举
**文件：game_data.h**
```cpp
enum class EnemyId {
    // 怪异系列
    Ogre,        // 食人魔
    Jester,      // 小丑脸
    DemonLord,   // 恶魔领主
    
    // 亡灵系列
    SkeletonNew, // 新骷髅
    Ghost,       // 幽灵
    BoneLord,    // 骷髅与交叉骨
    
    // 外星系列
    Robot,       // 机器人
    XenoBeast,   // 外星怪物
    UFO,         // 飞碟（阶段1）
    AlienPilot   // 外星人（阶段2）
};
```

#### 阶段 2：扩展 EnemyConfig 结构体
```cpp
struct EnemyConfig {
    EnemyId id;
    QString displayName;
    QString summary;
    float maxHealth;
    float moveSpeed;
    float contactDamage;
    float collisionRadius;
    
    // 新增行为字段
    enum Behavior { 
        Chase,          // 追逐
        KeepDistance,   // 保持距离
        Charge,         // 冲锋
        ShootAndMove,   // 射击移动
        SuicideBomb,    // 自杀式爆炸
        Boss            // Boss行为
    } behavior;
    
    // 行为参数
    float keepDistanceMin;      // 保持距离的最小距离
    float chargeUpMs;           // 冲锋准备时间
    float chargeDurationMs;     // 冲锋持续时间
    float chargeSpeedMult;      // 冲锋速度倍率
    float chargeCooldownMs;     // 冲锋冷却时间
    float shootIntervalMs;      // 射击间隔
    int shootBulletCount;       // 每次射击子弹数
    float bulletSpeed;          // 子弹速度
    float bulletDamage;         // 子弹伤害
    bool slowImmune;            // 减速免疫
    bool knockbackImmune;       // 击退免疫
    int spawnOnDeathId;         // 死亡时生成的新敌人ID
    float suicideRadius;        // 自爆半径
    float suicideDamage;        // 自爆伤害
};
```

### 4.2 怪物系列与 AI 设计

#### 怪异系列
| 怪物 | 行为 | AI 描述 | 特殊参数 |
|------|------|---------|----------|
| 食人魔 | Charge | 慢速追逐，每4秒蓄力2秒后以2倍速冲锋1秒 | `chargeUpMs=4000`, `chargeDurationMs=1000`, `chargeSpeedMult=2.0` |
| 小丑脸 | SuicideBomb | 高速冲撞玩家，贴脸时自爆 | `suicideRadius=80`, `suicideDamage=25` |
| 恶魔 | Boss | 站场中央不动，每2秒发射12发圆形弹幕 | `shootIntervalMs=2000`, `shootBulletCount=12` |

#### 亡灵系列
| 怪物 | 行为 | AI 描述 | 特殊参数 |
|------|------|---------|----------|
| 新骷髅 | Chase | 纯追逐，无特殊 | - |
| 幽灵 | KeepDistance | 与玩家保持150px距离，不攻击 | `keepDistanceMin=150` |
| 骷髅与交叉骨 | Boss | 站场中央，每1.5秒射出两圈交叉弹幕 | `shootIntervalMs=1500`, `shootBulletCount=16` |

#### 外星系列
| 怪物 | 行为 | AI 描述 | 特殊参数 |
|------|------|---------|----------|
| 机器人 | ShootAndMove | 追至300px内停下，每1秒射击玩家一次 | `shootIntervalMs=1000` |
| 外星怪物 | Chase | 距离>200px时移速+30%快速突进 | 动态速度调整 |
| 飞碟→外星人 | Boss (2阶段) | P1：绕玩家移动+扇形弹幕；P2：站中央密集圆形弹幕 | 阶段切换机制 |

### 4.3 数值配置表

| 怪物 | 生命 | 移速 | 接触伤害 | 半径 | 特殊攻击参数 |
|------|------|------|----------|------|--------------|
| **怪异系列** | | | | | |
| 食人魔 | 80 | 60 | 20 | 22 | 冲锋2倍速持续1s，冷却4s |
| 小丑脸 | 15 | 200 | 30 | 10 | 自爆半径80，伤害25 |
| 恶魔 | 300 | 0 | 0 | 30 | 弹幕2000ms间隔，12发，速度180，伤害6 |
| **亡灵系列** | | | | | |
| 新骷髅 | 20 | 90 | 8 | 18 | - |
| 幽灵 | 10 | 180 | 0 | 10 | 保持150px距离 |
| 骷髅与交叉骨 | 250 | 0 | 0 | 28 | 弹幕1500ms间隔，双圈8+8发，速度220，伤害5 |
| **外星系列** | | | | | |
| 机器人 | 40 | 110 | 0 | 16 | 射击间隔1000ms，子弹速度250，伤害4 |
| 外星怪物 | 35 | 170 | 18 | 14 | 距离>200px时移速×1.3 |
| 飞碟(阶段1) | 150 | 100 | 0 | 32 | 弹幕2000ms间隔，8发扇形±30°，速度200，伤害6 |
| 外星人(阶段2) | 200 | 0 | 0 | 26 | 弹幕1000ms间隔，16发圆形，速度240，伤害5 |

### 4.4 刷新频率梯度设计

| 梯度 | 波次范围 | 同时最大敌人数 | 刷新间隔(ms) | 怪物组合 |
|------|----------|----------------|--------------|----------|
| 1 | 1-2 | 5 | 4000 | 2骷髅 + 3幽灵 |
| 2 | 3-4 | 7 | 3000 | 2食人魔 + 2小丑脸 + 2骷髅 + 1幽灵 |
| 3 | 5-7 | 9 | 2500 | 3机器人 + 3外星怪物 + 1食人魔 + 1小丑脸 |
| 4 | 8-10 | 12 | 2000 | 2食人魔 + 2机器人 + 2外星怪物 + 1幽灵 |

### 4.5 Boss 出场节点
1. **第4波**：恶魔（怪异Boss）
2. **第7波**：骷髅与交叉骨（亡灵Boss）
3. **第9波**：飞碟→外星人（外星Boss，二阶段）

### 4.6 Boss 系列锁定规则
- **怪异Boss**（波次4）：刷新间隔 3000ms → 2400ms，上限 7 → 9
- **亡灵Boss**（波次7）：刷新间隔 2500ms → 2000ms，上限 9 → 10
- **外星Boss**（波次9）：刷新间隔 2000ms → 1600ms，上限 12 → 13

**规则**：Boss 存活期间只刷新与其同系列的普通怪物，死亡后恢复原梯度规则。

### 4.7 动态难度系统
- 随波次增加，普通怪物生命 ×1.05/波
- 伤害 ×1.03/波
- 保持后期挑战性

## 5. 技术实施要点

### 5.1 接口扩展
**文件：enemy_data.h**
```cpp
class EnemyData : public QObject {
    // 新增信号
signals:
    void requestShoot(QPointF direction, int bulletCount, float bulletSpeed, float bulletDamage);
    void requestSuicideExplosion(float radius, float damage);
    
    // 新增虚函数
public:
    virtual Behavior currentBehavior() const = 0;
    virtual void updateAI(float deltaSeconds, QPointF playerPosition) = 0;
    virtual void enterBossPhase(int phase) = 0;
};
```

### 5.2 AI 实现
**新文件：basic_enemy_data.cpp**
- 实现多行为状态机
- 射击计时器管理
- 弹幕生成算法
- Boss 阶段切换逻辑

### 5.3 子弹系统扩展
**文件：bullet_data.h**
```cpp
// BulletData 新增标记
[[nodiscard]] virtual bool isEnemyBullet() const = 0;
virtual void setEnemyBullet(bool enemy) = 0;
```

**碰撞检测调整**：
- 敌方子弹：仅对玩家有效
- 玩家子弹：仅对敌人有效

### 5.4 波次管理器增强
**文件：wave_manager.cpp**
- 添加当前系列锁定状态
- 实现动态刷新参数调整
- Boss 阶段切换触发器

## 6. 开发里程碑

### 里程碑 1：玩家闪避系统（预计 3 天）
- [ ] Player 接口扩展
- [ ] BasicPlayer 冲刺实现
- [ ] GameMainPage 事件集成
- [ ] 碰撞检测无敌帧
- [ ] UI 冷却显示

### 里程碑 2：敌人数据模型（预计 2 天）
- [ ] EnemyId 枚举扩展
- [ ] EnemyConfig 结构体增强
- [ ] 数值配置表实现
- [ ] 动态难度系统

### 里程碑 3：AI 行为系统（预计 4 天）
- [ ] BasicEnemyData AI 实现
- [ ] 多行为状态机
- [ ] 射击与弹幕系统
- [ ] Boss 阶段切换

### 里程碑 4：波次与刷新系统（预计 2 天）
- [ ] 梯度刷新频率实现
- [ ] Boss 出场节点
- [ ] 系列锁定规则
- [ ] 动态参数调整

### 里程碑 5：集成测试与平衡（预计 3 天）
- [ ] 分梯度强度测试
- [ ] Boss 战平衡调整
- [ ] 闪避手感微调
- [ ] 性能优化

## 7. 风险与应对

### 7.1 技术风险
1. **AI 性能**：复杂 AI 行为可能导致性能下降
   - **应对**：使用状态缓存，优化距离计算，限制同时活跃的复杂 AI 数量

2. **碰撞检测复杂度**：新增子弹类型增加碰撞计算
   - **应对**：使用空间分区，优化碰撞检测顺序

3. **状态同步**：Boss 阶段切换与系列锁定的状态管理
   - **应对**：设计清晰的状态机，添加调试日志

### 7.2 平衡风险
1. **难度曲线**：新敌人系统可能破坏现有平衡
   - **应对**：分阶段测试，收集玩家反馈，快速迭代调整

2. **Boss 战长度**：复杂 Boss 可能导致战斗时间过长
   - **应对**：设计阶段检查点，提供逃课机制

## 8. 测试计划

### 8.1 单元测试
- Player 冲刺状态机测试
- 敌人 AI 行为测试
- 子弹碰撞标记测试
- 波次管理器状态测试

### 8.2 集成测试
- 冲刺与无敌帧集成测试
- Boss 战全流程测试
- 系列锁定规则验证
- 动态难度效果测试

### 8.3 平衡测试
- 各梯度怪物组合强度
- Boss 战难度曲线
- 闪避冷却时间手感
- 后期游戏可持续性

## 9. 文档与交付

### 9.1 技术文档
- [ ] 新增类接口文档
- [ ] AI 行为状态图
- [ ] 数据配置格式说明
- [ ] 集成测试用例

### 9.2 配置文档
- [ ] 敌人数值配置表
- [ ] 波次梯度配置
- [ ] Boss 参数配置
- [ ] 平衡调整指南

## 10. 总结

本开发计划提供了从现有代码基础到优化系统的完整实施路径。通过分阶段实施，确保：
1. **功能完整性**：玩家闪避与敌人系统两大核心功能
2. **代码可维护性**：基于现有架构扩展，避免破坏性修改
3. **平衡可控性**：分梯度测试，快速迭代调整
4. **技术可行性**：基于 Qt/C++ 现有能力，充分利用现有框架

**预计总开发时间**：14 个工作日（约 3 周）

**关键成功因素**：
1. 保持与现有系统的兼容性
2. 分阶段测试与快速迭代
3. 清晰的数值配置与平衡调整流程
4. 完善的测试覆盖与性能监控

---

*最后更新：2026年5月7日*
*版本：1.0*