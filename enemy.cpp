#include "enemy.h"
#include "enemy_data.h"
#include <QBrush>
#include <QTimer>
#include <QPen>
#include <QObject>

EnemyView::EnemyView(EnemyData *data, QGraphicsItem *parent)
    : QGraphicsRectItem(parent), m_data(data)
{
    setupUI();
    syncPosition();
    syncHealthBar();

    // 【最稳妥写法】使用 QObject::connect 并完全指定模板类型
    // 注意：这里必须确保 enemy_data.h 里有 Q_OBJECT 宏
    if (m_data) {
        // 使用三参数 connect，不提供 context，保存连接用于后续断开
        m_connections.append(QObject::connect(m_data, &EnemyData::sigHurt, [this]() {
            onHurt();
        }));
        m_connections.append(QObject::connect(m_data, &EnemyData::sigHpChanged, [this]() {
            syncHealthBar();
        }));
    }
}
EnemyView::~EnemyView()
{
    for (auto &conn : m_connections)
        QObject::disconnect(conn);
}
void EnemyView::setupUI()
{
    setRect(0, 0, 40, 40);
    setBrush(QBrush(Qt::GlobalColor::yellow)); // 显式指定作用域
    setPen(QPen(Qt::PenStyle::NoPen));

    m_healthBarBg = new QGraphicsRectItem(0, 42, 40, 6, this);
    m_healthBarBg->setBrush(QBrush(Qt::GlobalColor::black));
    m_healthBarBg->setPen(QPen(Qt::PenStyle::NoPen));

    m_healthBarFg = new QGraphicsRectItem(0, 42, 40, 6, this);
    m_healthBarFg->setBrush(QBrush(Qt::GlobalColor::red));
    m_healthBarFg->setPen(QPen(Qt::PenStyle::NoPen));
}

void EnemyView::syncPosition()
{
    if (m_data) {
        setPos(m_data->pos());
    }
}

void EnemyView::syncHealthBar()
{
    if (!m_data) return;
    double percent = 0.0;
    if (m_data->maxHp() > 0) {
        percent = static_cast<double>(m_data->hp()) / static_cast<double>(m_data->maxHp());
    }
    m_healthBarFg->setRect(0, 42, 40 * percent, 6);
}

void EnemyView::onHurt()
{
    setBrush(QBrush(QColorConstants::Svg::orange));
    // Qt6 单次定时器：仅传 Lambda（省略上下文 this）
    QTimer::singleShot(100, [this]() {
        if (m_data && !m_data->isDead())
            setBrush(QBrush(Qt::GlobalColor::yellow));
    });
}
