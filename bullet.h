#ifndef BULLET_H
#define BULLET_H

#include <QGraphicsEllipseItem>
#include "bullet_data.h"

class BulletView : public QGraphicsEllipseItem
{
public:
    explicit BulletView(BulletData *data, QGraphicsItem *parent = nullptr);

    void syncPosition();

private:
    BulletData *m_data;
};

#endif // BULLET_H
