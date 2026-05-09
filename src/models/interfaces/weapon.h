#pragma once

#include <QObject>
#include <QPointF>

#include "game_data.h"

class Player;

class Weapon : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    ~Weapon() override = default;

    [[nodiscard]] virtual WeaponId id() const = 0;
    [[nodiscard]] virtual const WeaponConfig *config() const = 0;
    [[nodiscard]] virtual Player *owner() const = 0;
    [[nodiscard]] virtual bool isFiring() const = 0;

public slots:
    virtual void bindOwner(Player *owner) = 0;
    virtual void setAimDirection(const QPointF &direction) = 0;
    virtual void startFiring() = 0;
    virtual void stopFiring() = 0;
    virtual void advanceCooldown(float deltaMs) = 0;
    virtual void applyTrait(TraitId traitId) = 0;
    virtual void addExtraProjectiles(int count) = 0;
    virtual void applyRangeMultiplier(float multiplier) = 0;
    virtual void applyPierce(int extraPierces) = 0;
    virtual void applyBulletSizeScale(float scale) = 0;
    virtual void enableCombo(int interval, float damageMultiplier) = 0;
    [[nodiscard]] virtual float bulletSizeScale() const = 0;
    [[nodiscard]] virtual int pierceCount() const = 0;
    [[nodiscard]] virtual int extraProjectiles() const = 0;
    [[nodiscard]] virtual float rangeMultiplier() const = 0;
    [[nodiscard]] virtual int comboInterval() const = 0;
    [[nodiscard]] virtual float comboDamageMultiplier() const = 0;
    [[nodiscard]] virtual float fireRateScale() const = 0;
    [[nodiscard]] virtual int comboCounter() const = 0;

signals:
    void fireRequested(WeaponId weaponId, const QPointF &origin, const QPointF &direction);
    void cooldownChanged(float intervalMs);
};
