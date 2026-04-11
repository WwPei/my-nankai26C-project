#ifndef TRAIT_H
#define TRAIT_H

#include <QObject>
#include <QString>

class PlayerItem;  // 前置声明

/**
 * @brief 特性基类
 *
 * 代表一个可装备的 Roguelike 特性，可影响玩家属性或提供特殊效果。
 * 最多可同时激活 4 个特性。
 */
class Trait : public QObject
{
    Q_OBJECT
public:
    explicit Trait(const QString &name, QObject *parent = nullptr);
    virtual ~Trait();

    QString name() const { return m_name; }

    /**
     * @brief 当特性被装备到玩家时调用
     */
    virtual void onEquip(PlayerItem *player);

    /**
     * @brief 当特性被移除时调用
     */
    virtual void onUnequip(PlayerItem *player);

    /**
     * @brief 每帧更新（用于持续效果）
     * @param player 玩家对象
     * @param deltaTime 帧间隔时间（秒）
     */
    virtual void update(PlayerItem *player, float deltaTime);

protected:
    QString m_name;  ///< 特性名称
};

// ---------- 示例特性 ----------

/**
 * @brief 增加最大生命值的特性
 */
class TraitMaxHealth : public Trait
{
    Q_OBJECT
public:
    TraitMaxHealth(int bonusHp);

    void onEquip(PlayerItem *player) override;
    void onUnequip(PlayerItem *player) override;

private:
    int m_bonusHp;  ///< 增加的血量
};

/**
 * @brief 吸血特性（攻击时回复生命）
 */
class TraitLifeSteal : public Trait
{
    Q_OBJECT
public:
    TraitLifeSteal(float percent);

    /**
     * @brief 当造成伤害时调用（由 GameMainPage 在子弹命中时触发）
     * @param player 玩家对象
     * @param damage 造成的伤害值
     */
    void onDamageDealt(PlayerItem *player, int damage);

private:
    float m_percent;  ///< 吸血百分比（0.0 ~ 1.0）
};

#endif // TRAIT_H
