#ifndef ENEMY_DATA_H
#define ENEMY_DATA_H

#include <QObject>
#include <QPointF>
#include "game_data.h" // IWYU pragma: keep

class EnemyData : public QObject
{
    Q_OBJECT
public:
    // 这里的 type 对应配置表ID，比如 1=普通怪，2=精英怪
    explicit EnemyData(int typeId, QObject *parent = nullptr);

    // 数据访问
    int typeId() const { return m_typeId; }
    int hp() const { return m_hp; }
    int maxHp() const { return m_maxHp; }
    int damage() const { return m_damage; }
    qreal speed() const { return m_speed; }
    QPointF pos() const { return m_pos; }
    bool isDead() const { return m_isDead; }

    // 逻辑操作
    void setPos(const QPointF &p);
    void moveBy(qreal dx, qreal dy);
    void takeDamage(int dmg);
    void markAsDead(); // 标记死亡，由外部清理

signals:
    void sigPositionChanged(); // 位置变了
    void sigHpChanged();       // 血量变了
    void sigHurt();            // 受伤了 (用于特效)
    void sigDied();            // 死亡了

private:
    void initFromConfig(int typeId); // 从配置表初始化

    int m_typeId;
    int m_hp;
    int m_maxHp;
    int m_damage;
    qreal m_speed;
    QPointF m_pos;
    bool m_isDead;
};


#endif // ENEMY_DATA_H
