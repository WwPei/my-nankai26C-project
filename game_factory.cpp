#include "game_factory.h"
#include "enemy.h"
#include "bullet.h"
#include <QGraphicsScene>

GameFactory::GameFactory(QGraphicsScene *scene, QObject *parent)
    : QObject(parent), m_scene(scene) {}

// --- Enemy ---
EnemyData* GameFactory::createEnemyData(int typeId)
{
    return new EnemyData(typeId, this); // Factory 作为 parent 管理内存，或者由 GameMainPage 管理
}

EnemyView* GameFactory::createEnemyView(EnemyData *data)
{
    EnemyView *view = new EnemyView(data);
    if (m_scene) m_scene->addItem(view);
    return view;
}

// --- Bullet ---
BulletData* GameFactory::createBulletData(const QPointF &spawnPos, const QPointF &dir, int baseDamage, Weapon *weapon)
{
    BulletData *data = new BulletData(this);
    int totalDmg = baseDamage + (weapon ? weapon->damageBonus() : 0);
    double speed = (weapon ? weapon->projectileSpeed() : 8.0);

    data->init(spawnPos, dir, speed, totalDmg);
    return data;
}

BulletView* GameFactory::createBulletView(BulletData *data)
{
    BulletView *view = new BulletView(data);
    if (m_scene) m_scene->addItem(view);
    return view;
}

// --- Weapon ---
Weapon* GameFactory::createDefaultWeapon()
{
    WeaponConfig cfg;
    cfg.name = "拳头";
    cfg.damageBonus = 0;
    cfg.attackSpeed = 1.0;
    cfg.projectileSpeed = 8.0;
    cfg.color = Qt::gray;
    return new Weapon(cfg, this);
}
