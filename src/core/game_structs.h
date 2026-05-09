#pragma once

#include "game_enums.h"

#include <QList>
#include <QString>

struct WaveConfig {
    int roundDurationMs {30000};
    int updateIntervalMs {16};
    int maxConcurrentEnemies {15};
    int maxRounds {10};
    int initialEnemyCount {5};
    int enemySpawnIntervalMs {2200};
    int experiencePerEnemyDefeat {10};
    int upgradeSelectionCount {5};
};

struct PlayerClassConfig {
    PlayerClassId id;
    QString displayName;
    QString summary;
    WeaponId starterWeaponId;
    float maxHealth;
    float moveSpeed;
};

struct WeaponConfig {
    WeaponId id;
    QString displayName;
    QString summary;
    float baseDamage;
    float fireIntervalMs;
    float projectileSpeed;
    int projectileCount;
};

struct WeaponUpgradeConfig {
    WeaponUpgradeId id;
    QString displayName;
    QString summary;
    float extraProjectiles{0.0F};
    float rangeMultiplier{1.0F};
    int pierceCount{0};
    float bulletSizeScale{1.0F};
    int comboInterval{0};
    float comboDamageMultiplier{1.0F};
    float statMultiplier{1.0F};
};

struct BulletConfig {
    WeaponId weaponId;
    QString displayName;
    float damage;
    float speed;
    float collisionRadius;
};

struct EnemyConfig {
    EnemyId id;
    QString displayName;
    QString summary;
    float maxHealth;
    float moveSpeed;
    float contactDamage;
    float collisionRadius;
    float experienceValue{10.0F};
    QString imagePath;
    EnemyBehavior behavior{EnemyBehavior::Chase};
    float keepDistanceMin{0.0F};
    float chargeUpMs{0.0F};
    float chargeDurationMs{0.0F};
    float chargeSpeedMult{1.0F};
    float chargeCooldownMs{0.0F};
    float shootIntervalMs{0.0F};
    int shootBulletCount{0};
    float bulletSpeed{0.0F};
    float bulletDamage{0.0F};
    float shootTriggerDistance{0.0F};
    bool slowImmune{false};
    bool knockbackImmune{false};
    float suicideRadius{0.0F};
    float suicideDamage{0.0F};
};

struct TraitConfig {
    TraitId id;
    QString displayName;
    QString summary;
    float damageMultiplier;
    float defenseMultiplier;
    float speedMultiplier;
    float extraMaxHealth{0.0F};
    float criticalChance{0.0F};
    float criticalMultiplier{1.0F};
    float healOnKillPercent{0.0F};
    float expMultiplier{1.0F};
    float slowFactor{1.0F};
    float slowDuration{0.0F};
    float killSpeedBuff{0.0F};
    float killSpeedBuffDuration{0.0F};
    float bounceSearchRadius{0.0F};
    float bounceDamageRetention{0.0F};
};

struct UpgradeOption {
    QString optionId;
    UpgradeOptionKind kind {UpgradeOptionKind::Trait};
    QString displayName;
    QString summary;
    TraitId traitId {TraitId::QuickHands};
    WeaponUpgradeId weaponUpgradeId{WeaponUpgradeId::ExtraProjectiles};
    UpgradeRarity rarity{UpgradeRarity::Common};
    int level{1};
    QString iconPath;
};

using UpgradeOptions = QList<UpgradeOption>;

struct UpgradeAttributePreview {
    QString label;
    QString currentValue;
    QString nextValue;
    QString deltaText;
    bool changed {false};
    bool positiveChange {true};
};

using UpgradeAttributePreviews = QList<UpgradeAttributePreview>;

struct UpgradePreviewContext {
    PlayerClassId classId {PlayerClassId::Warrior};
    WeaponId weaponId {WeaponId::PeaShooter};
    QList<TraitId> ownedTraits;
};

struct BulletSpecialTemplate {
    BulletStyle style;
    QString displayName;
    float baseDamage;
    float baseSpeed;
    SpecialEffect effect{SpecialEffect::None};
    float effectParam1{0.0F};
    float effectParam2{0.0F};
    QString emojiText;
    QString defaultImagePath;
    QList<QString> altImagePaths;
    int maxLevel{1};
};

Q_DECLARE_METATYPE(UpgradeOption)
Q_DECLARE_METATYPE(UpgradeOptions)
Q_DECLARE_METATYPE(BulletSpecialTemplate)
