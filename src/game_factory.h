#pragma once

#include <QObject>
#include <QPointF>

#include "game_data.h"

class QGraphicsItem;
class Player;
class Weapon;
class EnemyData;
class EnemyView;
class BulletData;
class BulletView;
class Trait;

class GameFactory : public QObject
{
    Q_OBJECT

public:
    struct EnemyEntity {
        EnemyData *data {nullptr};
        EnemyView *view {nullptr};
    };

    struct BulletEntity {
        BulletData *data {nullptr};
        BulletView *view {nullptr};
    };

    explicit GameFactory(QObject *parent = nullptr);
    ~GameFactory() override = default;

    [[nodiscard]] const PlayerClassConfig *playerClassConfig(PlayerClassId id) const noexcept;
    [[nodiscard]] const WeaponConfig *weaponConfig(WeaponId id) const noexcept;
    [[nodiscard]] const BulletConfig *bulletConfig(WeaponId weaponId) const noexcept;
    [[nodiscard]] const EnemyConfig *enemyConfig(EnemyId id) const noexcept;
    [[nodiscard]] const TraitConfig *traitConfig(TraitId id) const noexcept;

    [[nodiscard]] Player *createPlayer(PlayerClassId classId, QObject *parent = nullptr) const;
    [[nodiscard]] Weapon *createStarterWeapon(PlayerClassId classId, Player *owner, QObject *parent = nullptr) const;
    [[nodiscard]] EnemyData *createEnemyData(EnemyId enemyId,
                                             const QPointF &spawnPosition,
                                             QObject *parent = nullptr) const;
    [[nodiscard]] EnemyView *createEnemyView(EnemyData *data, QGraphicsItem *parent = nullptr) const;
    [[nodiscard]] EnemyEntity createEnemyEntity(EnemyId enemyId,
                                                const QPointF &spawnPosition,
                                                QObject *dataParent = nullptr,
                                                QGraphicsItem *viewParent = nullptr) const;
    [[nodiscard]] BulletData *createBulletData(WeaponId weaponId,
                                               const QPointF &spawnPosition,
                                               const QPointF &direction,
                                               QObject *parent = nullptr) const;
    [[nodiscard]] BulletView *createBulletView(BulletData *data, QGraphicsItem *parent = nullptr) const;
    [[nodiscard]] BulletEntity createBulletEntity(WeaponId weaponId,
                                                  const QPointF &spawnPosition,
                                                  const QPointF &direction,
                                                  QObject *dataParent = nullptr,
                                                  QGraphicsItem *viewParent = nullptr) const;
    [[nodiscard]] Trait *createTrait(TraitId traitId, QObject *parent = nullptr) const;
};
