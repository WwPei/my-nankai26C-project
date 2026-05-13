#pragma once

#include <QObject>
#include <QPointF>
#include <QPointer>

#include "weapon.h"

class Player;

class BasicWeapon final : public Weapon
{
    Q_OBJECT

public:
    explicit BasicWeapon(const WeaponConfig *config, QObject *parent = nullptr);

    [[nodiscard]] WeaponId id() const override;
    [[nodiscard]] const WeaponConfig *config() const override;
    [[nodiscard]] Player *owner() const override;
    [[nodiscard]] bool isFiring() const override;

public slots:
    void bindOwner(Player *owner) override;
    void setAimDirection(const QPointF &direction) override;
    void startFiring() override;
    void stopFiring() override;
    void advanceCooldown(float deltaMs) override;
    void applyTrait(TraitId traitId) override;
    void addExtraProjectiles(int count) override;
    void applyRangeMultiplier(float multiplier) override;
    void applyPierce(int extraPierces) override;
    void applyBulletSizeScale(float scale) override;
    void enableCombo(int interval, float damageMultiplier) override;
    [[nodiscard]] float bulletSizeScale() const override;
    [[nodiscard]] int pierceCount() const override;
    [[nodiscard]] int extraProjectiles() const override;
    [[nodiscard]] float rangeMultiplier() const override;
    [[nodiscard]] int comboInterval() const override;
    [[nodiscard]] float comboDamageMultiplier() const override;
    [[nodiscard]] float fireRateScale() const override;
    [[nodiscard]] int comboCounter() const override;

private:
    [[nodiscard]] float currentIntervalMs() const;

    const WeaponConfig *m_config {nullptr};
    QPointer<Player> m_owner;
    QPointF m_aimDirection {1.0, 0.0};
    float m_remainingCooldownMs {0.0F};
    float m_fireRateScale {1.0F};
    bool m_isFiring {false};
    int m_extraProjectileCount{0};
    float m_rangeMultiplierValue{1.0F};
    int m_pierceCountValue{0};
    float m_bulletSizeScaleValue{1.0F};
    int m_comboIntervalValue{0};
    float m_comboDamageMultiplierValue{1.0F};
    int m_comboCounterValue{0};
};
