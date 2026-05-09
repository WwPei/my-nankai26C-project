#include "basic_bullet_data.h"

#include "combat_utils.h"

#include <algorithm>

BasicBulletData::BasicBulletData(WeaponId weaponId,
                                 const BulletConfig *config,
                                 const QPointF &spawnPosition,
                                 const QPointF &direction,
                                 QObject *parent)
    : BulletData(parent)
    , m_weaponId(weaponId)
    , m_config(config)
    , m_worldPosition(spawnPosition)
    , m_direction(normalizedDirection(direction))
{
}

WeaponId BasicBulletData::weaponId() const { return m_weaponId; }
const BulletConfig *BasicBulletData::config() const { return m_config; }
QPointF BasicBulletData::worldPosition() const { return m_worldPosition; }
QPointF BasicBulletData::direction() const { return m_direction; }

float BasicBulletData::damage() const
{
    const float base = m_config != nullptr ? m_config->damage : 0.0F;
    return base * m_damageMultiplierValue * m_comboMultiplierValue;
}

float BasicBulletData::speed() const
{
    return m_speedValue >= 0.0F ? m_speedValue : (m_config != nullptr ? m_config->speed : 0.0F);
}

float BasicBulletData::collisionRadius() const
{
    return (m_config != nullptr ? m_config->collisionRadius : 0.0F) * m_sizeScaleValue;
}

bool BasicBulletData::isExpired() const { return m_expired; }
bool BasicBulletData::isCritical() const { return m_isCritical; }
int BasicBulletData::remainingPierceCount() const { return m_pierceCount; }
float BasicBulletData::sizeScale() const { return m_sizeScaleValue; }
float BasicBulletData::comboMultiplier() const { return m_comboMultiplierValue; }
bool BasicBulletData::isEnemyBullet() const { return m_isEnemyBullet; }

void BasicBulletData::setWorldPosition(const QPointF &position)
{
    if (m_worldPosition == position) return;
    m_worldPosition = position;
    emit positionChanged(m_worldPosition);
}

void BasicBulletData::setDirection(const QPointF &direction)
{
    m_direction = normalizedDirection(direction, m_direction);
}

void BasicBulletData::advanceFrame(float deltaSeconds)
{
    if (m_expired || deltaSeconds <= 0.0F) return;
    setWorldPosition(m_worldPosition + m_direction * (speed() * deltaSeconds));
}

void BasicBulletData::expire()
{
    if (m_expired) return;
    m_expired = true;
    emit expired();
}

void BasicBulletData::setCritical(bool critical) { m_isCritical = critical; }
void BasicBulletData::setPierceCount(int count) { m_pierceCount = count; }
void BasicBulletData::setSizeScale(float scale) { m_sizeScaleValue = scale; }
void BasicBulletData::setComboMultiplier(float multiplier) { m_comboMultiplierValue = multiplier; }
void BasicBulletData::setDamageMultiplier(float multiplier) { m_damageMultiplierValue = multiplier; }
void BasicBulletData::setEnemyBullet(bool enemyBullet) { m_isEnemyBullet = enemyBullet; }
void BasicBulletData::setSpeed(float speed) { m_speedValue = speed; }
