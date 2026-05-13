#pragma once

#include <QMap>
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
    [[nodiscard]] virtual float damageMultiplier() const = 0;
    [[nodiscard]] virtual float defenseMultiplier() const = 0;
    [[nodiscard]] virtual float speedMultiplier() const = 0;
    [[nodiscard]] virtual float expMultiplier() const = 0;
    [[nodiscard]] virtual QMap<TraitId, int> traitLevels() const = 0;

public slots:
    virtual void setWorldPosition(const QPointF &position) = 0;
    virtual void setMoveDirection(const QPointF &direction) = 0;
    virtual void equipWeapon(WeaponId weaponId) = 0;
    virtual void applyTrait(TraitId traitId) = 0;
    virtual void receiveDamage(float amount) = 0;
    virtual void applyWeaponUpgrade(WeaponUpgradeId upgradeId) = 0;
    virtual void heal(float amount) = 0;
    virtual void applySpeedBuff(float multiplier, float durationSeconds) = 0;

public:
    virtual void dash() = 0;
    [[nodiscard]] virtual bool isDashing() const = 0;
    [[nodiscard]] virtual float dashCooldownRemaining() const = 0;
    [[nodiscard]] virtual float dashCooldownTotal() const = 0;
    [[nodiscard]] virtual QPointF dashDirection() const = 0;
    virtual void updateDash(float deltaSeconds) = 0;

signals:
    void moved(const QPointF &position);
    void healthChanged(float currentHealth, float maxHealth);
    void weaponChanged(WeaponId weaponId);
    void defeated();
    void dashCooldownChanged(float remainingSeconds, float totalSeconds);
};
