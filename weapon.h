#ifndef WEAPON_H
#define WEAPON_H

#include <QObject>
#include <QString>
#include <QColor>
#include "game_data.h"

/**
 * @brief 武器纯数据类
 * 不再负责 new Bullet，只负责描述属性
 */
class Weapon : public QObject
{
    Q_OBJECT
public:
    explicit Weapon(const WeaponConfig &config, QObject *parent = nullptr);

    // 属性访问
    QString name() const { return m_name; }
    int damageBonus() const { return m_damageBonus; }
    double attackSpeed() const { return m_attackSpeed; }
    double projectileSpeed() const { return m_projectileSpeed; }
    QColor color() const { return m_color; }

private:
    QString m_name;
    int m_damageBonus;
    double m_attackSpeed;
    double m_projectileSpeed;
    QColor m_color;
};

#endif // WEAPON_H
