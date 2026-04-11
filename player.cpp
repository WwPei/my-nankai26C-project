#include "player.h"  // IWYU pragma: keep
#include "weapon.h"  // IWYU pragma: keep
#include <QPen>
#include <QBrush>
#include <QGraphicsPolygonItem>  // Qt6：箭头形状
#include <QPolygonF>               // Qt6：多边形定义
#include <QtMath>                  // Qt6：atan2, M_PI

PlayerItem::PlayerItem(qreal w, qreal h)
    : QGraphicsRectItem(0, 0, w, h)
    , m_leftWeapon(nullptr)
    , m_rightWeapon(nullptr)
    , m_leftWeaponArrow(nullptr)
    , m_rightWeaponArrow(nullptr)
    , m_width(w)
    , m_height(h)
{
    setBrush(Qt::green);
    setPen(Qt::NoPen);
    setFlag(QGraphicsItem::ItemIsFocusable);
    // 关键：设置玩家矩形的变换原点为中心（便于后续扩展）
    setTransformOriginPoint(w / 2, h / 2);

    // 创建箭头形状（原点在几何中心）
    QPolygonF arrowShape = createArrowShape();

    // 左手武器箭头（蓝色，位于玩家左侧）
    // 使用 QGraphicsPolygonItem 替代圆形，支持旋转指向鼠标
    m_leftWeaponArrow = new QGraphicsPolygonItem(arrowShape, this);
    m_leftWeaponArrow->setBrush(Qt::blue);
    m_leftWeaponArrow->setPen(Qt::NoPen);
    m_leftWeaponArrow->setPos(-45, 0);  // 初始左侧位置
    // 关键：设置旋转原点为箭头中心（0,0就是中心）
    m_leftWeaponArrow->setTransformOriginPoint(0, 0);
    // 初始旋转0度（指向右），后续由 updateWeaponArrows 更新

    // 右手武器箭头（红色，位于玩家右侧）
    m_rightWeaponArrow = new QGraphicsPolygonItem(arrowShape, this);
    m_rightWeaponArrow->setBrush(Qt::red);
    m_rightWeaponArrow->setPen(Qt::NoPen);
     m_rightWeaponArrow->setTransformOriginPoint(0, 0);
    m_rightWeaponArrow->setPos(45, 0);  // 初始右侧位置
}
// 创建箭头形状：原点在中心，尖端朝右（0度）
// 形状：从中心向左延伸尾部，向右延伸尖端
QPolygonF PlayerItem::createArrowShape()
{
    QPolygonF shape;
    // 中心点在 (0,0)
    shape << QPointF(15, 0)     // 尖端（右侧）
          << QPointF(-5, -8)    // 左上
          << QPointF(-10, 0)    // 尾部凹陷
          << QPointF(-5, 8);    // 左下
    return shape;
}
void PlayerItem::setNormalColor()
{
    setBrush(Qt::green);
}

void PlayerItem::setHurtColor()
{
    setBrush(Qt::red);
}

void PlayerItem::setPixmap(const QPixmap &pix)
{
    Q_UNUSED(pix);
    // 预留：可替换为 QGraphicsPixmapItem
}

void PlayerItem::setLeftWeapon(Weapon *weapon)
{
    m_leftWeapon = weapon;
    if (weapon && m_leftWeaponArrow) {
        m_leftWeaponArrow->setBrush(weapon->color());
    }
}

void PlayerItem::setRightWeapon(Weapon *weapon)
{
    m_rightWeapon = weapon;
    if (weapon && m_rightWeaponArrow) {
        m_rightWeaponArrow->setBrush(weapon->color());
    }
}

Weapon* PlayerItem::leftWeapon() const
{
    return m_leftWeapon;
}

Weapon* PlayerItem::rightWeapon() const
{
    return m_rightWeapon;
}
// 获取玩家中心（场景坐标）
QPointF PlayerItem::getCenter() const
{
    // pos() 是左上角，加上半宽高得到中心
    return pos() + QPointF(m_width / 2, m_height / 2);
}

void PlayerItem::updateWeaponArrows(const QPointF &aimDir)
{
    if (aimDir.manhattanLength() < 0.1)
        return;

    // 计算旋转角度（箭头默认朝右，与aimDir角度一致）
    qreal angle = std::atan2(aimDir.y(), aimDir.x()) * 180.0 / M_PI;

    // 武器与玩家的间隔距离
    const qreal weaponOffset = 45.0;

    // 右手武器：位于玩家右侧（aimDir方向）
    // 位置 = 玩家中心 + aimDir * offset，但相对于玩家本地坐标
    // 由于武器是玩家的子项，pos是相对于玩家的
    // 我们需要武器始终位于玩家的"右侧"（相对于瞄准方向）

    // 新方案：左右武器固定位于瞄准方向的垂直两侧
    // 右手在 aimDir 的右侧，左手在左侧？不，这样不合理
    // 改为：右手在 aimDir 方向，左手在反方向？也不对

    // 正确方案：左右手分别位于玩家中心的两侧，垂直于aimDir
    QPointF perp(-aimDir.y(), aimDir.x());  // 垂直向量（左侧）

    // 右手：玩家中心右侧（perp的反方向 = 右侧）
    QPointF rightOffset = -perp * weaponOffset;
    m_rightWeaponArrow->setPos(rightOffset);
    m_rightWeaponArrow->setRotation(angle);

    // 左手：玩家中心左侧（perp方向 = 左侧）
    QPointF leftOffset = perp * weaponOffset;
    m_leftWeaponArrow->setPos(leftOffset);
    m_leftWeaponArrow->setRotation(angle);

    // 效果：无论鼠标在哪，左手始终在玩家左侧，右手在右侧
    // 但两者都旋转指向鼠标
}

// 获取左手武器发射位置（箭头尖端，场景坐标）
QPointF PlayerItem::getLeftWeaponMuzzle() const
{
    if (!m_leftWeaponArrow)
        return getCenter();

    // 箭头尖端在本地坐标系中是 (15, 0)（相对于箭头中心）
    // 需要转换为场景坐标
    QPointF localMuzzle(15, 0);  // 尖端位置
    return m_leftWeaponArrow->mapToScene(localMuzzle);
}

// 获取右手武器发射位置（箭头尖端，场景坐标）
QPointF PlayerItem::getRightWeaponMuzzle() const
{
    if (!m_rightWeaponArrow)
        return getCenter();

    QPointF localMuzzle(15, 0);
    return m_rightWeaponArrow->mapToScene(localMuzzle);
}
