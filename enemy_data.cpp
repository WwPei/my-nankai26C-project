#include "enemy_data.h"

EnemyData::EnemyData(int typeId, QObject *parent)
    : QObject(parent), m_typeId(typeId), m_isDead(false)
{
    initFromConfig(typeId);
}

void EnemyData::initFromConfig(int typeId)
{
    // 模拟配置表读取
    if (typeId == 1) {
        m_maxHp = 50;
        m_hp = 50;
        m_damage = 10;
        m_speed = 2.0;
    }
}

void EnemyData::setPos(const QPointF &p)
{
    if (m_pos == p) return;
    m_pos = p;
    emit sigPositionChanged();
}

void EnemyData::moveBy(qreal dx, qreal dy)
{
    m_pos += QPointF(dx, dy);
    emit sigPositionChanged();
}

void EnemyData::takeDamage(int dmg)
{
    if (m_isDead) return;
    m_hp -= dmg;
    if (m_hp < 0) m_hp = 0;

    emit sigHurt();
    emit sigHpChanged();

    if (m_hp <= 0) {
        markAsDead();
    }
}

void EnemyData::markAsDead()
{
    if (m_isDead) return;
    m_isDead = true;
    emit sigDied();
}
