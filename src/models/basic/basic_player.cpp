// FILE_LOCK: @qt6-logic-developer-emojidungeon
// 负责: BasicPlayer 类实现 - 从 game_factory.cpp 匿名命名空间提取
// 最后修改: 2026-05-09

#include "basic_player.h"

#include "combat_utils.h"

#include <algorithm>
#include <cmath>

BasicPlayer::BasicPlayer(const PlayerClassConfig *config, QObject *parent)
    : Player(parent)
    , m_config(config)
    , m_currentHealth(config != nullptr ? config->maxHealth : 0.0F)
    , m_weaponId(config != nullptr ? config->starterWeaponId : WeaponId::PeaShooter)
{
}

PlayerClassId BasicPlayer::classId() const
{
    return m_config != nullptr ? m_config->id : PlayerClassId::Warrior;
}

QPointF BasicPlayer::worldPosition() const
{
    return m_worldPosition;
}

float BasicPlayer::currentHealth() const
{
    return m_currentHealth;
}

float BasicPlayer::maxHealth() const
{
    return (m_config != nullptr ? m_config->maxHealth : 0.0F) + m_extraMaxHealth;
}

float BasicPlayer::moveSpeed() const
{
    return m_config != nullptr ? m_config->moveSpeed * m_speedMultiplier : 0.0F;
}

WeaponId BasicPlayer::weaponId() const
{
    return m_weaponId;
}

float BasicPlayer::damageMultiplier() const
{
    return m_damageMultiplier;
}

float BasicPlayer::defenseMultiplier() const
{
    return m_defenseMultiplier;
}

float BasicPlayer::speedMultiplier() const
{
    return m_speedMultiplier;
}

float BasicPlayer::expMultiplier() const
{
    return m_expMultiplier;
}

QMap<TraitId, int> BasicPlayer::traitLevels() const
{
    return m_traitLevels;
}

void BasicPlayer::setWorldPosition(const QPointF &position)
{
    if (m_worldPosition == position) {
        return;
    }

    m_worldPosition = position;
    emit moved(m_worldPosition);
}

void BasicPlayer::setMoveDirection(const QPointF &direction)
{
    m_moveDirection = direction;
}

void BasicPlayer::equipWeapon(WeaponId weaponId)
{
    if (m_weaponId == weaponId) {
        return;
    }

    m_weaponId = weaponId;
    emit weaponChanged(m_weaponId);
}

void BasicPlayer::applyTrait(TraitId traitId)
{
    const auto *config = GameConfig::findTraitConfig(traitId);
    if (config == nullptr) {
        return;
    }

    int &level = m_traitLevels[traitId];
    ++level;
    const float levelMultiplier = 1.0F + (level - 1) * 0.5F;

    m_damageMultiplier *= (1.0F + (config->damageMultiplier - 1.0F) * levelMultiplier);
    m_defenseMultiplier *= (1.0F + (config->defenseMultiplier - 1.0F) * levelMultiplier);
    m_speedMultiplier *= (1.0F + (config->speedMultiplier - 1.0F) * levelMultiplier);
    m_expMultiplier *= (1.0F + (config->expMultiplier - 1.0F) * levelMultiplier);
    m_extraMaxHealth += config->extraMaxHealth * levelMultiplier;

    m_currentHealth = std::min(maxHealth(), m_currentHealth + config->extraMaxHealth * levelMultiplier);
    emit healthChanged(m_currentHealth, maxHealth());
}

void BasicPlayer::applyWeaponUpgrade(WeaponUpgradeId upgradeId)
{
    Q_UNUSED(upgradeId);
}

void BasicPlayer::heal(float amount)
{
    if (amount <= 0.0F || m_currentHealth <= 0.0F) {
        return;
    }
    m_currentHealth = std::min(maxHealth(), m_currentHealth + amount);
    emit healthChanged(m_currentHealth, maxHealth());
}

void BasicPlayer::applySpeedBuff(float multiplier, float durationSeconds)
{
    Q_UNUSED(multiplier);
    Q_UNUSED(durationSeconds);
}

void BasicPlayer::receiveDamage(float amount)
{
    if (amount <= 0.0F || m_currentHealth <= 0.0F) {
        return;
    }

    const float actualDamage = std::max(0.0F, amount / std::max(0.1F, m_defenseMultiplier));
    m_currentHealth = std::max(0.0F, m_currentHealth - actualDamage);
    emit healthChanged(m_currentHealth, maxHealth());

    if (m_currentHealth <= 0.0F) {
        emit defeated();
    }
}

void BasicPlayer::dash()
{
    constexpr float kDashCooldown = 3.0F;
    if (m_isDashing || m_dashCooldownRemaining > 0.0F || m_dashHardStunRemaining > 0.0F) {
        return;
    }
    m_isDashing = true;
    m_dashDurationRemaining = 0.12F;
    m_dashCooldownRemaining = kDashCooldown;
    emit dashCooldownChanged(m_dashCooldownRemaining, kDashCooldown);
    m_dashHardStunRemaining = 0.08F;
}

bool BasicPlayer::isDashing() const
{
    return m_isDashing;
}

float BasicPlayer::dashCooldownRemaining() const
{
    return m_dashCooldownRemaining;
}

QPointF BasicPlayer::dashDirection() const
{
    return m_lastMoveDirection;
}

void BasicPlayer::updateDash(float deltaSeconds)
{
    if (m_dashCooldownRemaining > 0.0F) {
        m_dashCooldownRemaining = std::max(0.0F, m_dashCooldownRemaining - deltaSeconds);
        emit dashCooldownChanged(m_dashCooldownRemaining, dashCooldownTotal());
    }
    if (m_isDashing) {
        m_dashDurationRemaining -= deltaSeconds;
        if (m_dashDurationRemaining <= 0.0F) {
            m_isDashing = false;
            m_dashHardStunRemaining = std::max(0.0F, 0.08F);
        }
    }
    if (m_dashHardStunRemaining > 0.0F && !m_isDashing) {
        m_dashHardStunRemaining = std::max(0.0F, m_dashHardStunRemaining - deltaSeconds);
    }
}

void BasicPlayer::updateDashState(float deltaSeconds, const QPointF &moveDirection)
{
    if (moveDirection != QPointF(0, 0)) {
        m_lastMoveDirection = moveDirection;
    }
    updateDash(deltaSeconds);
}

float BasicPlayer::dashCooldownTotal() const
{
    constexpr float kDashCooldown = 3.0F;
    return kDashCooldown;
}