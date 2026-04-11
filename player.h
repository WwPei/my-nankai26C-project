#ifndef PLAYER_H
#define PLAYER_H

#include <QGraphicsRectItem>
#include <QGraphicsPolygonItem>
#include <QPixmap>

class Weapon;
class QGraphicsPolygonItem;
/**
 * @brief 玩家图形项
 *
 * 显示玩家角色（默认绿色矩形）、左右手武器箭头（支持旋转指向瞄准方向）。
 * 支持受伤变色闪烁、自定义图片、武器装备与访问。
 */
class PlayerItem : public QGraphicsRectItem
{
public:
    /**
     * @brief 构造玩家
     * @param w 玩家宽度
     * @param h 玩家高度
     */
    explicit PlayerItem(qreal w, qreal h);

    // --- 外观控制 ---
    /**
     * @brief 恢复正常颜色（绿色）
     */
    void setNormalColor();

    /**
     * @brief 设置为受伤颜色（红色）
     */
    void setHurtColor();

    /**
     * @brief 设置玩家图片（预留接口）
     * @param pix 要显示的图片
     *
     * 当前为预留实现，调用后不产生效果。
     * 未来可用于替换默认矩形绘制。
     */
    void setPixmap(const QPixmap &pix);

    // --- 武器管理 ---
    /**
     * @brief 装备左手武器
     * @param weapon 武器对象指针，可为 nullptr 表示卸下
     */
    void setLeftWeapon(Weapon *weapon);

    /**
     * @brief 装备右手武器
     * @param weapon 武器对象指针，可为 nullptr 表示卸下
     */
    void setRightWeapon(Weapon *weapon);

    /**
     * @brief 获取左手当前装备的武器
     */
    Weapon* leftWeapon() const;

    /**
     * @brief 获取右手当前装备的武器
     */
    Weapon* rightWeapon() const;

    // --- 武器箭头访问（用于外部获取位置/方向）---
    /**
     * @brief 获取左手武器箭头图形项
     *
     * 可用于计算子弹发射位置或进行碰撞检测。
     */
    QGraphicsPolygonItem* leftWeaponArrow() const { return m_leftWeaponArrow; }

    /**
     * @brief 获取右手武器箭头图形项
     */
    QGraphicsPolygonItem* rightWeaponArrow() const { return m_rightWeaponArrow; }

    // --- 武器更新 ---
    /**
     * @brief 更新武器箭头的位置和旋转角度
     * @param aimDir 瞄准方向单位向量（通常为从玩家指向鼠标的方向）
     *
     * 右手武器位于玩家前方（瞄准方向），左手武器位于前方偏左位置，
     * 两者箭头均旋转指向瞄准方向。
     */
    void updateWeaponArrows(const QPointF &aimDir);
    QPointF getLeftWeaponMuzzle() const;
    QPointF getRightWeaponMuzzle() const;



    // 获取玩家中心（场景坐标）
    QPointF getCenter() const;
private:
    Weapon *m_leftWeapon;                  ///< 左手装备的武器
    Weapon *m_rightWeapon;                 ///< 右手装备的武器

    QGraphicsPolygonItem *m_leftWeaponArrow;   ///< 左手武器箭头图形项
    QGraphicsPolygonItem *m_rightWeaponArrow;  ///< 右手武器箭头图形项
    qreal m_width;
    qreal m_height;

    // 创建箭头形状，原点在中心，便于旋转
    static QPolygonF createArrowShape();
};

#endif // PLAYER_H
