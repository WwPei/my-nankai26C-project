#pragma once

#include <QGraphicsItem>
#include <QPointer>

#include "bullet.h"

class BasicBulletView final : public BulletView
{
    Q_OBJECT

public:
    explicit BasicBulletView(QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    [[nodiscard]] QPainterPath shape() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    [[nodiscard]] BulletData *model() const override;

public slots:
    void bindModel(BulletData *data) override;
    void syncFromData() override;

private:
    QPointer<BulletData> m_model;
    qreal m_radius {6.0};
};
