#pragma once

#include <QGraphicsItem>
#include <QPixmap>
#include <QPointer>

#include "enemy.h"

class EnemyData;
class QTimer;

class BasicEnemyView final : public EnemyView
{
    Q_OBJECT

public:
    explicit BasicEnemyView(QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;
    [[nodiscard]] QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    [[nodiscard]] EnemyData *model() const override;

public slots:
    void bindModel(EnemyData *data) override;
    void syncFromData() override;

private:
    QPointer<EnemyData> m_model;
    QTimer *m_hitFlashTimer{nullptr};
    QPixmap m_pixmap;
    bool m_hasImage{false};
    qreal m_radius{18.0};
    qreal m_healthRatio{1.0};
    qreal m_hitFlashIntensity{0.0};
    QColor m_hitFlashColor;
};
