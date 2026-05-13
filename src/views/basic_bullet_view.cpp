#include "basic_bullet_view.h"

#include "bullet_data.h"
#include "combat_utils.h"

#include <QPainter>

BasicBulletView::BasicBulletView(QGraphicsItem *parent)
    : BulletView(parent)
{
}

QRectF BasicBulletView::boundingRect() const
{
    return QRectF(-m_radius, -m_radius, m_radius * 2.0, m_radius * 2.0);
}

QPainterPath BasicBulletView::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void BasicBulletView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const bool enemyBullet = m_model != nullptr && m_model->isEnemyBullet();
    const QColor bulletColor = enemyBullet
        ? QColor(255, 72, 72)
        : (m_model != nullptr ? bulletBaseColor(m_model->weaponId()) : QColor(254, 226, 92));
    const QPointF direction = m_model != nullptr ? normalizedDirection(m_model->direction()) : QPointF(1.0, 0.0);
    const QPointF tail = -direction * m_radius * 2.2;
    const QPointF side(-direction.y(), direction.x());

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);

    // 拖尾效果（半透明椭圆）
    painter->setBrush(QColor(bulletColor.red(), bulletColor.green(), bulletColor.blue(), 80));
    painter->drawEllipse(QRectF(tail.x() - m_radius, tail.y() - m_radius * 0.8, m_radius * 2.0, m_radius * 1.6));

    // 主体圆形
    painter->setBrush(bulletColor);
    painter->drawEllipse(boundingRect());

    // 前端发光箭头（光晕效果）
    painter->setBrush(bulletColor.lighter(160));
    QPainterPath glowPath;
    glowPath.moveTo(direction * (m_radius * 1.8));
    glowPath.lineTo(side * (m_radius * 0.55));
    glowPath.lineTo(-side * (m_radius * 0.55));
    glowPath.closeSubpath();
    painter->drawPath(glowPath);
}

BulletData *BasicBulletView::model() const
{
    return m_model;
}

void BasicBulletView::bindModel(BulletData *data)
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
    m_radius = m_model->collisionRadius() * 1.6;
    connect(m_model, &BulletData::positionChanged, this, [this](const QPointF &) {
        syncFromData();
    });
    connect(m_model, &BulletData::expired, this, &GameView::removalRequested);
    syncFromData();
}

void BasicBulletView::syncFromData()
{
    if (m_model == nullptr) {
        return;
    }

    const qreal newRadius = std::max<qreal>(6.0, m_model->collisionRadius() * 1.6);
    if (!qFuzzyCompare(m_radius, newRadius)) {
        prepareGeometryChange();
        m_radius = newRadius;
    }
    setPos(m_model->worldPosition());
    update();
}
