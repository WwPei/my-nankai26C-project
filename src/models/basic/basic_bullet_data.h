#pragma once

#include <QObject>
#include <QPointF>

#include "bullet_data.h"

class BasicBulletData : public BulletData
{
    Q_OBJECT

public:
    BasicBulletData(WeaponId weaponId,
                    const BulletConfig *config,
                    const QPointF &spawnPosition,
                    const QPointF &direction,
                    QObject *parent = nullptr);

    [[nodiscard]] WeaponId weaponId() const override;
    [[nodiscard]] const BulletConfig *config() const override;
    [[nodiscard]] QPointF worldPosition() const override;
    [[nodiscard]] QPointF direction() const override;
    [[nodiscard]] float damage() const override;
    [[nodiscard]] float speed() const override;
    [[nodiscard]] float collisionRadius() const override;
    [[nodiscard]] bool isExpired() const override;
    [[nodiscard]] bool isCritical() const override;
    [[nodiscard]] int remainingPierceCount() const override;
    [[nodiscard]] float sizeScale() const override;
    [[nodiscard]] float comboMultiplier() const override;
    [[nodiscard]] bool isEnemyBullet() const override;

public slots:
    void setWorldPosition(const QPointF &position) override;
    void setDirection(const QPointF &direction) override;
    void advanceFrame(float deltaSeconds) override;
    void expire() override;
    void setCritical(bool critical) override;
    void setPierceCount(int count) override;
    void setSizeScale(float scale) override;
    void setComboMultiplier(float multiplier) override;
    void setDamageMultiplier(float multiplier) override;
    void setEnemyBullet(bool enemyBullet) override;
    void setSpeed(float speed) override;

private:
    WeaponId m_weaponId {WeaponId::PeaShooter};
    const BulletConfig *m_config {nullptr};
    QPointF m_worldPosition;
    QPointF m_direction {1.0, 0.0};
    bool m_expired {false};
    bool m_isCritical{false};
    int m_pierceCount{0};
    float m_sizeScaleValue{1.0F};
    float m_comboMultiplierValue{1.0F};
    float m_damageMultiplierValue{1.0F};
    bool m_isEnemyBullet{false};
    float m_speedValue{-1.0F};
};
