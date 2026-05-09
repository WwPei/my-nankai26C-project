#pragma once

#include "bullet.h"
#include <QPainterPath>
#include <QPixmap>
#include <QPointer>
#include <QString>

class SpecialBulletView final : public BulletView {
    Q_OBJECT
public:
    explicit SpecialBulletView(QGraphicsItem *parent = nullptr);
    [[nodiscard]] QRectF boundingRect() const override;
    [[nodiscard]] QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    [[nodiscard]] BulletData *model() const override;
public slots:
    void bindModel(BulletData *data) override;
    void syncFromData() override;
private:
    QPointer<BulletData> m_model;
    qreal m_radius{6.0};
    QPixmap m_iconPixmap;
};
