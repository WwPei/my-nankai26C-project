#include "game_factory.h"

#include "bullet.h"
#include "enemy.h"
#include "player.h"
#include "trait.h"
#include "weapon.h"

#include <QGraphicsItem>
#include <QLineF>
#include <QPainter>
#include <QPointer>

#include <algorithm>
#include <cmath>

namespace {

[[nodiscard]] QPointF normalizedDirection(const QPointF &direction, const QPointF &fallback = QPointF(1.0, 0.0))
{
    const QLineF line(QPointF(), direction);
    if (line.length() <= 0.001) {
        return fallback;
    }

    return QPointF(direction.x() / line.length(), direction.y() / line.length());
}

[[nodiscard]] QColor enemyBaseColor(EnemyId id)
{
    switch (id) {
    case EnemyId::Slime:
        return QColor(QStringLiteral("#64d86d"));
    case EnemyId::Bat:
        return QColor(QStringLiteral("#ff875f"));
    case EnemyId::Skeleton:
        return QColor(QStringLiteral("#eceff4"));
    }

    return QColor(QStringLiteral("#d96b6b"));
}

[[nodiscard]] QColor bulletBaseColor(WeaponId id)
{
    switch (id) {
    case WeaponId::PeaShooter:
        return QColor(QStringLiteral("#f2de59"));
    case WeaponId::SpreadBlaster:
        return QColor(QStringLiteral("#ff934f"));
    case WeaponId::ArcWand:
        return QColor(QStringLiteral("#8d74ff"));
    }

    return QColor(QStringLiteral("#f2de59"));
}

class BasicPlayer final : public Player
{
    Q_OBJECT

public:
    explicit BasicPlayer(const PlayerClassConfig *config, QObject *parent = nullptr)
        : Player(parent)
        , m_config(config)
        , m_currentHealth(config != nullptr ? config->maxHealth : 0.0F)
        , m_weaponId(config != nullptr ? config->starterWeaponId : WeaponId::PeaShooter)
    {
    }

    [[nodiscard]] PlayerClassId classId() const override
    {
        return m_config != nullptr ? m_config->id : PlayerClassId::Warrior;
    }

    [[nodiscard]] QPointF worldPosition() const override
    {
        return m_worldPosition;
    }

    [[nodiscard]] float currentHealth() const override
    {
        return m_currentHealth;
    }

    [[nodiscard]] float maxHealth() const override
    {
        return m_config != nullptr ? m_config->maxHealth : 0.0F;
    }

    [[nodiscard]] float moveSpeed() const override
    {
        return m_config != nullptr ? m_config->moveSpeed * m_speedMultiplier : 0.0F;
    }

    [[nodiscard]] WeaponId weaponId() const override
    {
        return m_weaponId;
    }

public slots:
    void setWorldPosition(const QPointF &position) override
    {
        if (m_worldPosition == position) {
            return;
        }

        m_worldPosition = position;
        emit moved(m_worldPosition);
    }

    void setMoveDirection(const QPointF &direction) override
    {
        m_moveDirection = direction;
    }

    void equipWeapon(WeaponId weaponId) override
    {
        if (m_weaponId == weaponId) {
            return;
        }

        m_weaponId = weaponId;
        emit weaponChanged(m_weaponId);
    }

    void applyTrait(TraitId traitId) override
    {
        const auto *config = GameConfig::findTraitConfig(traitId);
        if (config == nullptr) {
            return;
        }

        m_damageMultiplier *= config->damageMultiplier;
        m_defenseMultiplier *= config->defenseMultiplier;
        m_speedMultiplier *= config->speedMultiplier;
    }

    void receiveDamage(float amount) override
    {
        if (amount <= 0.0F || m_currentHealth <= 0.0F) {
            return;
        }

        const float actualDamage = std::max(0.0F, amount / std::max(0.1F, m_defenseMultiplier));
        m_currentHealth = std::max(0.0F, m_currentHealth - actualDamage);
        emit healthChanged(m_currentHealth, maxHealth());

        if (m_currentHealth <= 0.0F) {
            emit defeated();
        }
    }

private:
    const PlayerClassConfig *m_config {nullptr};
    QPointF m_worldPosition;
    QPointF m_moveDirection;
    float m_currentHealth {0.0F};
    float m_damageMultiplier {1.0F};
    float m_defenseMultiplier {1.0F};
    float m_speedMultiplier {1.0F};
    WeaponId m_weaponId {WeaponId::PeaShooter};
};

class BasicWeapon final : public Weapon
{
    Q_OBJECT

public:
    explicit BasicWeapon(const WeaponConfig *config, QObject *parent = nullptr)
        : Weapon(parent)
        , m_config(config)
    {
    }

    [[nodiscard]] WeaponId id() const override
    {
        return m_config != nullptr ? m_config->id : WeaponId::PeaShooter;
    }

    [[nodiscard]] const WeaponConfig *config() const override
    {
        return m_config;
    }

    [[nodiscard]] Player *owner() const override
    {
        return m_owner;
    }

    [[nodiscard]] bool isFiring() const override
    {
        return m_isFiring;
    }

public slots:
    void bindOwner(Player *owner) override
    {
        m_owner = owner;
    }

    void setAimDirection(const QPointF &direction) override
    {
        m_aimDirection = normalizedDirection(direction);
    }

    void startFiring() override
    {
        m_isFiring = true;
        m_remainingCooldownMs = 0.0F;
        emit cooldownChanged(currentIntervalMs());
    }

    void stopFiring() override
    {
        m_isFiring = false;
    }

    void advanceCooldown(float deltaMs) override
    {
        if (!m_isFiring || m_owner == nullptr || m_config == nullptr) {
            return;
        }

        m_remainingCooldownMs -= deltaMs;
        while (m_remainingCooldownMs <= 0.0F) {
            emit fireRequested(id(), m_owner->worldPosition(), m_aimDirection);
            m_remainingCooldownMs += currentIntervalMs();
        }
    }

    void applyTrait(TraitId traitId) override
    {
        const auto *config = GameConfig::findTraitConfig(traitId);
        if (config == nullptr) {
            return;
        }

        m_fireRateScale *= std::max(0.1F, config->speedMultiplier);
        emit cooldownChanged(currentIntervalMs());
    }

private:
    [[nodiscard]] float currentIntervalMs() const
    {
        if (m_config == nullptr) {
            return 1000.0F;
        }

        return m_config->fireIntervalMs / std::max(0.1F, m_fireRateScale);
    }

    const WeaponConfig *m_config {nullptr};
    QPointer<Player> m_owner;
    QPointF m_aimDirection {1.0, 0.0};
    float m_remainingCooldownMs {0.0F};
    float m_fireRateScale {1.0F};
    bool m_isFiring {false};
};

class BasicEnemyData final : public EnemyData
{
    Q_OBJECT

public:
    BasicEnemyData(const EnemyConfig *config, const QPointF &spawnPosition, QObject *parent = nullptr)
        : EnemyData(parent)
        , m_config(config)
        , m_worldPosition(spawnPosition)
        , m_targetPosition(spawnPosition)
        , m_currentHealth(config != nullptr ? config->maxHealth : 0.0F)
    {
    }

    [[nodiscard]] EnemyId id() const override
    {
        return m_config != nullptr ? m_config->id : EnemyId::Slime;
    }

    [[nodiscard]] const EnemyConfig *config() const override
    {
        return m_config;
    }

    [[nodiscard]] QPointF worldPosition() const override
    {
        return m_worldPosition;
    }

    [[nodiscard]] float currentHealth() const override
    {
        return m_currentHealth;
    }

    [[nodiscard]] float maxHealth() const override
    {
        return m_config != nullptr ? m_config->maxHealth : 0.0F;
    }

    [[nodiscard]] float moveSpeed() const override
    {
        return m_config != nullptr ? m_config->moveSpeed : 0.0F;
    }

    [[nodiscard]] float contactDamage() const override
    {
        return m_config != nullptr ? m_config->contactDamage : 0.0F;
    }

    [[nodiscard]] float collisionRadius() const override
    {
        return m_config != nullptr ? m_config->collisionRadius : 0.0F;
    }

    [[nodiscard]] bool isDefeated() const override
    {
        return m_defeated;
    }

public slots:
    void setWorldPosition(const QPointF &position) override
    {
        if (m_worldPosition == position) {
            return;
        }

        m_worldPosition = position;
        emit positionChanged(m_worldPosition);
    }

    void receiveDamage(float amount) override
    {
        if (amount <= 0.0F || m_defeated) {
            return;
        }

        m_currentHealth = std::max(0.0F, m_currentHealth - amount);
        emit healthChanged(m_currentHealth, maxHealth());
        if (m_currentHealth <= 0.0F) {
            m_defeated = true;
            emit defeated();
        }
    }

    void setTargetPosition(const QPointF &position) override
    {
        m_targetPosition = position;
    }

    void advanceFrame(float deltaSeconds) override
    {
        if (m_defeated || deltaSeconds <= 0.0F) {
            return;
        }

        const QPointF delta = m_targetPosition - m_worldPosition;
        const QLineF distanceLine(QPointF(), delta);
        if (distanceLine.length() <= 0.001) {
            return;
        }

        const float step = moveSpeed() * deltaSeconds;
        if (step >= distanceLine.length()) {
            setWorldPosition(m_targetPosition);
            return;
        }

        const QPointF direction(delta.x() / distanceLine.length(), delta.y() / distanceLine.length());
        setWorldPosition(m_worldPosition + direction * step);
    }

private:
    const EnemyConfig *m_config {nullptr};
    QPointF m_worldPosition;
    QPointF m_targetPosition;
    float m_currentHealth {0.0F};
    bool m_defeated {false};
};

class BasicBulletData final : public BulletData
{
    Q_OBJECT

public:
    BasicBulletData(WeaponId weaponId,
                    const BulletConfig *config,
                    const QPointF &spawnPosition,
                    const QPointF &direction,
                    QObject *parent = nullptr)
        : BulletData(parent)
        , m_weaponId(weaponId)
        , m_config(config)
        , m_worldPosition(spawnPosition)
        , m_direction(normalizedDirection(direction))
    {
    }

    [[nodiscard]] WeaponId weaponId() const override
    {
        return m_weaponId;
    }

    [[nodiscard]] const BulletConfig *config() const override
    {
        return m_config;
    }

    [[nodiscard]] QPointF worldPosition() const override
    {
        return m_worldPosition;
    }

    [[nodiscard]] QPointF direction() const override
    {
        return m_direction;
    }

    [[nodiscard]] float damage() const override
    {
        return m_config != nullptr ? m_config->damage : 0.0F;
    }

    [[nodiscard]] float speed() const override
    {
        return m_config != nullptr ? m_config->speed : 0.0F;
    }

    [[nodiscard]] float collisionRadius() const override
    {
        return m_config != nullptr ? m_config->collisionRadius : 0.0F;
    }

    [[nodiscard]] bool isExpired() const override
    {
        return m_expired;
    }

public slots:
    void setWorldPosition(const QPointF &position) override
    {
        if (m_worldPosition == position) {
            return;
        }

        m_worldPosition = position;
        emit positionChanged(m_worldPosition);
    }

    void setDirection(const QPointF &direction) override
    {
        m_direction = normalizedDirection(direction, m_direction);
    }

    void advanceFrame(float deltaSeconds) override
    {
        if (m_expired || deltaSeconds <= 0.0F) {
            return;
        }

        setWorldPosition(m_worldPosition + m_direction * (speed() * deltaSeconds));
        if (std::abs(m_worldPosition.x()) > 420.0 || std::abs(m_worldPosition.y()) > 260.0) {
            expire();
        }
    }

    void expire() override
    {
        if (m_expired) {
            return;
        }

        m_expired = true;
        emit expired();
    }

private:
    WeaponId m_weaponId {WeaponId::PeaShooter};
    const BulletConfig *m_config {nullptr};
    QPointF m_worldPosition;
    QPointF m_direction {1.0, 0.0};
    bool m_expired {false};
};

class BasicEnemyView final : public EnemyView
{
    Q_OBJECT

public:
    explicit BasicEnemyView(QGraphicsItem *parent = nullptr)
        : EnemyView(parent)
    {
    }

    [[nodiscard]] QRectF boundingRect() const override
    {
        return QRectF(-m_radius, -m_radius, m_radius * 2.0, m_radius * 2.0);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        const QColor baseColor = m_model != nullptr ? enemyBaseColor(m_model->id()) : QColor(224, 92, 92);
        const QColor outlineColor = baseColor.lighter(145);

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(outlineColor, 1.4));
        painter->setBrush(baseColor);
        painter->drawEllipse(boundingRect());

        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 48));
        painter->drawEllipse(QRectF(-m_radius * 0.45, -m_radius * 0.55, m_radius * 0.75, m_radius * 0.65));

        if (m_model != nullptr && m_model->id() == EnemyId::Skeleton) {
            painter->setBrush(QColor(QStringLiteral("#262a33")));
            painter->drawEllipse(QRectF(-m_radius * 0.42, -m_radius * 0.2, m_radius * 0.22, m_radius * 0.26));
            painter->drawEllipse(QRectF(m_radius * 0.2, -m_radius * 0.2, m_radius * 0.22, m_radius * 0.26));
        }
    }

    [[nodiscard]] EnemyData *model() const override
    {
        return m_model;
    }

public slots:
    void bindModel(EnemyData *data) override
    {
        if (m_model == data) {
            return;
        }

        if (m_model != nullptr) {
            disconnect(m_model, nullptr, this, nullptr);
        }

        m_model = data;
        if (m_model == nullptr) {
            return;
        }

        prepareGeometryChange();
        m_radius = m_model->collisionRadius();
        connect(m_model, &EnemyData::positionChanged, this, [this](const QPointF &) {
            syncFromData();
        });
        connect(m_model, &EnemyData::defeated, this, &GameView::removalRequested);
        syncFromData();
    }

    void syncFromData() override
    {
        if (m_model == nullptr) {
            return;
        }

        const qreal newRadius = std::max<qreal>(12.0, m_model->collisionRadius());
        if (!qFuzzyCompare(m_radius, newRadius)) {
            prepareGeometryChange();
            m_radius = newRadius;
        }
        setPos(m_model->worldPosition());
        update();
    }

private:
    QPointer<EnemyData> m_model;
    qreal m_radius {18.0};
};

class BasicBulletView final : public BulletView
{
    Q_OBJECT

public:
    explicit BasicBulletView(QGraphicsItem *parent = nullptr)
        : BulletView(parent)
    {
    }

    [[nodiscard]] QRectF boundingRect() const override
    {
        return QRectF(-m_radius, -m_radius, m_radius * 2.0, m_radius * 2.0);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        const QColor bulletColor = m_model != nullptr ? bulletBaseColor(m_model->weaponId()) : QColor(254, 226, 92);
        const QPointF direction = m_model != nullptr ? normalizedDirection(m_model->direction()) : QPointF(1.0, 0.0);
        const QPointF tail = -direction * m_radius * 2.2;
        const QPointF side(-direction.y(), direction.x());

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(bulletColor.red(), bulletColor.green(), bulletColor.blue(), 80));
        painter->drawEllipse(QRectF(tail.x() - m_radius, tail.y() - m_radius * 0.8, m_radius * 2.0, m_radius * 1.6));

        painter->setBrush(bulletColor);
        painter->drawEllipse(boundingRect());

        painter->setBrush(bulletColor.lighter(160));
        QPainterPath glowPath;
        glowPath.moveTo(direction * (m_radius * 1.8));
        glowPath.lineTo(side * (m_radius * 0.55));
        glowPath.lineTo(-side * (m_radius * 0.55));
        glowPath.closeSubpath();
        painter->drawPath(glowPath);
    }

    [[nodiscard]] BulletData *model() const override
    {
        return m_model;
    }

public slots:
    void bindModel(BulletData *data) override
    {
        if (m_model == data) {
            return;
        }

        if (m_model != nullptr) {
            disconnect(m_model, nullptr, this, nullptr);
        }

        m_model = data;
        if (m_model == nullptr) {
            return;
        }

        prepareGeometryChange();
        m_radius = m_model->collisionRadius();
        connect(m_model, &BulletData::positionChanged, this, [this](const QPointF &) {
            syncFromData();
        });
        connect(m_model, &BulletData::expired, this, &GameView::removalRequested);
        syncFromData();
    }

    void syncFromData() override
    {
        if (m_model == nullptr) {
            return;
        }

        const qreal newRadius = std::max<qreal>(4.0, m_model->collisionRadius());
        if (!qFuzzyCompare(m_radius, newRadius)) {
            prepareGeometryChange();
            m_radius = newRadius;
        }
        setPos(m_model->worldPosition());
        update();
    }

private:
    QPointer<BulletData> m_model;
    qreal m_radius {6.0};
};

class BasicTrait final : public Trait
{
    Q_OBJECT

public:
    explicit BasicTrait(const TraitConfig *config, QObject *parent = nullptr)
        : Trait(parent)
        , m_config(config)
    {
    }

    [[nodiscard]] TraitId id() const override
    {
        return m_config != nullptr ? m_config->id : TraitId::QuickHands;
    }

    [[nodiscard]] QString displayName() const override
    {
        return m_config != nullptr ? m_config->displayName : QString();
    }

    [[nodiscard]] QString description() const override
    {
        return m_config != nullptr ? m_config->summary : QString();
    }

public slots:
    void applyToPlayer(Player *player) override
    {
        if (player == nullptr || m_config == nullptr) {
            return;
        }

        player->applyTrait(m_config->id);
    }

private:
    const TraitConfig *m_config {nullptr};
};

} // namespace


GameFactory::GameFactory(QObject *parent)
    : QObject(parent)
{
}

const PlayerClassConfig *GameFactory::playerClassConfig(PlayerClassId id) const noexcept
{
    return GameConfig::findPlayerClassConfig(id);
}

const WeaponConfig *GameFactory::weaponConfig(WeaponId id) const noexcept
{
    return GameConfig::findWeaponConfig(id);
}

const BulletConfig *GameFactory::bulletConfig(WeaponId weaponId) const noexcept
{
    return GameConfig::findBulletConfig(weaponId);
}

const EnemyConfig *GameFactory::enemyConfig(EnemyId id) const noexcept
{
    return GameConfig::findEnemyConfig(id);
}

const TraitConfig *GameFactory::traitConfig(TraitId id) const noexcept
{
    return GameConfig::findTraitConfig(id);
}

Player *GameFactory::createPlayer(PlayerClassId classId, QObject *parent) const
{
    const auto *config = playerClassConfig(classId);
    if (config == nullptr) {
        return nullptr;
    }

    return new BasicPlayer(config, parent);
}

Weapon *GameFactory::createStarterWeapon(PlayerClassId classId, Player *owner, QObject *parent) const
{
    const auto *classConfig = playerClassConfig(classId);
    if (classConfig == nullptr) {
        return nullptr;
    }

    const auto *config = weaponConfig(classConfig->starterWeaponId);
    if (config == nullptr) {
        return nullptr;
    }

    auto *weapon = new BasicWeapon(config, parent);
    weapon->bindOwner(owner);
    if (owner != nullptr) {
        owner->equipWeapon(config->id);
    }
    return weapon;
}

EnemyData *GameFactory::createEnemyData(EnemyId enemyId,
                                        const QPointF &spawnPosition,
                                        QObject *parent) const
{
    const auto *config = enemyConfig(enemyId);
    if (config == nullptr) {
        return nullptr;
    }

    return new BasicEnemyData(config, spawnPosition, parent);
}

EnemyView *GameFactory::createEnemyView(EnemyData *data, QGraphicsItem *parent) const
{
    if (data == nullptr) {
        return nullptr;
    }

    auto *view = new BasicEnemyView(parent);
    view->bindModel(data);
    return view;
}

GameFactory::EnemyEntity GameFactory::createEnemyEntity(EnemyId enemyId,
                                                        const QPointF &spawnPosition,
                                                        QObject *dataParent,
                                                        QGraphicsItem *viewParent) const
{
    EnemyEntity entity;
    entity.data = createEnemyData(enemyId, spawnPosition, dataParent);
    if (entity.data == nullptr) {
        return entity;
    }

    entity.view = createEnemyView(entity.data, viewParent);
    if (entity.view == nullptr) {
        entity.data->deleteLater();
        entity.data = nullptr;
    }

    return entity;
}

BulletData *GameFactory::createBulletData(WeaponId weaponId,
                                          const QPointF &spawnPosition,
                                          const QPointF &direction,
                                          QObject *parent) const
{
    const auto *config = bulletConfig(weaponId);
    if (config == nullptr) {
        return nullptr;
    }

    return new BasicBulletData(weaponId, config, spawnPosition, direction, parent);
}

BulletView *GameFactory::createBulletView(BulletData *data, QGraphicsItem *parent) const
{
    if (data == nullptr) {
        return nullptr;
    }

    auto *view = new BasicBulletView(parent);
    view->bindModel(data);
    return view;
}

GameFactory::BulletEntity GameFactory::createBulletEntity(WeaponId weaponId,
                                                          const QPointF &spawnPosition,
                                                          const QPointF &direction,
                                                          QObject *dataParent,
                                                          QGraphicsItem *viewParent) const
{
    BulletEntity entity;
    entity.data = createBulletData(weaponId, spawnPosition, direction, dataParent);
    if (entity.data == nullptr) {
        return entity;
    }

    entity.view = createBulletView(entity.data, viewParent);
    if (entity.view == nullptr) {
        entity.data->deleteLater();
        entity.data = nullptr;
    }

    return entity;
}

Trait *GameFactory::createTrait(TraitId traitId, QObject *parent) const
{
    const auto *config = traitConfig(traitId);
    if (config == nullptr) {
        return nullptr;
    }

    return new BasicTrait(config, parent);
}

#include "game_factory.moc"
