#pragma once

#include <QGraphicsItem>
#include <QPixmap>

class PlayerAvatarItem final : public QGraphicsItem
{
public:
    explicit PlayerAvatarItem(QGraphicsItem *parent = nullptr);

    [[nodiscard]] QRectF boundingRect() const override;

    void setAimDirection(const QPointF &direction);
    void setAimArrow(const QString &resourcePath);
    void setHitFlash(qreal intensity);
    void setHealthRatio(qreal ratio);
    void setIsDashing(bool dashing);
    void setIsHurt(bool hurt);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    [[nodiscard]] QPixmap currentFacePixmap() const;
    void loadImage(const QString &path, QPixmap &target);

    QPointF m_aimDirection {1.0, 0.0};
    qreal m_hitFlashIntensity {0.0};
    qreal m_healthRatio {1.0};
    bool m_isDashing {false};
    bool m_isHurt {false};

    QPixmap m_faceNormal;
    QPixmap m_faceMid;
    QPixmap m_faceHurt;
    QPixmap m_faceDash;
    QPixmap m_faceLowHp;
    QPixmap m_faceFull;
    QPixmap m_aimArrow;
    bool m_imagesLoaded {false};
    qreal m_renderRadius {24.0};
};