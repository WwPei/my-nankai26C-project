#pragma once

#include <QMap>
#include <QObject>
#include <QString>

#include "game_data.h"

class GameFactory;
class Player;
class Weapon;

class UpgradeResolver : public QObject
{
    Q_OBJECT

public:
    explicit UpgradeResolver(Player *player, Weapon *weapon, GameFactory *factory,
                             QObject *parent = nullptr);
    ~UpgradeResolver() override = default;

    void applyTrait(TraitId traitId);
    void applyWeaponUpgrade(WeaponUpgradeId upgradeId);
    void setClassId(PlayerClassId classId);
    [[nodiscard]] QString lastUpgradeSummary() const;
    [[nodiscard]] const QMap<TraitId, int> &traitCounts() const;
    [[nodiscard]] const QList<TraitId> &ownedTraits() const;
    void clearTraits();

signals:
    void statsChanged();

private:
    Player *m_player;
    Weapon *m_weapon;
    GameFactory *m_factory;
    PlayerClassId m_classId{PlayerClassId::Warrior};
    QMap<TraitId, int> m_traitCounts;
    QList<TraitId> m_ownedTraits;
    QString m_lastUpgradeSummary;
};
