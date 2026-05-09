#include "game_factory.h"

#include <QGraphicsItem>
#include <QLineF>
#include <QPainter>
#include <QPixmap>
#include <QPointer>
#include <QTimer>

#include "basic_bullet_data.h"
#include "basic_bullet_view.h"
#include "basic_enemy_data.h"
#include "basic_enemy_view.h"
#include "basic_player.h"
#include "basic_trait.h"
#include "basic_weapon.h"
#include "special_bullet_data.h"
#include "special_bullet_view.h"

GameFactory::GameFactory(QObject *parent)
    : QObject(parent)
{
}

const PlayerClassConfig *GameFactory::playerClassConfig(PlayerClassId id) const noexcept
{
    return GameConfig::findPlayerClassConfig(id);
}

const WeaponConfig *GameFactory::weaponConfig(WeaponId id) const noexcept
{
    return GameConfig::findWeaponConfig(id);
}

const BulletConfig *GameFactory::bulletConfig(WeaponId weaponId) const noexcept
{
    return GameConfig::findBulletConfig(weaponId);
}

const EnemyConfig *GameFactory::enemyConfig(EnemyId id) const noexcept
{
    return GameConfig::findEnemyConfig(id);
}

const TraitConfig *GameFactory::traitConfig(TraitId id) const noexcept
{
    return GameConfig::findTraitConfig(id);
}

Player *GameFactory::createPlayer(PlayerClassId classId, QObject *parent) const
{
    const auto *config = playerClassConfig(classId);
    if (config == nullptr) {
        return nullptr;
    }

    return new BasicPlayer(config, parent);
}

Weapon *GameFactory::createStarterWeapon(PlayerClassId classId, Player *owner, QObject *parent) const
{
    const auto *classConfig = playerClassConfig(classId);
    if (classConfig == nullptr) {
        return nullptr;
    }

    const auto *config = weaponConfig(classConfig->starterWeaponId);
    if (config == nullptr) {
        return nullptr;
    }

    auto *weapon = new BasicWeapon(config, parent);
    weapon->bindOwner(owner);
    if (owner != nullptr) {
        owner->equipWeapon(config->id);
    }
    return weapon;
}

EnemyData *GameFactory::createEnemyData(EnemyId enemyId,
                                        const QPointF &spawnPosition,
                                        QObject *parent) const
{
    const auto *config = enemyConfig(enemyId);
    if (config == nullptr) {
        return nullptr;
    }

    return new BasicEnemyData(config, spawnPosition, parent);
}

EnemyView *GameFactory::createEnemyView(EnemyData *data, QGraphicsItem *parent) const
{
    if (data == nullptr) {
        return nullptr;
    }

    auto *view = new BasicEnemyView(parent);
    view->bindModel(data);
    return view;
}

GameFactory::EnemyEntity GameFactory::createEnemyEntity(EnemyId enemyId,
                                                        const QPointF &spawnPosition,
                                                        QObject *dataParent,
                                                        QGraphicsItem *viewParent) const
{
    EnemyEntity entity;
    entity.data = createEnemyData(enemyId, spawnPosition, dataParent);
    if (entity.data == nullptr) {
        return entity;
    }

    entity.view = createEnemyView(entity.data, viewParent);
    if (entity.view == nullptr) {
        entity.data->deleteLater();
        entity.data = nullptr;
    }

    return entity;
}

BulletData *GameFactory::createBulletData(WeaponId weaponId,
                                          const QPointF &spawnPosition,
                                          const QPointF &direction,
                                          QObject *parent) const
{
    const auto *config = bulletConfig(weaponId);
    if (config == nullptr) {
        return nullptr;
    }

    return new BasicBulletData(weaponId, config, spawnPosition, direction, parent);
}

BulletView *GameFactory::createBulletView(BulletData *data, QGraphicsItem *parent) const
{
    if (data == nullptr) {
        return nullptr;
    }

    auto *view = new BasicBulletView(parent);
    view->bindModel(data);
    return view;
}

BulletData *GameFactory::createSpecialBulletData(const BulletSpecialTemplate &tmpl,
                                                  const QPointF &spawnPosition,
                                                  const QPointF &direction,
                                                  QObject *parent) const
{
    const auto *bulletCfg = bulletConfig(WeaponId::PeaShooter);
    BulletSpecialConfig specialConfig;
    specialConfig.effect = tmpl.effect;
    specialConfig.param1 = tmpl.effectParam1;
    specialConfig.param2 = tmpl.effectParam2;
    specialConfig.emojiIcon = tmpl.emojiText;
    specialConfig.imagePath = tmpl.defaultImagePath;
    specialConfig.altImagePaths = tmpl.altImagePaths;

    return new SpecialBulletData(WeaponId::PeaShooter, bulletCfg, spawnPosition,
                                  direction, specialConfig, parent);
}

BulletView *GameFactory::createSpecialBulletView(BulletData *data, QGraphicsItem *parent) const
{
    if (data == nullptr) {
        return nullptr;
    }

    auto *view = new SpecialBulletView(parent);
    view->bindModel(data);
    return view;
}

GameFactory::BulletEntity GameFactory::createBulletEntity(WeaponId weaponId,
                                                          const QPointF &spawnPosition,
                                                          const QPointF &direction,
                                                          QObject *dataParent,
                                                          QGraphicsItem *viewParent) const
{
    BulletEntity entity;
    entity.data = createBulletData(weaponId, spawnPosition, direction, dataParent);
    if (entity.data == nullptr) {
        return entity;
    }

    entity.view = createBulletView(entity.data, viewParent);
    if (entity.view == nullptr) {
        entity.data->deleteLater();
        entity.data = nullptr;
    }

    return entity;
}

Trait *GameFactory::createTrait(TraitId traitId, QObject *parent) const
{
    const auto *config = traitConfig(traitId);
    if (config == nullptr) {
        return nullptr;
    }

    return new BasicTrait(config, parent);
}
