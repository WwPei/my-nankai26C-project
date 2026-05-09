#include "battle_arena_view.h"

#include <QPainter>

#include <cmath>

BattleArenaView::BattleArenaView(QGraphicsScene *scene, QWidget *parent)
    : QGraphicsView(scene, parent)
{
    setObjectName(QStringLiteral("battleArenaView"));
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::TextAntialiasing, true);
    setFrameShape(QFrame::NoFrame);
    setAlignment(Qt::AlignCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
}

void BattleArenaView::setGridVisible(bool visible)
{
    if (m_gridVisible == visible) {
        return;
    }

    m_gridVisible = visible;
    viewport()->update();
}

void BattleArenaView::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, QColor(QStringLiteral("#2a2a2a")));

    if (!m_gridVisible) {
        return;
    }

    constexpr qreal majorStep = 80.0;
    constexpr qreal minorStep = 40.0;

    QPen minorPen(QColor(58, 64, 74, 120));
    minorPen.setWidthF(1.0);
    QPen majorPen(QColor(86, 96, 112, 150));
    majorPen.setWidthF(1.0);

    const qreal left = std::floor(rect.left() / minorStep) * minorStep;
    const qreal right = std::ceil(rect.right() / minorStep) * minorStep;
    const qreal top = std::floor(rect.top() / minorStep) * minorStep;
    const qreal bottom = std::ceil(rect.bottom() / minorStep) * minorStep;

    for (qreal x = left; x <= right; x += minorStep) {
        painter->setPen(std::fmod(std::abs(x), majorStep) < 0.1 ? majorPen : minorPen);
        painter->drawLine(QLineF(x, top, x, bottom));
    }

    for (qreal y = top; y <= bottom; y += minorStep) {
        painter->setPen(std::fmod(std::abs(y), majorStep) < 0.1 ? majorPen : minorPen);
        painter->drawLine(QLineF(left, y, right, y));
    }
}
