#include "combat_utils.h"

#include <QLineF>

[[nodiscard]] QPointF normalizedDirection(const QPointF &direction, const QPointF &fallback)
{
    const QLineF line(QPointF(), direction);
    if (line.length() <= 0.001) {
        return fallback;
    }

    return QPointF(direction.x() / line.length(), direction.y() / line.length());
}

[[nodiscard]] QColor enemyBaseColor(EnemyId id)
{
    return QColor(QStringLiteral("#d96b6b"));
}

[[nodiscard]] QColor enemyHealthBarColor(float ratio)
{
    if (ratio < 0.25F) {
        return QColor(QStringLiteral("#e05a5a"));
    }
    if (ratio < 0.5F) {
        return QColor(QStringLiteral("#f0c24b"));
    }
    return QColor(QStringLiteral("#32c766"));
}

[[nodiscard]] DamageVisualType damageVisualTypeForWeapon(WeaponId id)
{
    switch (id) {
    case WeaponId::PeaShooter:
        return DamageVisualType::Neutral;
    case WeaponId::SpreadBlaster:
        return DamageVisualType::Rapid;
    case WeaponId::ArcWand:
        return DamageVisualType::Arcane;
    }

    return DamageVisualType::Neutral;
}

[[nodiscard]] QColor damageFlashColor(DamageVisualType type)
{
    switch (type) {
    case DamageVisualType::Neutral:
        return QColor(QStringLiteral("#ff8f8f"));
    case DamageVisualType::Rapid:
        return QColor(QStringLiteral("#ffb347"));
    case DamageVisualType::Arcane:
        return QColor(QStringLiteral("#8d74ff"));
    }

    return QColor(QStringLiteral("#ff8f8f"));
}

[[nodiscard]] QColor bulletBaseColor(WeaponId id)
{
    switch (id) {
    case WeaponId::PeaShooter:
        return QColor(QStringLiteral("#f2de59"));
    case WeaponId::SpreadBlaster:
        return QColor(QStringLiteral("#ff934f"));
    case WeaponId::ArcWand:
        return QColor(QStringLiteral("#8d74ff"));
    }

    return QColor(QStringLiteral("#f2de59"));
}
