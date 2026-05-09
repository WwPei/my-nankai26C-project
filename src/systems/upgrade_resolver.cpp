#include "upgrade_resolver.h"

#include "game_factory.h"
#include "player.h"
#include "weapon.h"

#include <utility>

namespace {

[[nodiscard]] QString compareStatLine(const QString &label,
                                      float beforeValue,
                                      float afterValue,
                                      int decimals = 1,
                                      const QString &suffix = QString())
{
    return QStringLiteral("%1：%2%5 -> %3%5（%4%5）")
        .arg(label,
             QString::number(beforeValue, 'f', decimals),
             QString::number(afterValue, 'f', decimals),
             QString::number(afterValue - beforeValue, 'f', decimals),
             suffix);
}

} // namespace

UpgradeResolver::UpgradeResolver(Player *player, Weapon *weapon, GameFactory *factory,
                                 QObject *parent)
    : QObject(parent)
    , m_player(player)
    , m_weapon(weapon)
    , m_factory(factory)
{
}

void UpgradeResolver::setClassId(PlayerClassId classId)
{
    m_classId = classId;
}

void UpgradeResolver::clearTraits()
{
    m_traitCounts.clear();
    m_ownedTraits.clear();
}

const QMap<TraitId, int> &UpgradeResolver::traitCounts() const
{
    return m_traitCounts;
}

const QList<TraitId> &UpgradeResolver::ownedTraits() const
{
    return m_ownedTraits;
}

QString UpgradeResolver::lastUpgradeSummary() const
{
    return m_lastUpgradeSummary;
}

void UpgradeResolver::applyTrait(TraitId traitId)
{
    const int previousLevel = m_traitCounts.value(traitId, 0);
    const int newLevel = previousLevel + 1;
    m_traitCounts[traitId] = newLevel;

    if (previousLevel == 0) {
        m_ownedTraits.push_back(traitId);
    }

    float beforeDamageMultiplier = 1.0F;
    float beforeDefenseMultiplier = 1.0F;
    float beforeSpeedMultiplier = 1.0F;
    for (const TraitId ownedTraitId : std::as_const(m_ownedTraits)) {
        const TraitConfig *ownedTraitConfig = GameConfig::findTraitConfig(ownedTraitId);
        if (ownedTraitConfig == nullptr) {
            continue;
        }
        beforeDamageMultiplier *= ownedTraitConfig->damageMultiplier;
        beforeDefenseMultiplier *= ownedTraitConfig->defenseMultiplier;
        beforeSpeedMultiplier *= ownedTraitConfig->speedMultiplier;
    }

    const WeaponConfig *weaponConfig = (m_factory != nullptr && m_player != nullptr)
        ? m_factory->weaponConfig(m_player->weaponId())
        : nullptr;
    const PlayerClassConfig *classConfig = (m_factory != nullptr)
        ? m_factory->playerClassConfig(m_classId)
        : GameConfig::findPlayerClassConfig(m_classId);
    const float beforeDamage = weaponConfig != nullptr ? weaponConfig->baseDamage * beforeDamageMultiplier : 0.0F;
    const float beforeAttackSpeed = weaponConfig != nullptr && weaponConfig->fireIntervalMs > 0.0F
        ? (1000.0F / weaponConfig->fireIntervalMs) * beforeSpeedMultiplier
        : 0.0F;
    const float beforeMoveSpeed = classConfig != nullptr
        ? classConfig->moveSpeed * beforeSpeedMultiplier
        : 0.0F;

    if (m_player != nullptr) {
        m_player->applyTrait(traitId);
    }
    if (m_weapon != nullptr) {
        m_weapon->applyTrait(traitId);
    }

    const TraitConfig *newTraitConfig = GameConfig::findTraitConfig(traitId);
    const float afterDamageMultiplier = beforeDamageMultiplier
        * (newTraitConfig != nullptr ? newTraitConfig->damageMultiplier : 1.0F);
    const float afterSpeedMultiplier = beforeSpeedMultiplier
        * (newTraitConfig != nullptr ? newTraitConfig->speedMultiplier : 1.0F);
    const float afterDamage = weaponConfig != nullptr
        ? weaponConfig->baseDamage * afterDamageMultiplier
        : beforeDamage;
    const float afterAttackSpeed = weaponConfig != nullptr && weaponConfig->fireIntervalMs > 0.0F
        ? (1000.0F / weaponConfig->fireIntervalMs) * afterSpeedMultiplier
        : beforeAttackSpeed;
    const float afterMoveSpeed = classConfig != nullptr
        ? classConfig->moveSpeed * afterSpeedMultiplier
        : beforeMoveSpeed;
    const float afterDefenseMultiplier = beforeDefenseMultiplier
        * (newTraitConfig != nullptr ? newTraitConfig->defenseMultiplier : 1.0F);
    m_lastUpgradeSummary = QStringLiteral(
        "<b>特性：</b><span style='color:#82afff;'>%1 Lv.%2</span><br/>"
        "%3<br/>"
        "%4<br/>"
        "%5<br/>"
        "%6")
                               .arg(newTraitConfig != nullptr ? newTraitConfig->displayName : QStringLiteral("未知特性"),
                                    QString::number(newLevel),
                                    compareStatLine(QStringLiteral("攻击"), beforeDamage, afterDamage),
                                    compareStatLine(QStringLiteral("攻速"), beforeAttackSpeed, afterAttackSpeed, 2),
                                    compareStatLine(QStringLiteral("移速"), beforeMoveSpeed, afterMoveSpeed, 0),
                                    compareStatLine(QStringLiteral("防御"), beforeDefenseMultiplier, afterDefenseMultiplier, 2, QStringLiteral("x")));

    emit statsChanged();
}

void UpgradeResolver::applyWeaponUpgrade(WeaponUpgradeId upgradeId)
{
    if (m_weapon == nullptr) {
        return;
    }

    const auto *config = GameConfig::findWeaponUpgradeConfig(upgradeId);
    if (config == nullptr) {
        return;
    }

    if (config->extraProjectiles > 0.0F) {
        m_weapon->addExtraProjectiles(static_cast<int>(config->extraProjectiles));
    }
    if (config->rangeMultiplier > 1.0F) {
        m_weapon->applyRangeMultiplier(config->rangeMultiplier);
    }
    if (config->pierceCount > 0) {
        m_weapon->applyPierce(config->pierceCount);
    }
    if (config->bulletSizeScale > 1.0F) {
        m_weapon->applyBulletSizeScale(config->bulletSizeScale);
    }
    if (config->comboInterval > 0) {
        m_weapon->enableCombo(config->comboInterval, config->comboDamageMultiplier);
    }

    m_lastUpgradeSummary = QStringLiteral(
        "<b>武器升级：</b><span style='color:#f0c24b;'>%1</span><br/>"
        "<span style='color:#93a0b4;'>%2</span>")
                               .arg(config->displayName, config->summary);

    emit statsChanged();
}
