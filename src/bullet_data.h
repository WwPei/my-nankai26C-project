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
    [[nodiscard]] virtual bool isExpired() const = 0;

public slots:
    virtual void setWorldPosition(const QPointF &position) = 0;
    virtual void setDirection(const QPointF &direction) = 0;
    virtual void advanceFrame(float deltaSeconds) = 0;
    virtual void expire() = 0;

signals:
    void positionChanged(const QPointF &position);
    void expired();
};
