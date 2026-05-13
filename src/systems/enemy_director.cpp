#include "enemy_director.h"

#include "bullet.h"
#include "enemy.h"
#include "game_factory.h"
#include "wave_manager.h"

#include <QFrame>
#include <QGraphicsScene>
#include <QLabel>
#include <QProgressBar>
#include <QRandomGenerator>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <cmath>
#include <utility>

namespace {

constexpr qreal kEnemySpawnInset = 24.0;

[[nodiscard]] qreal randomBetween(qreal minimum, qreal maximum)
{
    return minimum + (maximum - minimum) * QRandomGenerator::global()->generateDouble();
}

} // namespace

EnemyDirector::EnemyDirector(GameFactory *factory, QGraphicsScene *scene,
                             QWidget *parentWidget,
                             QObject *parent)
    : QObject(parent)
    , m_factory(factory)
    , m_scene(scene)
    , m_parentWidget(parentWidget)
{
}

void EnemyDirector::setStateVectors(QVector<GameFactory::EnemyEntity> *enemies,
                                    QVector<GameFactory::BulletEntity> *bullets)
{
    m_enemies = enemies;
    m_bullets = bullets;
}

void EnemyDirector::connectEnemyShootSignals(EnemyData *enemyData)
{
    QObject::connect(enemyData, &EnemyData::requestShoot, this,
        [this](QPointF direction, int bulletCount, float bulletSpeed, float bulletDamage) {
            auto *ed = qobject_cast<EnemyData *>(sender());
            if (ed == nullptr || m_factory == nullptr || m_bullets == nullptr) return;
            const QPointF origin = ed->worldPosition();
            const auto *config = ed->config();
            const bool isBoss = (config != nullptr && config->behavior == EnemyBehavior::Boss);
            for (int i = 0; i < bulletCount; ++i) {
                QPointF bulletDir;
                if (isBoss) {
                    const float angle = (2.0F * 3.14159265F * i) / static_cast<float>(bulletCount);
                    bulletDir = QPointF(std::cos(angle), std::sin(angle));
                } else {
                    const float angleOffset = (i - (bulletCount - 1) / 2.0F) * 0.15F;
                    const float cosA = std::cos(angleOffset);
                    const float sinA = std::sin(angleOffset);
                    bulletDir = QPointF(direction.x() * cosA - direction.y() * sinA,
                                       direction.x() * sinA + direction.y() * cosA);
                }
                auto bulletEntity = m_factory->createBulletEntity(
                    WeaponId::PeaShooter, origin, bulletDir, this);
                if (bulletEntity.data != nullptr) {
                    bulletEntity.data->setEnemyBullet(true);
                    bulletEntity.data->setSpeed(bulletSpeed);
                    const float baseDamage = std::max(1.0F, bulletEntity.data->damage());
                    bulletEntity.data->setDamageMultiplier(bulletDamage / baseDamage);
                }
                if (bulletEntity.view != nullptr && bulletEntity.data != nullptr) {
                    m_scene->addItem(bulletEntity.view);
                    m_bullets->append(bulletEntity);
                }
            }
        });
}

void EnemyDirector::spawnTestEnemy(int maxConcurrent, bool bossIsActive, EnemyId currentBossId)
{
    if (m_factory == nullptr || m_enemies == nullptr
        || m_enemies->size() >= maxConcurrent) {
        return;
    }

    EnemyId enemyId;
    if (bossIsActive) {
        switch (currentBossId) {
        case EnemyId::DemonLord:
            enemyId = (QRandomGenerator::global()->bounded(2) == 0) ? EnemyId::Ogre : EnemyId::Jester;
            break;
        case EnemyId::BoneLord:
            enemyId = (QRandomGenerator::global()->bounded(2) == 0) ? EnemyId::SkeletonNew : EnemyId::Ghost;
            break;
        case EnemyId::UFO:
        case EnemyId::AlienPilot:
            enemyId = (QRandomGenerator::global()->bounded(2) == 0) ? EnemyId::Robot : EnemyId::XenoBeast;
            break;
        default:
            enemyId = randomEnemyId();
            break;
        }
    } else {
        enemyId = randomEnemyId();
    }

    auto enemy = m_factory->createEnemyEntity(enemyId, randomEnemySpawnPosition(), this);
    if (enemy.data == nullptr || enemy.view == nullptr) {
        if (enemy.view != nullptr) {
            enemy.view->deleteLater();
        }
        if (enemy.data != nullptr) {
            enemy.data->deleteLater();
        }
        return;
    }

    connectEnemyShootSignals(enemy.data);

    m_scene->addItem(enemy.view);
    m_enemies->push_back(enemy);
}

void EnemyDirector::spawnBossIfPending(WaveManager *wm, bool &bossIsActive, EnemyId &currentBossId)
{
    if (wm == nullptr || m_factory == nullptr || m_scene == nullptr
        || m_enemies == nullptr || m_bullets == nullptr) {
        return;
    }

    const QList<EnemyId> bosses = wm->pendingBosses();
    for (const EnemyId bossId : bosses) {
        if (bossId == EnemyId::AlienPilot) {
            QTimer::singleShot(1000, this, [this, &bossIsActive, &currentBossId]() {
                if (m_factory == nullptr || m_scene == nullptr
                    || m_enemies == nullptr || m_bullets == nullptr) {
                    return;
                }
                const QPointF center = m_scene->sceneRect().center();
                auto entity = m_factory->createEnemyEntity(EnemyId::AlienPilot, center, this);
                if (entity.data == nullptr || entity.view == nullptr) {
                    if (entity.view != nullptr) entity.view->deleteLater();
                    if (entity.data != nullptr) entity.data->deleteLater();
                    return;
                }
                m_scene->addItem(entity.view);
                m_enemies->push_back(entity);
                onBossEntityCreated(entity);
                bossIsActive = true;
                currentBossId = EnemyId::AlienPilot;
            });
            continue;
        }

        const QPointF center = m_scene->sceneRect().center();
        auto entity = m_factory->createEnemyEntity(bossId, center, this);
        if (entity.data == nullptr || entity.view == nullptr) {
            if (entity.view != nullptr) entity.view->deleteLater();
            if (entity.data != nullptr) entity.data->deleteLater();
            continue;
        }
        m_scene->addItem(entity.view);
        m_enemies->push_back(entity);
        onBossEntityCreated(entity);
        bossIsActive = true;
        currentBossId = bossId;

        EnemyId minionId;
        switch (bossId) {
        case EnemyId::DemonLord:
            minionId = EnemyId::Ogre;
            break;
        case EnemyId::BoneLord:
            minionId = EnemyId::SkeletonNew;
            break;
        case EnemyId::UFO:
        case EnemyId::AlienPilot:
            minionId = EnemyId::Robot;
            break;
        default:
            minionId = EnemyId::Ogre;
            break;
        }

        for (int i = 0; i < 4; ++i) {
            const float angle = (static_cast<float>(i) / 4.0F) * 2.0F * 3.14159265F;
            const QPointF offset(std::cos(angle) * 150.0F, std::sin(angle) * 150.0F);
            auto minion = m_factory->createEnemyEntity(minionId, center + offset, this);
            if (minion.data == nullptr || minion.view == nullptr) {
                if (minion.view != nullptr) minion.view->deleteLater();
                if (minion.data != nullptr) minion.data->deleteLater();
                continue;
            }
            connectEnemyShootSignals(minion.data);
            m_scene->addItem(minion.view);
            m_enemies->push_back(minion);
        }
    }
}

void EnemyDirector::onBossEntityCreated(GameFactory::EnemyEntity entity)
{
    if (entity.data == nullptr) {
        return;
    }

    const EnemyId bossId = entity.data->id();
    const EnemyConfig *config = m_factory != nullptr ? m_factory->enemyConfig(bossId) : nullptr;
    const QString bossName = config != nullptr ? config->displayName : QStringLiteral("Boss");

    QObject::connect(entity.data, &EnemyData::healthChanged, this, [this](float, float) {
        auto *boss = qobject_cast<EnemyData *>(sender());
        if (boss != nullptr) {
            updateBossHealthBar(boss, m_bossHpLabel, m_bossHpBar);
        }
    });

    QObject::connect(entity.data, &EnemyData::defeated, this, [this]() {
        hideBossHealthBar(m_bossHpPanel, m_bossHpLabel, m_bossHpBar);
    });

    connectEnemyShootSignals(entity.data);

    QObject::connect(entity.data, &EnemyData::requestSpawnMinion, this, [this, bossId]() {
        if (m_factory == nullptr || m_scene == nullptr
            || m_enemies == nullptr || m_bullets == nullptr) return;
        auto *enemyData = qobject_cast<EnemyData *>(sender());
        if (enemyData == nullptr) return;

        EnemyId minionId;
        switch (bossId) {
        case EnemyId::DemonLord:
            minionId = (QRandomGenerator::global()->bounded(2) == 0) ? EnemyId::Ogre : EnemyId::Jester;
            break;
        case EnemyId::BoneLord:
            minionId = (QRandomGenerator::global()->bounded(2) == 0) ? EnemyId::SkeletonNew : EnemyId::Ghost;
            break;
        case EnemyId::UFO:
        case EnemyId::AlienPilot:
            minionId = (QRandomGenerator::global()->bounded(2) == 0) ? EnemyId::Robot : EnemyId::XenoBeast;
            break;
        default:
            minionId = randomEnemyId();
            break;
        }

        const QPointF bossPos = enemyData->worldPosition();
        const float angle = static_cast<float>(QRandomGenerator::global()->bounded(360)) * 3.14159265F / 180.0F;
        const QPointF spawnPos(bossPos.x() + std::cos(angle) * 200.0F,
                               bossPos.y() + std::sin(angle) * 200.0F);

        auto minion = m_factory->createEnemyEntity(minionId, spawnPos, this);
        if (minion.data == nullptr || minion.view == nullptr) {
            if (minion.view != nullptr) minion.view->deleteLater();
            if (minion.data != nullptr) minion.data->deleteLater();
            return;
        }

        connectEnemyShootSignals(minion.data);

        m_scene->addItem(minion.view);
        m_enemies->push_back(minion);
    });

    showBossHealthBar(bossName, m_bossHpPanel, m_bossHpLabel, m_bossHpBar);
}

void EnemyDirector::updateBossHealthBar(EnemyData *boss, QLabel *hpLabel, QProgressBar *hpBar)
{
    if (hpBar == nullptr || boss == nullptr) return;
    const float ratio = boss->currentHealth() / std::max(1.0F, boss->maxHealth());
    hpBar->setValue(static_cast<int>(ratio * 100));
    if (boss->isDefeated()) {
        hideBossHealthBar(m_bossHpPanel, m_bossHpLabel, m_bossHpBar);
    }
}

void EnemyDirector::showBossHealthBar(const QString &name, QFrame *&panel,
                                      QLabel *&label, QProgressBar *&bar)
{
    if (panel != nullptr) return;

    panel = new QFrame(m_parentWidget);
    panel->setObjectName(QStringLiteral("bossHpPanel"));
    panel->setStyleSheet(QStringLiteral(
        "QFrame#bossHpPanel { background: rgba(20, 5, 5, 0.9); border: 2px solid #8b0000; border-radius: 8px; padding: 6px 16px; }"
        "QLabel { color: white; font-size: 14px; font-weight: bold; }"
        "QProgressBar { border: 1px solid #8b0000; border-radius: 4px; background: #1a0000; height: 14px; text-align: center; }"
        "QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #ff4444, stop:1 #8b0000); border-radius: 3px; }"
    ));

    auto *bossLayout = new QVBoxLayout(panel);
    bossLayout->setContentsMargins(8, 4, 8, 4);
    bossLayout->setSpacing(2);

    label = new QLabel(name, panel);
    label->setAlignment(Qt::AlignCenter);
    bossLayout->addWidget(label);

    bar = new QProgressBar(panel);
    bar->setMinimum(0);
    bar->setMaximum(100);
    bar->setValue(100);
    bar->setTextVisible(true);
    bar->setFormat(QStringLiteral("%v / %m"));
    bossLayout->addWidget(bar);

    panel->adjustSize();
    const QRect parentRect = m_parentWidget != nullptr ? m_parentWidget->rect() : QRect();
    constexpr int panelWidth = 300;
    const int panelX = (parentRect.width() - panelWidth) / 2;
    panel->setGeometry(panelX, 10, panelWidth, panel->sizeHint().height());
    panel->show();
}

void EnemyDirector::hideBossHealthBar(QFrame *&panel, QLabel *&label, QProgressBar *&bar)
{
    if (panel != nullptr) {
        panel->hide();
        panel->deleteLater();
        panel = nullptr;
        label = nullptr;
        bar = nullptr;
    }
}

void EnemyDirector::positionBossBarFixed(int x, int y)
{
    if (m_bossHpPanel == nullptr) return;
    m_bossHpPanel->move(x, y);
}

QPointF EnemyDirector::randomEnemySpawnPosition() const
{
    if (m_scene == nullptr) {
        return QPointF();
    }

    const QRectF rect = m_scene->sceneRect().adjusted(kEnemySpawnInset, kEnemySpawnInset,
                                                       -kEnemySpawnInset, -kEnemySpawnInset);
    const int edgeIndex = QRandomGenerator::global()->bounded(4);
    switch (edgeIndex) {
    case 0:
        return QPointF(rect.left(), randomBetween(rect.top(), rect.bottom()));
    case 1:
        return QPointF(rect.right(), randomBetween(rect.top(), rect.bottom()));
    case 2:
        return QPointF(randomBetween(rect.left(), rect.right()), rect.top());
    default:
        return QPointF(randomBetween(rect.left(), rect.right()), rect.bottom());
    }
}

EnemyId EnemyDirector::randomEnemyId()
{
    static const QList<EnemyId> nonBossEnemies = {
        EnemyId::Ogre, EnemyId::Jester,
        EnemyId::SkeletonNew, EnemyId::Ghost,
        EnemyId::Robot, EnemyId::XenoBeast
    };
    return nonBossEnemies.at(QRandomGenerator::global()->bounded(nonBossEnemies.size()));
}
