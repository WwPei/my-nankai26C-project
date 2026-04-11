#include "weapon.h"

Weapon::Weapon(const WeaponConfig &config, QObject *parent)
    : QObject(parent)
    , m_name(config.name)
    , m_damageBonus(config.damageBonus)
    , m_attackSpeed(config.attackSpeed)
    , m_projectileSpeed(config.projectileSpeed)
    , m_color(config.color)
{
}
