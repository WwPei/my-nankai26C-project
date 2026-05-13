#pragma once

#include <QGraphicsView>
#include <QPixmap>

class BattleArenaView final : public QGraphicsView
{
public:
    explicit BattleArenaView(QGraphicsScene *scene, QWidget *parent = nullptr);

    void setGridVisible(bool visible);
    void setBackgroundImage(const QPixmap &pixmap);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    bool m_gridVisible {false};
    QPixmap m_backgroundImage;
};
