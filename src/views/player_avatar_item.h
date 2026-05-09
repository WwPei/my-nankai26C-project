#pragma once

#include <QGraphicsItem>

class PlayerAvatarItem final : public QGraphicsItem
{
public:
    explicit PlayerAvatarItem(QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;

    void setAimDirection(const QPointF &direction);
    void setHitFlash(qreal intensity);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QPointF m_aimDirection {1.0, 0.0};
    qreal m_hitFlashIntensity {0.0};
};
