#pragma once

#include <QObject>
#include <QPointF>

#include "game_data.h"

class BulletData : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    ~BulletData() override = default;

    [[nodiscard]] virtual WeaponId weaponId() const = 0;
    [[nodiscard]] virtual const BulletConfig *config() const = 0;
    [[nodiscard]] virtual QPointF worldPosition() const = 0;
    [[nodiscard]] virtual QPointF direction() const = 0;
    [[nodiscard]] virtual float damage() const = 0;
    [[nodiscard]] virtual float speed() const = 0;
    [[nodiscard]] virtual float collisionRadius() const = 0;
    [[nodiscard]] virtual bool isCritical() const = 0;
    [[nodiscard]] virtual bool isExpired() const = 0;
    [[nodiscard]] virtual int remainingPierceCount() const = 0;
    [[nodiscard]] virtual float sizeScale() const = 0;
    [[nodiscard]] virtual float comboMultiplier() const = 0;
    [[nodiscard]] virtual bool isEnemyBullet() const = 0;

public slots:
    virtual void setWorldPosition(const QPointF &position) = 0;
    virtual void setDirection(const QPointF &direction) = 0;
    virtual void advanceFrame(float deltaSeconds) = 0;
    virtual void expire() = 0;
    virtual void setCritical(bool critical) = 0;
    virtual void setPierceCount(int count) = 0;
    virtual void setSizeScale(float scale) = 0;
    virtual void setComboMultiplier(float multiplier) = 0;
    virtual void setDamageMultiplier(float multiplier) = 0;
    virtual void setEnemyBullet(bool enemyBullet) = 0;
    virtual void setSpeed(float speed) = 0;

signals:
    void positionChanged(const QPointF &position);
    void expired();
};
