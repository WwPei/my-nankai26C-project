#pragma once

#include <QGraphicsView>

class BattleArenaView final : public QGraphicsView
{
public:
    explicit BattleArenaView(QGraphicsScene *scene, QWidget *parent = nullptr);

    void setGridVisible(bool visible);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    bool m_gridVisible {true};
};
