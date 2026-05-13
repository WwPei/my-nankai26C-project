#pragma once

#include <QMap>
#include <QObject>
#include <QPointF>

#include "game_data.h"
#include "player.h"

class BasicPlayer final : public Player
{
    Q_OBJECT

public:
    explicit BasicPlayer(const PlayerClassConfig *config, QObject *parent = nullptr);

    [[nodiscard]] PlayerClassId classId() const override;
    [[nodiscard]] QPointF worldPosition() const override;
    [[nodiscard]] float currentHealth() const override;
    [[nodiscard]] float maxHealth() const override;
    [[nodiscard]] float moveSpeed() const override;
    [[nodiscard]] WeaponId weaponId() const override;
    [[nodiscard]] float damageMultiplier() const override;
    [[nodiscard]] float defenseMultiplier() const override;
    [[nodiscard]] float speedMultiplier() const override;
    [[nodiscard]] float expMultiplier() const override;
    [[nodiscard]] QMap<TraitId, int> traitLevels() const override;

public slots:
    void setWorldPosition(const QPointF &position) override;
    void setMoveDirection(const QPointF &direction) override;
    void equipWeapon(WeaponId weaponId) override;
    void applyTrait(TraitId traitId) override;
    void applyWeaponUpgrade(WeaponUpgradeId upgradeId) override;
    void heal(float amount) override;
    void applySpeedBuff(float multiplier, float durationSeconds) override;
    void receiveDamage(float amount) override;

public:
    void dash() override;
    [[nodiscard]] bool isDashing() const override;
    [[nodiscard]] float dashCooldownRemaining() const override;
    [[nodiscard]] float dashCooldownTotal() const override;
    [[nodiscard]] QPointF dashDirection() const override;
    void updateDash(float deltaSeconds) override;

    void updateDashState(float deltaSeconds, const QPointF &moveDirection);

private:
    const PlayerClassConfig *m_config{nullptr};
    QPointF m_worldPosition;
    QPointF m_moveDirection;
    float m_currentHealth{0.0F};
    float m_damageMultiplier{1.0F};
    float m_defenseMultiplier{1.0F};
    float m_speedMultiplier{1.0F};
    WeaponId m_weaponId{WeaponId::PeaShooter};
    float m_expMultiplier{1.0F};
    float m_extraMaxHealth{0.0F};
    QMap<TraitId, int> m_traitLevels;
    bool m_isDashing{false};
    float m_dashDurationRemaining{0.0F};
    float m_dashCooldownRemaining{0.0F};
    float m_dashHardStunRemaining{0.0F};
    QPointF m_lastMoveDirection{0.0F, -1.0F};
};