#pragma once

#include <QObject>
#include <QPointF>

#include "game_data.h"

class Player : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    ~Player() override = default;

    [[nodiscard]] virtual PlayerClassId classId() const = 0;
    [[nodiscard]] virtual QPointF worldPosition() const = 0;
    [[nodiscard]] virtual float currentHealth() const = 0;
    [[nodiscard]] virtual float maxHealth() const = 0;
    [[nodiscard]] virtual float moveSpeed() const = 0;
    [[nodiscard]] virtual WeaponId weaponId() const = 0;

public slots:
    virtual void setWorldPosition(const QPointF &position) = 0;
    virtual void setMoveDirection(const QPointF &direction) = 0;
    virtual void equipWeapon(WeaponId weaponId) = 0;
    virtual void applyTrait(TraitId traitId) = 0;
    virtual void receiveDamage(float amount) = 0;

signals:
    void moved(const QPointF &position);
    void healthChanged(float currentHealth, float maxHealth);
    void weaponChanged(WeaponId weaponId);
    void defeated();
};
