#pragma once

#include <QColor>
#include <QLineF>
#include <QPointF>

#include "game_data.h"

[[nodiscard]] QPointF normalizedDirection(const QPointF &direction, const QPointF &fallback = QPointF(1.0, 0.0));

[[nodiscard]] QColor enemyBaseColor(EnemyId id);

[[nodiscard]] QColor enemyHealthBarColor(float ratio);

[[nodiscard]] DamageVisualType damageVisualTypeForWeapon(WeaponId id);

[[nodiscard]] QColor damageFlashColor(DamageVisualType type);

[[nodiscard]] QColor bulletBaseColor(WeaponId id);
