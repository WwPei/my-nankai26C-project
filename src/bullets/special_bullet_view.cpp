#include "special_bullet_view.h"

#include "bullet_data.h"
#include "combat_utils.h"
#include "special_bullet_data.h"

#include <QPainter>
#include <algorithm>

SpecialBulletView::SpecialBulletView(QGraphicsItem *parent)
    : BulletView(parent)
{
}

QRectF SpecialBulletView::boundingRect() const
{
    return QRectF(-m_radius, -m_radius, m_radius * 2.0, m_radius * 2.0);
}

QPainterPath SpecialBulletView::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void SpecialBulletView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const bool enemyBullet = m_model != nullptr && m_model->isEnemyBullet();
    const QColor bulletColor = enemyBullet
        ? QColor(255, 72, 72)
        : (m_model != nullptr ? bulletBaseColor(m_model->weaponId()) : QColor(254, 226, 92));
    const QPointF direction = m_model != nullptr ? normalizedDirection(m_model->direction()) : QPointF(1.0, 0.0);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QPointF tail = -direction * m_radius * 2.2;

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(bulletColor.red(), bulletColor.green(), bulletColor.blue(), 60));
    painter->drawEllipse(QRectF(tail.x() - m_radius, tail.y() - m_radius * 0.7, m_radius * 2.0, m_radius * 1.4));

    if (!m_iconPixmap.isNull()) {
        const QRectF bodyRect = boundingRect();
        const QRectF drawRect = bodyRect.adjusted(-m_radius * 0.3, -m_radius * 0.3, m_radius * 0.3, m_radius * 0.3);
        painter->drawPixmap(drawRect.toRect(), m_iconPixmap);
    } else {
        painter->setBrush(bulletColor);
        painter->drawEllipse(boundingRect());
        const QPointF side(-direction.y(), direction.x());
        painter->setBrush(bulletColor.lighter(160));
        QPainterPath glowPath;
        glowPath.moveTo(direction * (m_radius * 1.8));
        glowPath.lineTo(side * (m_radius * 0.55));
        glowPath.lineTo(-side * (m_radius * 0.55));
        glowPath.closeSubpath();
        painter->drawPath(glowPath);
    }
}

BulletData *SpecialBulletView::model() const
{
    return m_model;
}

void SpecialBulletView::bindModel(BulletData *data)
{
    if (m_model == data) {
        return;
    }

    if (m_model != nullptr) {
        disconnect(m_model, nullptr, this, nullptr);
    }

    m_model = data;
    if (m_model == nullptr) {
        return;
    }

    prepareGeometryChange();
    m_radius = m_model->collisionRadius();

    auto *specialData = qobject_cast<SpecialBulletData *>(m_model.data());
    if (specialData != nullptr) {
        const QString imagePath = specialData->currentImagePath();
        if (!imagePath.isEmpty()) {
            m_iconPixmap = QPixmap(imagePath);
        }
    }

    connect(m_model, &BulletData::positionChanged, this, [this](const QPointF &) {
        syncFromData();
    });
    connect(m_model, &BulletData::expired, this, &GameView::removalRequested);
    syncFromData();
}

void SpecialBulletView::syncFromData()
{
    if (m_model == nullptr) {
        return;
    }

    const qreal newRadius = std::max<qreal>(4.0, m_model->collisionRadius());
    if (!qFuzzyCompare(m_radius, newRadius)) {
        prepareGeometryChange();
        m_radius = newRadius;
    }

    auto *specialData = qobject_cast<SpecialBulletData *>(m_model.data());
    if (specialData != nullptr) {
        const QString imagePath = specialData->currentImagePath();
        if (!imagePath.isEmpty()) {
            QPixmap newPixmap(imagePath);
            if (newPixmap.cacheKey() != m_iconPixmap.cacheKey()) {
                m_iconPixmap = newPixmap;
            }
        }
    }

    setPos(m_model->worldPosition());
    update();
}
