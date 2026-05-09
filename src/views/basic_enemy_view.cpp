#include "basic_enemy_view.h"

#include "combat_utils.h"
#include "enemy_data.h"

#include <QPainter>
#include <QTimer>

#include <algorithm>

BasicEnemyView::BasicEnemyView(QGraphicsItem *parent)
    : EnemyView(parent)
    , m_hitFlashTimer(new QTimer(this))
{
    setZValue(3.0);
    m_hitFlashTimer->setInterval(32);
    connect(m_hitFlashTimer, &QTimer::timeout, this, [this]() {
        m_hitFlashIntensity = std::max(0.0, m_hitFlashIntensity - 0.16);
        if (m_hitFlashIntensity <= 0.0) {
            m_hitFlashTimer->stop();
        }
        update();
    });
}

QRectF BasicEnemyView::boundingRect() const
{
    return QRectF(-m_radius - 4.0, -m_radius - 18.0, m_radius * 2.0 + 8.0, m_radius * 2.0 + 22.0);
}

QPainterPath BasicEnemyView::shape() const
{
    QPainterPath path;
    path.addEllipse(QRectF(-m_radius, -m_radius, m_radius * 2.0, m_radius * 2.0));
    return path;
}

void BasicEnemyView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    const qreal flashScale = 1.0 + m_hitFlashIntensity * 0.08;
    const QRectF bodyRect(-m_radius * flashScale, -m_radius * flashScale,
                          m_radius * 2.0 * flashScale, m_radius * 2.0 * flashScale);

    if (m_hasImage && !m_pixmap.isNull()) {
        painter->drawPixmap(
            QRectF(-m_radius * flashScale, -m_radius * flashScale,
                   m_radius * 2.0 * flashScale, m_radius * 2.0 * flashScale),
            m_pixmap, m_pixmap.rect());
    } else {
        const QColor baseColor = m_model != nullptr ? enemyBaseColor(m_model->id()) : QColor(224, 92, 92);
        const QColor outlineColor = QColor::fromRgbF(
            std::clamp(baseColor.redF() + 0.12 * m_hitFlashIntensity, 0.0, 1.0),
            std::clamp(baseColor.greenF() + 0.12 * m_hitFlashIntensity, 0.0, 1.0),
            std::clamp(baseColor.blueF() + 0.12 * m_hitFlashIntensity, 0.0, 1.0),
            1.0).lighter(140);
        const QColor bodyColor = QColor::fromRgbF(
            std::clamp(baseColor.redF() + 0.32 * m_hitFlashIntensity, 0.0, 1.0),
            std::clamp(baseColor.greenF() + 0.18 * m_hitFlashIntensity, 0.0, 1.0),
            std::clamp(baseColor.blueF() + 0.18 * m_hitFlashIntensity, 0.0, 1.0),
            1.0);

        painter->setPen(QPen(outlineColor, 1.4));
        painter->setBrush(bodyColor);
        painter->drawEllipse(bodyRect);

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 48));
        painter->drawEllipse(QRectF(-m_radius * 0.45, -m_radius * 0.55, m_radius * 0.75, m_radius * 0.65));

        if (m_model != nullptr && m_model->id() == EnemyId::SkeletonNew) {
            painter->setBrush(QColor(QStringLiteral("#262a33")));
            painter->drawEllipse(QRectF(-m_radius * 0.42, -m_radius * 0.2, m_radius * 0.22, m_radius * 0.26));
            painter->drawEllipse(QRectF(m_radius * 0.2, -m_radius * 0.2, m_radius * 0.22, m_radius * 0.26));
        }
    }

    const QColor flashColor = m_hitFlashColor.isValid()
        ? m_hitFlashColor
        : QColor(QStringLiteral("#ff8f8f"));
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(flashColor.red(),
                             flashColor.green(),
                             flashColor.blue(),
                             static_cast<int>(72 * m_hitFlashIntensity)));
    painter->drawEllipse(bodyRect.adjusted(-4.0, -4.0, 4.0, 4.0));

    const QRectF healthBarRect(-m_radius, -m_radius - 12.0, m_radius * 2.0, 5.0);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(18, 20, 24, 220));
    painter->drawRoundedRect(healthBarRect.adjusted(-1.0, -1.0, 1.0, 1.0), 3.0, 3.0);
    painter->setBrush(QColor(42, 47, 58));
    painter->drawRoundedRect(healthBarRect, 2.5, 2.5);

    if (m_healthRatio > 0.0F) {
        QRectF healthChunk = healthBarRect.adjusted(0.8, 0.8, -0.8, -0.8);
        healthChunk.setWidth(healthChunk.width() * std::clamp<qreal>(m_healthRatio, 0.0, 1.0));
        painter->setBrush(enemyHealthBarColor(m_healthRatio));
        painter->drawRoundedRect(healthChunk, 2.0, 2.0);
    }
}

EnemyData *BasicEnemyView::model() const
{
    return m_model;
}

void BasicEnemyView::bindModel(EnemyData *data)
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

    const QString path = m_model->imagePath();
    if (!path.isEmpty()) {
        m_pixmap.load(path);
        m_hasImage = !m_pixmap.isNull();
    } else {
        m_pixmap = QPixmap();
        m_hasImage = false;
    }

    connect(m_model, &EnemyData::positionChanged, this, [this](const QPointF &) {
        syncFromData();
    });
    connect(m_model, &EnemyData::damageReceived, this, [this](DamageVisualType damageType, float) {
        m_hitFlashColor = damageFlashColor(damageType);
        m_hitFlashIntensity = 1.0;
        m_hitFlashTimer->start();
        update();
    });
    connect(m_model, &EnemyData::healthChanged, this, [this](float currentHealth, float maxHealth) {
        m_healthRatio = maxHealth > 0.0F ? currentHealth / maxHealth : 0.0F;
        update();
    });
    connect(m_model, &EnemyData::defeated, this, &GameView::removalRequested);
    syncFromData();
}

void BasicEnemyView::syncFromData()
{
    if (m_model == nullptr) {
        return;
    }

    const qreal newRadius = std::max<qreal>(12.0, m_model->collisionRadius());
    if (!qFuzzyCompare(m_radius, newRadius)) {
        prepareGeometryChange();
        m_radius = newRadius;
    }
    m_healthRatio = m_model->maxHealth() > 0.0F
        ? m_model->currentHealth() / m_model->maxHealth()
        : 0.0F;
    setPos(m_model->worldPosition());
    update();
}
