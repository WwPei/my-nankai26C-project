#ifndef BULLET_DATA_H
#define BULLET_DATA_H

#include <QObject>
#include <QPointF>

class BulletData : public QObject
{
    Q_OBJECT
public:
    explicit BulletData(QObject *parent = nullptr);

    // 初始化数据
    void init(const QPointF &spawnPos, const QPointF &dir, double speed, int damage);

    // 数据访问
    QPointF pos() const { return m_pos; }
    QPointF dir() const { return m_dir; }
    double speed() const { return m_speed; }
    int damage() const { return m_damage; }
    bool isActive() const { return m_isActive; }

    // 逻辑操作
    void update(); // 每帧移动
    void setPos(const QPointF &p);
    void deactivate(); // 标记为失效（命中或出界）

signals:
    void sigMoved();

private:
    QPointF m_pos;
    QPointF m_dir;
    double m_speed;
    int m_damage;
    bool m_isActive;
};

#endif // BULLET_DATA_H
