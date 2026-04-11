#include "bullet_data.h"

BulletData::BulletData(QObject *parent)
    : QObject(parent), m_speed(0), m_damage(0), m_isActive(false) {}

void BulletData::init(const QPointF &spawnPos, const QPointF &dir, double speed, int damage)
{
    m_pos = spawnPos;
    m_dir = dir;
    m_speed = speed;
    m_damage = damage;
    m_isActive = true;
}

void BulletData::update()
{
    if (!m_isActive) return;
    m_pos += m_dir * m_speed;
    emit sigMoved();
}

void BulletData::setPos(const QPointF &p)
{
    m_pos = p;
}

void BulletData::deactivate()
{
    m_isActive = false;
}
