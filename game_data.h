#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <QString>
#include <QColor>
#include <QPointF>

// 职业数据结构，用于从职业选择页传递到游戏主页面
struct ClassData {
    QString name;        // 职业名称
    QString iconPath;    // 图标资源路径（预留）
    int hp;              // 最大血量
    int strength;        // 力量（影响攻击伤害）
    double attackSpeed;  // 攻击速度（每秒攻击次数）
};

// 武器静态配置 (这里简化演示，实际可从JSON读取)
struct WeaponConfig {
    QString name;
    int damageBonus;
    double attackSpeed;
    double projectileSpeed;
    QColor color;
};

#endif // GAME_DATA_H
