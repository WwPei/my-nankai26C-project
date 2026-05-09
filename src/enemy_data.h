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
    virtual void receiveDamage(float amount) = 0;
    virtual void setTargetPosition(const QPointF &position) = 0;
    virtual void advanceFrame(float deltaSeconds) = 0;

signals:
    void positionChanged(const QPointF &position);
    void healthChanged(float currentHealth, float maxHealth);
    void defeated();
};
