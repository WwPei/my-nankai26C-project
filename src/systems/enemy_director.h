#pragma once

#include <QObject>
#include <QPointF>
#include <QVector>

#include "game_data.h"
#include "game_factory.h"

class QFrame;
class QGraphicsScene;
class QLabel;
class QProgressBar;
class QWidget;
class WaveManager;

class EnemyDirector : public QObject
{
    Q_OBJECT

public:
    explicit EnemyDirector(GameFactory *factory, QGraphicsScene *scene,
                           QWidget *parentWidget,
                           QObject *parent = nullptr);
    ~EnemyDirector() override = default;

    void setStateVectors(QVector<GameFactory::EnemyEntity> *enemies,
                         QVector<GameFactory::BulletEntity> *bullets);

    void spawnTestEnemy(int maxConcurrent, bool bossIsActive, EnemyId currentBossId);
    void spawnBossIfPending(WaveManager *wm, bool &bossIsActive, EnemyId &currentBossId);
    void onBossEntityCreated(GameFactory::EnemyEntity entity);

    void updateBossHealthBar(EnemyData *boss, QLabel *hpLabel, QProgressBar *hpBar);
    void showBossHealthBar(const QString &name, QFrame *&panel,
                           QLabel *&label, QProgressBar *&bar);
    void hideBossHealthBar(QFrame *&panel, QLabel *&label, QProgressBar *&bar);
    void positionBossBarFixed(int x, int y);

    [[nodiscard]] QPointF randomEnemySpawnPosition() const;
    [[nodiscard]] static EnemyId randomEnemyId();

private:
    void connectEnemyShootSignals(EnemyData *enemyData);

    GameFactory *m_factory;
    QGraphicsScene *m_scene;
    QWidget *m_parentWidget;
    QVector<GameFactory::EnemyEntity> *m_enemies{nullptr};
    QVector<GameFactory::BulletEntity> *m_bullets{nullptr};
    QFrame *m_bossHpPanel{nullptr};
    QLabel *m_bossHpLabel{nullptr};
    QProgressBar *m_bossHpBar{nullptr};
};
