#include "special_bullet_data.h"

#include "combat_utils.h"

#include <QRandomGenerator>
#include <QtMath>

SpecialBulletData::SpecialBulletData(WeaponId weaponId,
                                     const BulletConfig *config,
                                     const QPointF &spawnPos,
                                     const QPointF &direction,
                                     const BulletSpecialConfig &specialConfig,
                                     QObject *parent)
    : BasicBulletData(weaponId, config, spawnPos, direction, parent)
    , m_specialConfig(specialConfig)
{
    if (!m_specialConfig.altImagePaths.isEmpty()) {
        m_imageCycleIndex = QRandomGenerator::global()->bounded(m_specialConfig.altImagePaths.size());
    }
}

SpecialEffect SpecialBulletData::specialEffect() const
{
    return m_specialConfig.effect;
}

const BulletSpecialConfig &SpecialBulletData::specialConfig() const
{
    return m_specialConfig;
}

QString SpecialBulletData::currentImagePath() const
{
    if (!m_specialConfig.altImagePaths.isEmpty()) {
        return m_specialConfig.altImagePaths.at(m_imageCycleIndex);
    }
    return m_specialConfig.imagePath;
}

bool SpecialBulletData::isAttached() const
{
    return m_attached;
}

void SpecialBulletData::setAttached(bool attached, int targetIndex)
{
    m_attached = attached;
    m_attachedTarget = targetIndex;
}

int SpecialBulletData::attachedTargetIndex() const
{
    return m_attachedTarget;
}

float SpecialBulletData::attachTimer() const
{
    return m_attachTimer;
}

void SpecialBulletData::advanceAttachTimer(float dt)
{
    m_attachTimer += dt;
}

void SpecialBulletData::advanceFrame(float deltaSeconds)
{
    if (m_attached) {
        advanceAttachTimer(deltaSeconds);
        return;
    }
    BasicBulletData::advanceFrame(deltaSeconds);
}

void SpecialBulletData::advanceSpecialFrame(float deltaSeconds, const QVector<QPointF> &enemyPositions)
{
    if (m_specialConfig.effect != SpecialEffect::TrackAndAttach) {
        BasicBulletData::advanceFrame(deltaSeconds);
        return;
    }

    if (m_attached) {
        advanceAttachTimer(deltaSeconds);
        return;
    }

    const QPointF myPos = worldPosition();
    float nearestDist = m_specialConfig.param2 > 0.0F ? m_specialConfig.param2 : 200.0F;
    int nearestIdx = -1;

    for (int i = 0; i < enemyPositions.size(); ++i) {
        const QPointF diff = enemyPositions.at(i) - myPos;
        const float dist = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());
        if (dist < nearestDist) {
            nearestDist = dist;
            nearestIdx = i;
        }
    }

    if (nearestIdx >= 0) {
        const QPointF diff = enemyPositions.at(nearestIdx) - myPos;
        const float dist = qSqrt(diff.x() * diff.x() + diff.y() * diff.y());
        if (dist > 1.0F) {
            setDirection(QPointF(diff.x() / dist, diff.y() / dist));
        }
        BasicBulletData::advanceFrame(deltaSeconds);
    } else {
        BasicBulletData::advanceFrame(deltaSeconds);
    }
}
