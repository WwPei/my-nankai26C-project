#pragma once

#include <QGraphicsObject>

class GameView : public QGraphicsObject
{
    Q_OBJECT

public:
    using QGraphicsObject::QGraphicsObject;
    ~GameView() override = default;

public slots:
    virtual void syncFromData() = 0;

signals:
    void removalRequested();
};
