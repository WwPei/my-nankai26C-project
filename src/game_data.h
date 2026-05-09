#pragma once

#include <QList>
#include <QMetaType>
#include <QString>

enum class PageId {
    Start,
    ClassSelect,
    GameMain,
    Upgrade
};

enum class BattleFlowState {
    Inactive,
    Battle,
    Upgrade
};

enum class PlayerClassId {
    Warrior,
    Ranger,
    Caster
};

enum class WeaponId {
    PeaShooter,
    SpreadBlaster,
    ArcWand
};

enum class EnemyId {
    Slime,
    Bat,
    Skeleton
};

enum class TraitId {
    QuickHands,
    ThickSkin,
    LuckyShot
};

enum class UpgradeOptionKind {
    Trait,
    Weapon,
    Stat
};

struct WaveConfig {
    int roundDurationMs {30000};
    int updateIntervalMs {16};
    int maxConcurrentEnemies {10};
    int maxRounds {10};
    int initialEnemyCount {3};
    int enemySpawnIntervalMs {3000};
    int experiencePerEnemyDefeat {10};
    int upgradeSelectionCount {3};
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
};

struct TraitConfig {
    TraitId id;
    QString displayName;
    QString summary;
    float damageMultiplier;
    float defenseMultiplier;
    float speedMultiplier;
};

struct UpgradeOption {
    QString optionId;
    UpgradeOptionKind kind {UpgradeOptionKind::Trait};
    QString displayName;
    QString summary;
    TraitId traitId {TraitId::QuickHands};
};

using UpgradeOptions = QList<UpgradeOption>;

namespace GameConfig {

inline const WaveConfig kWaveConfig {};
inline const QList<int> kExperienceThresholds {
    0,
    20,
    45,
    75,
    110,
    150,
    195,
    245,
    300,
    360
};

inline const QList<PlayerClassConfig> kPlayerClasses {
    {
        PlayerClassId::Warrior,
        QStringLiteral("战士"),
        QStringLiteral("近中距离稳健成长，适合作为默认职业占位。"),
        WeaponId::PeaShooter,
        120.0F,
        235.0F
    },
    {
        PlayerClassId::Ranger,
        QStringLiteral("游侠"),
        QStringLiteral("偏高机动与持续输出，便于后续扩展风筝流派。"),
        WeaponId::SpreadBlaster,
        95.0F,
        270.0F
    },
    {
        PlayerClassId::Caster,
        QStringLiteral("施法者"),
        QStringLiteral("慢速高爆发，适配后续法术与轨道弹体系。"),
        WeaponId::ArcWand,
        85.0F,
        220.0F
    }
};

inline const QList<WeaponConfig> kWeapons {
    {
        WeaponId::PeaShooter,
        QStringLiteral("豆豆发射器"),
        QStringLiteral("基础单发武器，占位用于验证玩家-武器绑定链路。"),
        10.0F,
        500.0F,
        420.0F,
        1
    },
    {
        WeaponId::SpreadBlaster,
        QStringLiteral("散射喷发器"),
        QStringLiteral("扇形多发武器，占位用于验证多子弹配置结构。"),
        7.0F,
        700.0F,
        380.0F,
        3
    },
    {
        WeaponId::ArcWand,
        QStringLiteral("弧光法杖"),
        QStringLiteral("高伤慢速武器，占位用于验证法术型配置。"),
        16.0F,
        900.0F,
        360.0F,
        1
    }
};

inline const QList<BulletConfig> kBullets {
    {
        WeaponId::PeaShooter,
        QStringLiteral("豌豆弹"),
        10.0F,
        420.0F,
        6.0F
    },
    {
        WeaponId::SpreadBlaster,
        QStringLiteral("散射弹"),
        7.0F,
        380.0F,
        5.0F
    },
    {
        WeaponId::ArcWand,
        QStringLiteral("弧光弹"),
        16.0F,
        360.0F,
        7.0F
    }
};

inline const QList<EnemyConfig> kEnemies {
    {
        EnemyId::Slime,
        QStringLiteral("史莱姆"),
        QStringLiteral("基础近战敌人，占位用于验证波次刷怪接口。"),
        25.0F,
        110.0F,
        5.0F,
        18.0F
    },
    {
        EnemyId::Bat,
        QStringLiteral("蝙蝠"),
        QStringLiteral("高速轻量敌人，占位用于验证移动速度差异。"),
        15.0F,
        180.0F,
        4.0F,
        12.0F
    },
    {
        EnemyId::Skeleton,
        QStringLiteral("骷髅"),
        QStringLiteral("耐久型敌人，占位用于验证不同生命模板。"),
        40.0F,
        95.0F,
        7.0F,
        20.0F
    }
};

inline const QList<TraitConfig> kTraits {
    {
        TraitId::QuickHands,
        QStringLiteral("快手"),
        QStringLiteral("提高攻击节奏，为升级页占位选项之一。"),
        1.15F,
        1.00F,
        1.05F
    },
    {
        TraitId::ThickSkin,
        QStringLiteral("厚皮"),
        QStringLiteral("提高生存能力，用于演示特性配置结构。"),
        1.00F,
        1.20F,
        1.00F
    },
    {
        TraitId::LuckyShot,
        QStringLiteral("幸运射击"),
        QStringLiteral("提高伤害上限，用于演示伤害增益型特性。"),
        1.25F,
        1.00F,
        1.00F
    }
};

inline const UpgradeOptions kUpgradeOptions {
    {
        QStringLiteral("trait.quick_hands"),
        UpgradeOptionKind::Trait,
        QStringLiteral("快手"),
        QStringLiteral("提高攻击节奏，作为阶段2升级选项基线。"),
        TraitId::QuickHands
    },
    {
        QStringLiteral("trait.thick_skin"),
        UpgradeOptionKind::Trait,
        QStringLiteral("厚皮"),
        QStringLiteral("提高生存能力，作为阶段2升级选项基线。"),
        TraitId::ThickSkin
    },
    {
        QStringLiteral("trait.lucky_shot"),
        UpgradeOptionKind::Trait,
        QStringLiteral("幸运射击"),
        QStringLiteral("提高伤害上限，作为阶段2升级选项基线。"),
        TraitId::LuckyShot
    }
};

[[nodiscard]] inline const PlayerClassConfig *findPlayerClassConfig(PlayerClassId id)
{
    for (const auto &config : kPlayerClasses) {
        if (config.id == id) {
            return &config;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const WeaponConfig *findWeaponConfig(WeaponId id)
{
    for (const auto &config : kWeapons) {
        if (config.id == id) {
            return &config;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const EnemyConfig *findEnemyConfig(EnemyId id)
{
    for (const auto &config : kEnemies) {
        if (config.id == id) {
            return &config;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const BulletConfig *findBulletConfig(WeaponId weaponId)
{
    for (const auto &config : kBullets) {
        if (config.weaponId == weaponId) {
            return &config;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const TraitConfig *findTraitConfig(TraitId id)
{
    for (const auto &config : kTraits) {
        if (config.id == id) {
            return &config;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const UpgradeOption *findUpgradeOptionByTraitId(TraitId id)
{
    for (const auto &option : kUpgradeOptions) {
        if (option.kind == UpgradeOptionKind::Trait && option.traitId == id) {
            return &option;
        }
    }
    return nullptr;
}

[[nodiscard]] inline int experienceThresholdForLevel(int level)
{
    if (level <= 0) {
        return 0;
    }
    if (level >= kExperienceThresholds.size()) {
        return kExperienceThresholds.constLast();
    }
    return kExperienceThresholds.at(level);
}

[[nodiscard]] inline int levelForExperience(int experience)
{
    int currentLevel = 0;
    for (int index = 0; index < kExperienceThresholds.size(); ++index) {
        if (experience < kExperienceThresholds.at(index)) {
            break;
        }
        currentLevel = index;
    }
    return currentLevel;
}

[[nodiscard]] inline int experienceToNextLevel(int experience)
{
    const int currentLevel = levelForExperience(experience);
    if (currentLevel + 1 >= kExperienceThresholds.size()) {
        return 0;
    }

    return kExperienceThresholds.at(currentLevel + 1) - experience;
}

} // namespace GameConfig

Q_DECLARE_METATYPE(UpgradeOption)
Q_DECLARE_METATYPE(UpgradeOptions)
Q_DECLARE_METATYPE(BattleFlowState)
Q_DECLARE_METATYPE(UpgradeOptionKind)
