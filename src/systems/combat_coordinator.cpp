#include "combat_coordinator.h"

#include "bullet.h"
#include "combat_utils.h"
#include "enemy.h"
#include "game_factory.h"
#include "player.h"
#include "special_bullet_config.h"
#include "special_bullet_data.h"
#include "special_bullet_view.h"
#include "wave_manager.h"
#include "weapon.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QLineF>

#include <algorithm>
#include <cmath>

namespace {

constexpr qreal kSpreadStepDegrees = 12.0;
constexpr double kPi = 3.14159265358979323846;
constexpr float kPlayerDamageCooldownMs = 600.0F;

[[nodiscard]] QPointF normalizedVector(const QPointF &vector, const QPointF &fallback = QPointF(1.0, 0.0))
{
    const QLineF line(QPointF(), vector);
    if (line.length() <= 0.001) {
        return fallback;
    }

    return QPointF(vector.x() / line.length(), vector.y() / line.length());
}

[[nodiscard]] QPointF rotateVector(const QPointF &vector, qreal angleDegrees)
{
    const double radians = angleDegrees * kPi / 180.0;
    const double cosValue = std::cos(radians);
    const double sinValue = std::sin(radians);
    return QPointF(vector.x() * cosValue - vector.y() * sinValue,
                   vector.x() * sinValue + vector.y() * cosValue);
}

} // namespace

CombatCoordinator::CombatCoordinator(GameFactory *factory, QGraphicsScene *scene,
                                     Player *player, Weapon *weapon,
                                     QObject *parent)
    : QObject(parent)
    , m_factory(factory)
    , m_scene(scene)
    , m_player(player)
    , m_weapon(weapon)
{
}

void CombatCoordinator::handleWeaponFireRequested(WeaponId weaponId,
                                                  const QPointF &origin,
                                                  const QPointF &direction,
                                                  QVector<GameFactory::BulletEntity> &bullets)
{
    if (m_factory == nullptr) {
        return;
    }

    const WeaponConfig *config = m_factory->weaponConfig(weaponId);
    const int projectileCount = std::max(1, config != nullptr ? config->projectileCount : 1);
    const QPointF normalizedDirectionValue = normalizedVector(direction);

    const BulletSpecialTemplate *specialTemplate = nullptr;
    if (m_activeBulletStyle != BulletStyle::Normal) {
        specialTemplate = BulletTemplateConfig::findBulletTemplate(m_activeBulletStyle);
    }

    auto createBulletForDirection = [this, weaponId, &origin, specialTemplate](const QPointF &dir) -> GameFactory::BulletEntity {
        if (specialTemplate != nullptr) {
            BulletSpecialConfig specialConfig;
            specialConfig.effect = specialTemplate->effect;
            specialConfig.param1 = specialTemplate->effectParam1;
            specialConfig.param2 = specialTemplate->effectParam2;
            specialConfig.emojiIcon = specialTemplate->emojiText;
            specialConfig.imagePath = specialTemplate->defaultImagePath;
            specialConfig.altImagePaths = specialTemplate->altImagePaths;

            GameFactory::BulletEntity entity;
            entity.data = m_factory->createSpecialBulletData(*specialTemplate, origin, dir, this);
            if (entity.data != nullptr) {
                entity.view = m_factory->createSpecialBulletView(entity.data);
            }
            return entity;
        }
        return m_factory->createBulletEntity(weaponId, origin, dir, this);
    };

    const bool useParallelSpread = (weaponId == WeaponId::PeaShooter);
    QVector<QPointF> baseDirections;
    baseDirections.reserve(projectileCount);

    constexpr qreal kParallelOffset = 10.0;
    const QPointF perpDirection(-normalizedDirectionValue.y(), normalizedDirectionValue.x());

    for (int projectileIndex = 0; projectileIndex < projectileCount; ++projectileIndex) {
        const qreal centeredIndex
            = static_cast<qreal>(projectileIndex) - (static_cast<qreal>(projectileCount - 1) / 2.0);
        const QPointF projectileDirection = projectileCount == 1
            ? normalizedDirectionValue
            : normalizedVector(rotateVector(normalizedDirectionValue, centeredIndex * kSpreadStepDegrees),
                               normalizedDirectionValue);
        baseDirections.push_back(projectileDirection);

        const QPointF spawnPos = useParallelSpread
            ? origin + perpDirection * centeredIndex * kParallelOffset
            : origin;

        auto bullet = createBulletForDirection(projectileDirection);
        if (bullet.data == nullptr || bullet.view == nullptr) {
            if (bullet.view != nullptr) {
                bullet.view->deleteLater();
            }
            if (bullet.data != nullptr) {
                bullet.data->deleteLater();
            }
            continue;
        }

        if (useParallelSpread) {
            bullet.data->setWorldPosition(spawnPos);
        }

        if (m_weapon != nullptr) {
            bullet.data->setPierceCount(m_weapon->pierceCount());
            bullet.data->setSizeScale(m_weapon->bulletSizeScale());
        }
        if (m_player != nullptr) {
            bullet.data->setDamageMultiplier(m_player->damageMultiplier());
        }
        if (m_weapon != nullptr && m_weapon->comboInterval() > 0) {
            if (m_weapon->comboCounter() % m_weapon->comboInterval() == 0) {
                bullet.data->setComboMultiplier(m_weapon->comboDamageMultiplier());
            }
        }

        m_scene->addItem(bullet.view);
        bullets.push_back(bullet);
    }

    if (m_weapon != nullptr) {
        const int extraCount = m_weapon->extraProjectiles();
        for (int extraIndex = 1; extraIndex <= extraCount; ++extraIndex) {
            const qreal angleOffset = (extraIndex % 2 == 1 ? 1.0 : -1.0)
                * kSpreadStepDegrees * static_cast<qreal>(extraIndex);

            for (const QPointF &baseDir : baseDirections) {
                const QPointF extraDirection = normalizedVector(rotateVector(baseDir, angleOffset),
                                                               baseDir);

                auto bullet = createBulletForDirection(extraDirection);
                if (bullet.data == nullptr || bullet.view == nullptr) {
                    if (bullet.view != nullptr) {
                        bullet.view->deleteLater();
                    }
                    if (bullet.data != nullptr) {
                        bullet.data->deleteLater();
                    }
                    continue;
                }

                bullet.data->setPierceCount(m_weapon->pierceCount());
                bullet.data->setSizeScale(m_weapon->bulletSizeScale());
                if (m_player != nullptr) {
                    bullet.data->setDamageMultiplier(m_player->damageMultiplier());
                }
                if (m_weapon->comboInterval() > 0) {
                    if (m_weapon->comboCounter() % m_weapon->comboInterval() == 0) {
                        bullet.data->setComboMultiplier(m_weapon->comboDamageMultiplier());
                    }
                }

                m_scene->addItem(bullet.view);
                bullets.push_back(bullet);
            }
        }
    }
}

void CombatCoordinator::setActiveBulletStyle(BulletStyle style)
{
    m_activeBulletStyle = style;
}

BulletStyle CombatCoordinator::activeBulletStyle() const noexcept
{
    return m_activeBulletStyle;
}

void CombatCoordinator::resolveCombatCollisions(QVector<GameFactory::BulletEntity> &bullets,
                                                QVector<GameFactory::EnemyEntity> &enemies,
                                                QGraphicsEllipseItem *playerMarker,
                                                float &playerDamageCooldownRemainingMs,
                                                const QMap<TraitId, int> &traitCounts)
{
    for (const auto &bullet : std::as_const(bullets)) {
        if (bullet.data == nullptr || bullet.view == nullptr || bullet.data->isExpired()) {
            continue;
        }
        if (bullet.data->isEnemyBullet()) {
            continue;
        }

        const auto collidingItems = bullet.view->collidingItems();
        for (QGraphicsItem *item : collidingItems) {
            auto *enemyView = qobject_cast<EnemyView *>(item->toGraphicsObject());
            if (enemyView == nullptr || enemyView->model() == nullptr || enemyView->model()->isDefeated()) {
                continue;
            }

            if (traitCounts.contains(TraitId::Frostbite) && traitCounts.value(TraitId::Frostbite) > 0) {
                const TraitConfig *frostbiteConfig = GameConfig::findTraitConfig(TraitId::Frostbite);
                if (frostbiteConfig != nullptr) {
                    enemyView->model()->applySlow(frostbiteConfig->slowFactor, frostbiteConfig->slowDuration);
                }
            }

            enemyView->model()->receiveDamage(bullet.data->damage(),
                                             damageVisualTypeForWeapon(bullet.data->weaponId()));

            if (bullet.data->remainingPierceCount() > 0) {
                bullet.data->setPierceCount(bullet.data->remainingPierceCount() - 1);
            } else {
                bullet.data->expire();
                break;
            }
        }
    }

    for (const auto &bullet : std::as_const(bullets)) {
        if (bullet.data == nullptr || bullet.view == nullptr || bullet.data->isExpired()) continue;
        if (!bullet.data->isEnemyBullet()) continue;
        if (playerMarker != nullptr && m_player != nullptr
            && bullet.view->collidesWithItem(playerMarker)) {
            m_player->receiveDamage(bullet.data->damage());
            bullet.data->expire();
        }
    }

    if (m_player == nullptr || playerMarker == nullptr) {
        return;
    }

    if (playerDamageCooldownRemainingMs > 0.0F) {
        return;
    }

    float maxDamage = 0.0F;
    for (const auto &enemy : std::as_const(enemies)) {
        if (enemy.data == nullptr || enemy.view == nullptr || enemy.data->isDefeated()) {
            continue;
        }

        if (!enemy.view->collidesWithItem(playerMarker)) {
            continue;
        }
        maxDamage = std::max(maxDamage, enemy.data->contactDamage());
    }
    if (maxDamage > 0.0F) {
        m_player->receiveDamage(maxDamage);
        playerDamageCooldownRemainingMs = kPlayerDamageCooldownMs;
    }
}

void CombatCoordinator::cleanupExpiredBullets(QVector<GameFactory::BulletEntity> &bullets)
{
    for (qsizetype index = bullets.size() - 1; index >= 0; --index) {
        const auto &bullet = bullets.at(index);
        if (bullet.data != nullptr && !bullet.data->isExpired()) {
            continue;
        }

        if (bullet.view != nullptr) {
            m_scene->removeItem(bullet.view);
            bullet.view->deleteLater();
        }
        if (bullet.data != nullptr) {
            bullet.data->deleteLater();
        }
        bullets.removeAt(index);
    }
}

void CombatCoordinator::cleanupDefeatedEnemies(QVector<GameFactory::EnemyEntity> &enemies,
                                               WaveManager *wm,
                                               const QMap<TraitId, int> &traitCounts)
{
    for (qsizetype index = enemies.size() - 1; index >= 0; --index) {
        const auto &enemy = enemies.at(index);
        if (enemy.data != nullptr && !enemy.data->isDefeated()) {
            continue;
        }

        if (enemy.data != nullptr && wm != nullptr) {
            int expAmount = static_cast<int>(static_cast<float>(GameConfig::kWaveConfig.experiencePerEnemyDefeat)
                * GameConfig::waveExpMultiplier(wm->currentRound()));
            if (m_player != nullptr) {
                expAmount = static_cast<int>(static_cast<float>(expAmount) * m_player->expMultiplier());
            }
            wm->addExperience(expAmount);

            if (traitCounts.contains(TraitId::VampiricAura) && m_player != nullptr) {
                const int vampLevel = traitCounts.value(TraitId::VampiricAura);
                const TraitConfig *vampConfig = GameConfig::findTraitConfig(TraitId::VampiricAura);
                if (vampConfig != nullptr && vampLevel > 0) {
                    const float healAmount = m_player->maxHealth() * vampConfig->healOnKillPercent * static_cast<float>(vampLevel);
                    m_player->heal(healAmount);
                }
            }

            if (traitCounts.contains(TraitId::Adrenaline) && m_player != nullptr) {
                const int adrLevel = traitCounts.value(TraitId::Adrenaline);
                const TraitConfig *adrConfig = GameConfig::findTraitConfig(TraitId::Adrenaline);
                if (adrConfig != nullptr && adrLevel > 0) {
                    m_player->applySpeedBuff(1.0F + adrConfig->killSpeedBuff * static_cast<float>(adrLevel),
                                             adrConfig->killSpeedBuffDuration);
                }
            }
        }

        if (enemy.view != nullptr) {
            m_scene->removeItem(enemy.view);
            enemy.view->deleteLater();
        }
        if (enemy.data != nullptr) {
            enemy.data->deleteLater();
        }
        enemies.removeAt(index);
    }
}
