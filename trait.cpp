#include "trait.h"
#include "player.h"  // 假设 PlayerItem 未来会增加血量操作接口

Trait::Trait(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
{}

Trait::~Trait() = default;

void Trait::onEquip(PlayerItem *player)
{
    Q_UNUSED(player);
}

void Trait::onUnequip(PlayerItem *player)
{
    Q_UNUSED(player);
}

void Trait::update(PlayerItem *player, float deltaTime)
{
    Q_UNUSED(player);
    Q_UNUSED(deltaTime);
}

// ----- TraitMaxHealth -----
TraitMaxHealth::TraitMaxHealth(int bonusHp)
    : Trait("最大生命 +" + QString::number(bonusHp))
    , m_bonusHp(bonusHp)
{}

void TraitMaxHealth::onEquip(PlayerItem *player)
{
    // 实际项目中需要在 PlayerItem 中添加增加最大血量的方法
    // 这里仅示意
    Q_UNUSED(player);
    // player->addMaxHealth(m_bonusHp);
}

void TraitMaxHealth::onUnequip(PlayerItem *player)
{
    Q_UNUSED(player);
    // player->addMaxHealth(-m_bonusHp);
}

// ----- TraitLifeSteal -----
TraitLifeSteal::TraitLifeSteal(float percent)
    : Trait("吸血 " + QString::number(static_cast<int>(percent * 100)) + "%")
    , m_percent(percent)
{}

void TraitLifeSteal::onDamageDealt(PlayerItem *player, int damage)
{
    Q_UNUSED(player);
    Q_UNUSED(damage);
    // int heal = static_cast<int>(damage * m_percent);
    // player->heal(heal);
}
