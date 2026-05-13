#include "player_avatar_item.h"

#include <QPainter>
#include <QPainterPath>

#include <algorithm>
#include <cmath>

namespace {

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
    return QRectF(-m_renderRadius - 4.0, -m_renderRadius - 18.0,
                  m_renderRadius * 2.0 + 8.0, m_renderRadius * 2.0 + 22.0);
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

void PlayerAvatarItem::setAimArrow(const QString &resourcePath)
{
    loadImage(resourcePath, m_aimArrow);
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

void PlayerAvatarItem::setHealthRatio(qreal ratio)
{
    const qreal bounded = std::clamp(ratio, 0.0, 1.0);
    if (qFuzzyCompare(m_healthRatio, bounded)) {
        return;
    }

    m_healthRatio = bounded;
    if (m_imagesLoaded) {
        update();
    }
}

void PlayerAvatarItem::setIsDashing(bool dashing)
{
    if (m_isDashing == dashing) {
        return;
    }

    m_isDashing = dashing;
    if (m_imagesLoaded) {
        update();
    }
}

void PlayerAvatarItem::setIsHurt(bool hurt)
{
    if (m_isHurt == hurt) {
        return;
    }

    m_isHurt = hurt;
    if (m_imagesLoaded) {
        update();
    }
}

void PlayerAvatarItem::loadImage(const QString &path, QPixmap &target)
{
    target = QPixmap(path);
}

QPixmap PlayerAvatarItem::currentFacePixmap() const
{
    if (!m_imagesLoaded) {
        return QPixmap();
    }

    if (m_isDashing) {
        return m_faceDash;
    }

    if (m_isHurt) {
        return m_faceHurt;
    }

    if (m_healthRatio >= 0.999F) {
        return m_faceFull;
    }

    if (m_healthRatio > 0.5F) {
        return m_faceMid;
    }

    if (m_healthRatio > 0.3F) {
        return m_faceNormal;
    }

    return m_faceLowHp;
}

void PlayerAvatarItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!m_imagesLoaded) {
        loadImage(QStringLiteral(":/character/loudly_crying_face_3d.png"), m_faceNormal);
        loadImage(QStringLiteral(":/character/shushing_face_3d.png"), m_faceMid);
        loadImage(QStringLiteral(":/character/pouting_face_3d.png"), m_faceHurt);
        loadImage(QStringLiteral(":/character/rolling_on_the_floor_laughing_3d.png"), m_faceDash);
        loadImage(QStringLiteral(":/character/smiling_face_with_halo_3d.png"), m_faceLowHp);
        loadImage(QStringLiteral(":/character/smiling_face_with_sunglasses_3d.png"), m_faceFull);
        m_imagesLoaded = true;
    }

    const QColor shadowColor(30, 120, 255, static_cast<int>(90 + 60 * (1.0 - m_hitFlashIntensity)));
    painter->setPen(Qt::NoPen);
    painter->setBrush(shadowColor);
    painter->drawEllipse(QRectF(-m_renderRadius - 2.0, -m_renderRadius - 2.0,
                                m_renderRadius * 2.0 + 4.0, m_renderRadius * 2.0 + 4.0));

    const QPixmap face = currentFacePixmap();
    if (!face.isNull()) {
        const qreal flashScale = 1.0 + m_hitFlashIntensity * 0.08;
        const QRectF faceRect(-m_renderRadius * flashScale, -m_renderRadius * flashScale,
                              m_renderRadius * 2.0 * flashScale, m_renderRadius * 2.0 * flashScale);
        painter->drawPixmap(faceRect, face, face.rect());

        if (m_hitFlashIntensity > 0.01) {
            painter->setBrush(QColor(255, 143, 143,
                                     static_cast<int>(72 * m_hitFlashIntensity)));
            painter->drawEllipse(faceRect.adjusted(-4.0, -4.0, 4.0, 4.0));
        }
    } else {
        const QColor bodyColor = QColor::fromRgbF(
            0.255 + 0.45 * m_hitFlashIntensity,
            0.412 + 0.16 * m_hitFlashIntensity,
            0.882 + 0.06 * m_hitFlashIntensity,
            1.0);
        painter->setBrush(bodyColor);
        painter->drawEllipse(QRectF(-m_renderRadius, -m_renderRadius,
                                    m_renderRadius * 2.0, m_renderRadius * 2.0));
    }

    if (!m_aimArrow.isNull()) {
        painter->save();
        const QPointF arrowCenter = m_aimDirection * (m_renderRadius + 16.0);
        painter->translate(arrowCenter);
        const qreal angle = std::atan2(-m_aimDirection.y(), m_aimDirection.x()) * 180.0 / 3.14159265;
        painter->rotate(angle);
        constexpr qreal arrowSize = 28.0;
        painter->drawPixmap(QRectF(-arrowSize * 0.5, -arrowSize * 0.5,
                                   arrowSize, arrowSize),
                            m_aimArrow, m_aimArrow.rect());
        painter->restore();
    }
}