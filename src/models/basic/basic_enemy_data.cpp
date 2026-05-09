// FILE_LOCK: @qt6-logic-developer-emojidungeon
// 负责: BasicEnemyData 类实现 - 敌人运行时状态与 AI 逻辑
// 最后修改: 2026-05-07

#include "basic_enemy_data.h"

#include <QLineF>

#include <algorithm>
#include <cmath>

// ============================================================================
// 构造 / 析构
// ============================================================================

BasicEnemyData::BasicEnemyData(const EnemyConfig *config, const QPointF &spawnPosition, QObject *parent)
    : EnemyData(parent)
    , m_config(config)
    , m_worldPosition(spawnPosition)
    , m_targetPosition(spawnPosition)
    , m_currentHealth(config != nullptr ? config->maxHealth : 0.0F)
    , m_behavior(config != nullptr ? config->behavior : EnemyBehavior::Chase)
{
}

// ============================================================================
// 属性查询（const）
// ============================================================================

EnemyId BasicEnemyData::id() const
{
    return m_config != nullptr ? m_config->id : EnemyId::Ogre;
}

const EnemyConfig *BasicEnemyData::config() const
{
    return m_config;
}

QPointF BasicEnemyData::worldPosition() const
{
    return m_worldPosition;
}

float BasicEnemyData::currentHealth() const
{
    return m_currentHealth;
}

float BasicEnemyData::maxHealth() const
{
    return m_config != nullptr ? m_config->maxHealth : 0.0F;
}

float BasicEnemyData::moveSpeed() const
{
    const float baseSpeed = m_config != nullptr ? m_config->moveSpeed : 0.0F;
    if (m_moveSpeedValue > 0.0F) {
        return m_moveSpeedValue;
    }
    return baseSpeed * m_slowFactorValue;
}

float BasicEnemyData::contactDamage() const
{
    return m_config != nullptr ? m_config->contactDamage : 0.0F;
}

float BasicEnemyData::collisionRadius() const
{
    return m_config != nullptr ? m_config->collisionRadius : 0.0F;
}

bool BasicEnemyData::isDefeated() const
{
    return m_defeated;
}

// ============================================================================
// 减速效果查询
// ============================================================================

float BasicEnemyData::slowFactor() const
{
    return m_slowFactorValue;
}

float BasicEnemyData::slowRemainingSeconds() const
{
    return m_slowRemainingSecondsValue;
}

// ============================================================================
// AI 相关状态查询
// ============================================================================

EnemyBehavior BasicEnemyData::currentBehavior() const
{
    return m_behavior;
}

int BasicEnemyData::currentBossPhase() const
{
    return m_bossPhase;
}

float BasicEnemyData::chargeCooldownRemaining() const
{
    return m_chargeCooldownRemainingValue;
}

float BasicEnemyData::chargeProgress() const
{
    if (!m_isCharging && m_chargeUpRemaining <= 0.0F) return 0.0F;
    return 1.0F;
}

float BasicEnemyData::shootCooldownRemaining() const
{
    return m_shootCooldownRemainingValue;
}

QString BasicEnemyData::imagePath() const
{
    return m_config != nullptr ? m_config->imagePath : QString();
}

// ============================================================================
// Slots: 核心逻辑
// ============================================================================

void BasicEnemyData::setWorldPosition(const QPointF &position)
{
    if (m_worldPosition == position) {
        return;
    }

    m_worldPosition = position;
    emit positionChanged(m_worldPosition);
}

void BasicEnemyData::receiveDamage(float amount, DamageVisualType damageType)
{
    if (amount <= 0.0F || m_defeated) {
        return;
    }

    m_currentHealth = std::max(0.0F, m_currentHealth - amount);
    emit damageReceived(damageType, amount);
    emit healthChanged(m_currentHealth, maxHealth());
    if (m_currentHealth <= 0.0F) {
        m_defeated = true;
        emit defeated();
    }
}

void BasicEnemyData::setTargetPosition(const QPointF &position)
{
    m_targetPosition = position;
}

void BasicEnemyData::advanceFrame(float deltaSeconds)
{
    if (m_defeated || deltaSeconds <= 0.0F) {
        return;
    }

    // ---- 减速效果衰减 ----
    if (m_slowRemainingSecondsValue > 0.0F) {
        m_slowRemainingSecondsValue = std::max(0.0F, m_slowRemainingSecondsValue - deltaSeconds);
        if (m_slowRemainingSecondsValue <= 0.0F) {
            m_slowFactorValue = 1.0F;
        }
    }

    // ---- 蓄力冲刺逻辑 ----
    if (m_isCharging) {
        if (m_chargeUpRemaining > 0.0F) {
            m_chargeUpRemaining = std::max(0.0F, m_chargeUpRemaining - deltaSeconds);
            return; // 蓄力期间不移动
        }
        if (m_chargeDurationRemaining <= 0.0F) {
            m_chargeDurationRemaining = m_config->chargeDurationMs / 1000.0F;
        }
        m_chargeDurationRemaining = std::max(0.0F, m_chargeDurationRemaining - deltaSeconds);
        m_moveSpeedValue = m_config->moveSpeed * m_config->chargeSpeedMult * m_slowFactorValue;
        if (m_chargeDurationRemaining <= 0.0F) {
            m_isCharging = false;
            m_chargeCooldownRemainingValue = m_config->chargeCooldownMs / 1000.0F;
            m_moveSpeedValue = m_config->moveSpeed * m_slowFactorValue;
        }
    }

    // ---- 向目标点移动 ----
    const QPointF delta = m_targetPosition - m_worldPosition;
    const QLineF distanceLine(QPointF(), delta);
    if (distanceLine.length() <= 0.001) {
        return;
    }

    const float step = moveSpeed() * deltaSeconds;
    if (step >= distanceLine.length()) {
        setWorldPosition(m_targetPosition);
        return;
    }

    const QPointF direction(delta.x() / distanceLine.length(), delta.y() / distanceLine.length());
    setWorldPosition(m_worldPosition + direction * step);

    // ---- ShootAndMove 距离保持覆盖 ----
    // 当行为为 ShootAndMove 且设定了 keepDistanceMin 时，
    // 若与目标距离过近（<85% keepDistanceMin），强制朝远离目标的方向移动
    if (m_config->behavior == EnemyBehavior::ShootAndMove && m_config->keepDistanceMin > 0.0F) {
        const QLineF distLine(m_worldPosition, m_targetPosition);
        if (distLine.length() < m_config->keepDistanceMin * 0.85F) {
            // Move away：从目标位置指向当前位置的方向即为"远离"方向
            const QPointF awayDir = QPointF(m_worldPosition.x() - m_targetPosition.x(),
                                             m_worldPosition.y() - m_targetPosition.y())
                                     / std::max(0.001F, static_cast<float>(distLine.length()));
            setWorldPosition(m_worldPosition + awayDir * (moveSpeed() * deltaSeconds));
        }
    }
}

void BasicEnemyData::applySlow(float factor, float durationSeconds)
{
    m_slowFactorValue = factor;
    m_slowRemainingSecondsValue = durationSeconds;
}

void BasicEnemyData::enterBossPhase(int phase)
{
    if (m_behavior != EnemyBehavior::Boss) return;
    m_bossPhase = phase;
    if (phase == 2) {
        m_moveSpeedValue = m_config->moveSpeed * 1.5F;
        m_shootCooldownRemainingValue = 0.0F;
    }
}

void BasicEnemyData::updateAI(float deltaSeconds, QPointF playerPosition)
{
    // 冷却递减
    if (m_chargeCooldownRemainingValue > 0.0F) {
        m_chargeCooldownRemainingValue = std::max(0.0F, m_chargeCooldownRemainingValue - deltaSeconds);
    }
    if (m_shootCooldownRemainingValue > 0.0F) {
        m_shootCooldownRemainingValue = std::max(0.0F, m_shootCooldownRemainingValue - deltaSeconds);
    }
    if (m_minionSpawnCooldownValue > 0.0F) {
        m_minionSpawnCooldownValue = std::max(0.0F, m_minionSpawnCooldownValue - deltaSeconds);
    }

    switch (m_behavior) {
    case EnemyBehavior::Chase:
        setTargetPosition(playerPosition);
        break;

    case EnemyBehavior::KeepDistance: {
        QPointF toPlayer = playerPosition - m_worldPosition;
        float dist = std::sqrt(toPlayer.x() * toPlayer.x() + toPlayer.y() * toPlayer.y());
        if (dist < m_config->keepDistanceMin && dist > 0.01F) {
            QPointF awayDir = QPointF(-toPlayer.x() / dist, -toPlayer.y() / dist);
            setTargetPosition(m_worldPosition + awayDir * m_config->keepDistanceMin);
        } else {
            setTargetPosition(m_worldPosition);
        }
        break;
    }

    case EnemyBehavior::Charge:
        setTargetPosition(playerPosition);
        if (m_chargeCooldownRemainingValue <= 0.0F && !m_isCharging) {
            m_chargeUpRemaining = m_config->chargeUpMs / 1000.0F;
            m_isCharging = true;
        }
        break;

    case EnemyBehavior::ShootAndMove: {
        QPointF toPlayer = playerPosition - m_worldPosition;
        float dist = std::sqrt(toPlayer.x() * toPlayer.x() + toPlayer.y() * toPlayer.y());

        // 判定是否在射击触发距离内（shootTriggerDistance == 0 表示始终可射击）
        const bool inShootingRange = (m_config->shootTriggerDistance <= 0.0F)
            || (dist <= m_config->shootTriggerDistance);

        if (!inShootingRange) {
            // 超出射击触发距离：仅追击玩家，不射击
            setTargetPosition(playerPosition);
        } else {
            // 进入射击范围：距离管理 + 射击
            if (m_config->keepDistanceMin > 0.0F) {
                if (dist < m_config->keepDistanceMin && dist > 0.01F) {
                    // 太近：远离玩家
                    QPointF awayDir = QPointF(-toPlayer.x() / dist, -toPlayer.y() / dist);
                    setTargetPosition(m_worldPosition + awayDir * m_config->keepDistanceMin);
                } else if (dist > m_config->keepDistanceMin) {
                    // 太远（但在射击距离内）：靠近玩家
                    setTargetPosition(playerPosition);
                } else {
                    // 理想距离：原地不动
                    setTargetPosition(m_worldPosition);
                }
            } else {
                // 无保持距离设定：原地射击
                setTargetPosition(m_worldPosition);
            }
        }

        // 射击冷却判定（仅在射击范围内且方向有效时开火）
        if (m_shootCooldownRemainingValue <= 0.0F && dist > 0.01F && inShootingRange) {
            emit requestShoot(QPointF(toPlayer.x() / dist, toPlayer.y() / dist),
                             m_config->shootBulletCount, m_config->bulletSpeed, m_config->bulletDamage);
            m_shootCooldownRemainingValue = m_config->shootIntervalMs / 1000.0F;
        }
        break;
    }

    case EnemyBehavior::Boss:
        setTargetPosition(playerPosition);
        if (m_shootCooldownRemainingValue <= 0.0F) {
            QPointF toPlayer = playerPosition - m_worldPosition;
            float dist = std::sqrt(toPlayer.x() * toPlayer.x() + toPlayer.y() * toPlayer.y());
            if (dist > 0.01F) {
                emit requestShoot(QPointF(toPlayer.x() / dist, toPlayer.y() / dist),
                                 m_config->shootBulletCount, m_config->bulletSpeed, m_config->bulletDamage);
            }
            m_shootCooldownRemainingValue = m_config->shootIntervalMs / 1000.0F;
        }
        if (m_minionSpawnCooldownValue <= 0.0F) {
            emit requestSpawnMinion();
            m_minionSpawnCooldownValue = 4.0F;
        }
        break;

    default:
        break;
    }
}
