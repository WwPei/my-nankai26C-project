#include "bullet.h"
#include <QBrush>
#include <QPen>

BulletView::BulletView(BulletData *data, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent), m_data(data)
{
    // 视觉设置
    setRect(0, 0, 12, 12); // 直径12
    setBrush(QBrush(Qt::white));
    setPen(QPen(Qt::NoPen));

    // 初始同步
    syncPosition();
}

void BulletView::syncPosition()
{
    // BulletData 的 pos 是中心点，GraphicsItem 的 pos 是左上角
    setPos(m_data->pos() - QPointF(6, 6));
}
