#pragma once

#include <QObject>
#include <QPointF>

#include "game_data.h"

class EnemyData : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    ~EnemyData() override = default;

    [[nodiscard]] virtual EnemyId id() const = 0;
    [[nodiscard]] virtual const EnemyConfig *config() const = 0;
    [[nodiscard]] virtual QPointF worldPosition() const = 0;
    [[nodiscard]] virtual float currentHealth() const = 0;
    [[nodiscard]] virtual float maxHealth() const = 0;
    [[nodiscard]] virtual float moveSpeed() const = 0;
    [[nodiscard]] virtual float contactDamage() const = 0;
    [[nodiscard]] virtual float collisionRadius() const = 0;
    [[nodiscard]] virtual bool isDefeated() const = 0;

public slots:
    virtual void setWorldPosition(const QPointF &position) = 0;
    virtual void receiveDamage(float amount, DamageVisualType damageType = DamageVisualType::Neutral) = 0;
    virtual void setTargetPosition(const QPointF &position) = 0;
    virtual void advanceFrame(float deltaSeconds) = 0;
    virtual void applySlow(float factor, float durationSeconds) = 0;
    [[nodiscard]] virtual float slowFactor() const = 0;
    [[nodiscard]] virtual float slowRemainingSeconds() const = 0;

public:
    [[nodiscard]] virtual EnemyBehavior currentBehavior() const = 0;
    virtual void updateAI(float deltaSeconds, QPointF playerPosition) = 0;
    virtual void enterBossPhase(int phase) = 0;
    [[nodiscard]] virtual int currentBossPhase() const = 0;
    [[nodiscard]] virtual float chargeCooldownRemaining() const = 0;
    [[nodiscard]] virtual float chargeProgress() const = 0;
    [[nodiscard]] virtual float shootCooldownRemaining() const = 0;
    [[nodiscard]] virtual QString imagePath() const = 0;

signals:
    void positionChanged(const QPointF &position);
    void healthChanged(float currentHealth, float maxHealth);
    void damageReceived(DamageVisualType damageType, float amount);
    void defeated();
    void requestShoot(QPointF direction, int bulletCount, float bulletSpeed, float bulletDamage);
    void requestSuicideExplosion(float radius, float damage);
    void requestSpawnMinion();
};
