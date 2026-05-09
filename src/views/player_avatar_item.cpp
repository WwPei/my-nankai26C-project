#include "player_avatar_item.h"

#include <QPainter>
#include <QPainterPath>

#include <cmath>

namespace {

constexpr qreal kPlayerVisualRadius = 16.0;

[[nodiscard]] QPointF normalizedVector(const QPointF &vector, const QPointF &fallback = QPointF(1.0, 0.0))
{
    const QLineF line(QPointF(), vector);
    if (line.length() <= 0.001) {
        return fallback;
    }

    return QPointF(vector.x() / line.length(), vector.y() / line.length());
}

} // namespace

PlayerAvatarItem::PlayerAvatarItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setZValue(5.0);
}

[[nodiscard]] QRectF PlayerAvatarItem::boundingRect() const
{
    return QRectF(-28.0, -28.0, 56.0, 56.0);
}

void PlayerAvatarItem::setAimDirection(const QPointF &direction)
{
    const QPointF normalized = normalizedVector(direction);
    if (QLineF(normalized, m_aimDirection).length() <= 0.0001) {
        return;
    }

    m_aimDirection = normalized;
    update();
}

void PlayerAvatarItem::setHitFlash(qreal intensity)
{
    const qreal bounded = std::clamp(intensity, 0.0, 1.0);
    if (qFuzzyCompare(m_hitFlashIntensity, bounded)) {
        return;
    }

    m_hitFlashIntensity = bounded;
    update();
}

void PlayerAvatarItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    const QColor shadowColor(30, 120, 255, static_cast<int>(90 + 60 * (1.0 - m_hitFlashIntensity)));
    painter->setPen(Qt::NoPen);
    painter->setBrush(shadowColor);
    painter->drawEllipse(QRectF(-22.0, -22.0, 44.0, 44.0));

    const QColor bodyColor = QColor::fromRgbF(
        0.255 + 0.45 * m_hitFlashIntensity,
        0.412 + 0.16 * m_hitFlashIntensity,
        0.882 + 0.06 * m_hitFlashIntensity,
        1.0);
    painter->setBrush(bodyColor);
    painter->drawEllipse(QRectF(-kPlayerVisualRadius, -kPlayerVisualRadius,
                                kPlayerVisualRadius * 2.0, kPlayerVisualRadius * 2.0));

    painter->setPen(QPen(QColor(210, 232, 255), 1.4));
    painter->setBrush(QColor(195, 225, 255));
    QPainterPath pointerPath;
    const QPointF forward = m_aimDirection * 23.0;
    const QPointF side(-m_aimDirection.y(), m_aimDirection.x());
    pointerPath.moveTo(forward);
    pointerPath.lineTo(m_aimDirection * 8.0 + side * 6.0);
    pointerPath.lineTo(m_aimDirection * 8.0 - side * 6.0);
    pointerPath.closeSubpath();
    painter->drawPath(pointerPath);
}
