// FILE_LOCK: @qt6-logic-developer-emojidungeon
// 负责: BasicWeapon 类实现（从 game_factory.cpp 匿名命名空间提取）
// 最后修改: 2026-05-07

#include "basic_weapon.h"

#include "player.h"
#include "combat_utils.h"

#include <algorithm>
#include <cmath>

// ---- 构造与查询 ----

BasicWeapon::BasicWeapon(const WeaponConfig *config, QObject *parent)
    : Weapon(parent)
    , m_config(config)
{
}

WeaponId BasicWeapon::id() const
{
    return m_config != nullptr ? m_config->id : WeaponId::PeaShooter;
}

const WeaponConfig *BasicWeapon::config() const
{
    return m_config;
}

Player *BasicWeapon::owner() const
{
    return m_owner;
}

bool BasicWeapon::isFiring() const
{
    return m_isFiring;
}

// ---- 武器控制 ----

void BasicWeapon::bindOwner(Player *owner)
{
    m_owner = owner;
}

void BasicWeapon::setAimDirection(const QPointF &direction)
{
    m_aimDirection = normalizedDirection(direction);
}

void BasicWeapon::startFiring()
{
    m_isFiring = true;
    m_remainingCooldownMs = 0.0F;
    emit cooldownChanged(currentIntervalMs());
}

void BasicWeapon::stopFiring()
{
    m_isFiring = false;
}

// ---- 冷却推进与 combo 计数 ----

void BasicWeapon::advanceCooldown(float deltaMs)
{
    if (!m_isFiring || m_owner == nullptr || m_config == nullptr) {
        return;
    }

    m_remainingCooldownMs -= deltaMs;
    while (m_remainingCooldownMs <= 0.0F) {
        ++m_comboCounterValue;
        emit fireRequested(id(), m_owner->worldPosition(), m_aimDirection);
        m_remainingCooldownMs += currentIntervalMs();
    }
}

// ---- 特性应用 ----

void BasicWeapon::applyTrait(TraitId traitId)
{
    const auto *config = GameConfig::findTraitConfig(traitId);
    if (config == nullptr) {
        return;
    }

    m_fireRateScale *= std::max(0.1F, config->speedMultiplier);
    emit cooldownChanged(currentIntervalMs());
}

// ---- 武器升级槽位 ----

void BasicWeapon::addExtraProjectiles(int count)
{
    m_extraProjectileCount += count;
}

void BasicWeapon::applyRangeMultiplier(float multiplier)
{
    m_rangeMultiplierValue *= multiplier;
}

void BasicWeapon::applyPierce(int extraPierces)
{
    m_pierceCountValue += extraPierces;
}

void BasicWeapon::applyBulletSizeScale(float scale)
{
    m_bulletSizeScaleValue *= scale;
}

void BasicWeapon::enableCombo(int interval, float damageMultiplier)
{
    if (m_comboIntervalValue == 0) {
        m_comboIntervalValue = interval;
        m_comboDamageMultiplierValue = damageMultiplier;
    }
}

// ---- 升级状态查询 ----

float BasicWeapon::bulletSizeScale() const
{
    return m_bulletSizeScaleValue;
}

int BasicWeapon::pierceCount() const
{
    return m_pierceCountValue;
}

int BasicWeapon::extraProjectiles() const
{
    return m_extraProjectileCount;
}

float BasicWeapon::rangeMultiplier() const
{
    return m_rangeMultiplierValue;
}

int BasicWeapon::comboInterval() const
{
    return m_comboIntervalValue;
}

float BasicWeapon::comboDamageMultiplier() const
{
    return m_comboDamageMultiplierValue;
}

float BasicWeapon::fireRateScale() const
{
    return m_fireRateScale;
}

int BasicWeapon::comboCounter() const
{
    return m_comboCounterValue;
}

// ---- 内部工具 ----

float BasicWeapon::currentIntervalMs() const
{
    if (m_config == nullptr) {
        return 1000.0F;
    }

    return m_config->fireIntervalMs / std::max(0.1F, m_fireRateScale);
}
