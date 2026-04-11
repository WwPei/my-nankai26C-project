#ifndef GAME_FACTORY_H
#define GAME_FACTORY_H

#include <QObject>
#include "game_data.h"
#include "enemy_data.h"
#include "bullet_data.h"
#include "weapon.h"

// 前向声明视图类
class EnemyView;
class BulletView;
class QGraphicsScene;

/**
 * @brief 游戏对象工厂
 * 唯一职责：创建 Data 和 View，并把它们绑定在一起
 */
class GameFactory : public QObject
{
    Q_OBJECT
public:
    explicit GameFactory(QGraphicsScene *scene, QObject *parent = nullptr);

    // 创建敌人
    EnemyData* createEnemyData(int typeId);
    EnemyView* createEnemyView(EnemyData *data);

    // 创建子弹 (这就是从 Weapon 里移出来的逻辑)
    BulletData* createBulletData(const QPointF &spawnPos, const QPointF &dir, int baseDamage, Weapon *weapon);
    BulletView* createBulletView(BulletData *data);

    // 创建默认武器
    Weapon* createDefaultWeapon();

private:
    QGraphicsScene *m_scene;
};

#endif // GAME_FACTORY_H
