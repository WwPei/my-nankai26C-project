// FILE_LOCK: @qt6-logic-developer-emojidungeon
// 负责: BasicEnemyData 类声明 - 敌人数据的具体实现
// 最后修改: 2026-05-07

#pragma once

#include <QObject>
#include <QPointF>

#include "enemy_data.h"

/// @brief 敌人数据的具体实现类，封装所有敌人的运行时状态与 AI 行为。
/// @details 继承自 EnemyData 纯虚接口，每个实例绑定一个 EnemyConfig 配置。
///          支持 Chase、KeepDistance、Charge、ShootAndMove、Boss 五种行为模式。
///          Boss 敌人支持多阶段切换，Charge 敌人支持蓄力冲刺机制。
class BasicEnemyData final : public EnemyData
{
    Q_OBJECT

public:
    /// @brief 构造敌人数据，绑定配置与出生点。
    /// @param config 敌人静态配置（血量/速度/行为等），可为 nullptr（此时使用安全默认值）。
    /// @param spawnPosition 世界出生坐标。
    /// @param parent Qt 父对象，用于生命周期管理。
    explicit BasicEnemyData(const EnemyConfig *config, const QPointF &spawnPosition, QObject *parent = nullptr);

    // ---- 属性查询接口（const） ----
    [[nodiscard]] EnemyId id() const override;
    [[nodiscard]] const EnemyConfig *config() const override;
    [[nodiscard]] QPointF worldPosition() const override;
    [[nodiscard]] float currentHealth() const override;
    [[nodiscard]] float maxHealth() const override;
    [[nodiscard]] float moveSpeed() const override;
    [[nodiscard]] float contactDamage() const override;
    [[nodiscard]] float collisionRadius() const override;
    [[nodiscard]] bool isDefeated() const override;

    // ---- 减速效果查询 ----
    [[nodiscard]] float slowFactor() const override;
    [[nodiscard]] float slowRemainingSeconds() const override;

    // ---- AI 相关状态查询 ----
    [[nodiscard]] EnemyBehavior currentBehavior() const override;
    [[nodiscard]] int currentBossPhase() const override;
    [[nodiscard]] float chargeCooldownRemaining() const override;
    [[nodiscard]] float chargeProgress() const override;
    [[nodiscard]] float shootCooldownRemaining() const override;
    [[nodiscard]] QString imagePath() const override;

public slots:
    /// @brief 设置世界坐标，位置变化时自动发射 positionChanged 信号。
    void setWorldPosition(const QPointF &position) override;

    /// @brief 受到伤害处理，触发 damageReceived / healthChanged，血量归零时触发 defeated。
    void receiveDamage(float amount, DamageVisualType damageType) override;

    /// @brief 设置 AI 导航目标点（Chase/Charge 行为下由 updateAI 调用）。
    void setTargetPosition(const QPointF &position) override;

    /// @brief 每帧推进逻辑：减速衰减、蓄力计时、向目标点移动。
    void advanceFrame(float deltaSeconds) override;

    /// @brief 施加减速效果。
    /// @param factor 速度系数（0.0~1.0，越小越慢）。
    /// @param durationSeconds 持续时间（秒）。
    void applySlow(float factor, float durationSeconds) override;

    /// @brief Boss 进入指定阶段（仅 Boss 行为有效）。
    void enterBossPhase(int phase) override;

    /// @brief 每帧 AI 决策：根据行为模式更新目标位置、发射子弹、发起冲刺。
    /// @param deltaSeconds 帧间隔（秒）。
    /// @param playerPosition 玩家当前世界坐标。
    void updateAI(float deltaSeconds, QPointF playerPosition) override;

private:
    const EnemyConfig *m_config{nullptr};
    QPointF m_worldPosition;
    QPointF m_targetPosition;
    float m_currentHealth{0.0F};
    bool m_defeated{false};
    float m_slowFactorValue{1.0F};
    float m_slowRemainingSecondsValue{0.0F};
    float m_moveSpeedValue{0.0F};
    EnemyBehavior m_behavior{EnemyBehavior::Chase};
    int m_bossPhase{0};
    float m_chargeUpRemaining{0.0F};
    float m_chargeDurationRemaining{0.0F};
    float m_chargeCooldownRemainingValue{0.0F};
    float m_shootCooldownRemainingValue{0.0F};
    float m_minionSpawnCooldownValue{0.0F};
    bool m_isCharging{false};
};
