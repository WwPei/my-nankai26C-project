#include "basic_trait.h"

#include "player.h"

BasicTrait::BasicTrait(const TraitConfig *config, QObject *parent)
    : Trait(parent)
    , m_config(config)
{
}

TraitId BasicTrait::id() const
{
    return m_config != nullptr ? m_config->id : TraitId::QuickHands;
}

QString BasicTrait::displayName() const
{
    return m_config != nullptr ? m_config->displayName : QString();
}

QString BasicTrait::description() const
{
    return m_config != nullptr ? m_config->summary : QString();
}

void BasicTrait::applyToPlayer(Player *player)
{
    if (player == nullptr || m_config == nullptr) return;
    player->applyTrait(m_config->id);
}
