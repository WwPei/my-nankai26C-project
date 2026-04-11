#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsRectItem>
#include "enemy_data.h"
#include <QVector>
#include <QMetaObject>
class EnemyView : public QGraphicsRectItem
{
public:
    explicit EnemyView(EnemyData *data, QGraphicsItem *parent = nullptr);
   ~EnemyView();  // ⬅ 添加析构函数
    // 手动同步接口（也可以全用信号槽，手动同步性能更好）
    void syncPosition();
    void syncHealthBar();

private:
    void setupUI();
    void onHurt(); // 处理受伤闪烁

    EnemyData *m_data;
    QGraphicsRectItem *m_healthBarBg;
    QGraphicsRectItem *m_healthBarFg;
    QVector<QMetaObject::Connection> m_connections;
};

#endif // ENEMY_H
