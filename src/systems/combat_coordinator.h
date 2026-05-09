#pragma once

#include <QMap>
#include <QObject>
#include <QPointF>
#include <QVector>

#include "game_data.h"
#include "game_factory.h"

class QGraphicsEllipseItem;
class QGraphicsScene;
class Player;
class WaveManager;
class Weapon;

class CombatCoordinator : public QObject
{
    Q_OBJECT

public:
    explicit CombatCoordinator(GameFactory *factory, QGraphicsScene *scene,
                               Player *player, Weapon *weapon,
                               QObject *parent = nullptr);
    ~CombatCoordinator() override = default;

    void handleWeaponFireRequested(WeaponId weaponId, const QPointF &origin,
                                   const QPointF &direction,
                                   QVector<GameFactory::BulletEntity> &bullets);
    void setActiveBulletStyle(BulletStyle style);
    [[nodiscard]] BulletStyle activeBulletStyle() const noexcept;
    void resolveCombatCollisions(QVector<GameFactory::BulletEntity> &bullets,
                                 QVector<GameFactory::EnemyEntity> &enemies,
                                 QGraphicsEllipseItem *playerMarker,
                                 float &playerDamageCooldownRemainingMs,
                                 const QMap<TraitId, int> &traitCounts);
    void cleanupExpiredBullets(QVector<GameFactory::BulletEntity> &bullets);
    void cleanupDefeatedEnemies(QVector<GameFactory::EnemyEntity> &enemies,
                                WaveManager *wm,
                                const QMap<TraitId, int> &traitCounts);

private:
    GameFactory *m_factory;
    QGraphicsScene *m_scene;
    Player *m_player;
    Weapon *m_weapon;
    BulletStyle m_activeBulletStyle{BulletStyle::Normal};
};
