// FILE_LOCK: @qt6-logic-developer-emojidungeon
// 负责: 游戏配置表 - 经验阈值、波次经验倍率、职业/武器/敌人/Trait配置
// 最后修改: 2026-05-09

#pragma once

#include "game_structs.h"

#include <QMap>

namespace GameConfig {

inline const WaveConfig kWaveConfig {};
inline const QList<int> kExperienceThresholds {
    0,
    0,
    10,
    50,
    140,
    300,
    550,
    910,
    1400,
    2040,
    2850,
    3850,
    5060,
    6500,
    8190,
    10150
};

[[nodiscard]] inline float waveExpMultiplier(int waveNumber) {
    if (waveNumber >= 10) return 1.5F;
    if (waveNumber >= 8) return 1.2F;
    if (waveNumber >= 4) return 1.1F;
    return 1.0F;
}

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
        2
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

inline const QList<WeaponUpgradeConfig> kWeaponUpgrades{
    {
        WeaponUpgradeId::ExtraProjectiles,
        QStringLiteral("弹道增幅"),
        QStringLiteral("增加1条额外弹道"),
        1.0F, 1.0F, 0, 1.0F, 0, 1.0F, 1.0F
    },
    {
        WeaponUpgradeId::RangeBoost,
        QStringLiteral("射程提升"),
        QStringLiteral("子弹飞行距离+30%"),
        0.0F, 1.30F, 0, 1.0F, 0, 1.0F, 1.0F
    },
    {
        WeaponUpgradeId::Pierce,
        QStringLiteral("穿透"),
        QStringLiteral("子弹穿透1个敌人后继续飞行"),
        0.0F, 1.0F, 1, 1.0F, 0, 1.0F, 1.0F
    },
    {
        WeaponUpgradeId::BulletSize,
        QStringLiteral("子弹增幅"),
        QStringLiteral("子弹碰撞半径+40%"),
        0.0F, 1.0F, 0, 1.40F, 0, 1.0F, 1.0F
    },
    {
        WeaponUpgradeId::Combo,
        QStringLiteral("连击"),
        QStringLiteral("每第3发子弹造成额外50%伤害"),
        0.0F, 1.0F, 0, 1.0F, 3, 1.50F, 1.0F
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

inline const QList<EnemyConfig> kEnemies{
    {
        EnemyId::Ogre,
        QStringLiteral("食人魔"),
        QStringLiteral("高生命近战"),
        80.0F, 110.0F, 12.0F, 22.0F, 15.0F,
        QStringLiteral(":/images/ogre_3d.png"),
        EnemyBehavior::Charge,
        0.0F, 800.0F, 600.0F, 2.5F, 3000.0F
    },
    {
        EnemyId::Jester,
        QStringLiteral("小丑脸"),
        QStringLiteral("灵活穿梭，远距射击"),
        35.0F, 90.0F, 6.0F, 15.0F, 12.0F,
        QStringLiteral(":/images/clown_face_3d.png"),
        EnemyBehavior::ShootAndMove,
        180.0F, 0.0F, 0.0F, 1.0F, 0.0F,
        2000.0F, 1, 170.0F, 6.0F
    },
    {
        EnemyId::DemonLord,
        QStringLiteral("恶魔领主"),
        QStringLiteral("怪异Boss，第4波登场"),
        350.0F, 45.0F, 18.0F, 28.0F, 80.0F,
        QStringLiteral(":/images/smiling_face_with_horns_3d.png"),
        EnemyBehavior::Boss,
        0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
        2500.0F, 8, 150.0F, 10.0F,
        true, true
    },
    {
        EnemyId::SkeletonNew,
        QStringLiteral("骷髅兵"),
        QStringLiteral("亡灵近战单位"),
        25.0F, 70.0F, 8.0F, 16.0F, 12.0F,
        QStringLiteral(":/images/skull_3d.png"),
        EnemyBehavior::Chase
    },
    {
        EnemyId::Ghost,
        QStringLiteral("幽灵"),
        QStringLiteral("飘忽不定，穿墙"),
        20.0F, 85.0F, 5.0F, 14.0F, 12.0F,
        QStringLiteral(":/images/ghost_3d.png"),
        EnemyBehavior::ShootAndMove,
        100.0F, 0.0F, 0.0F, 1.0F, 0.0F,
        1800.0F, 3, 170.0F, 5.0F,
        150.0F
    },
    {
        EnemyId::BoneLord,
        QStringLiteral("骨王"),
        QStringLiteral("亡灵Boss，第7波登场"),
        420.0F, 40.0F, 22.0F, 30.0F, 100.0F,
        QStringLiteral(":/images/skull_and_crossbones_3d.png"),
        EnemyBehavior::Boss,
        0.0F, 1000.0F, 500.0F, 3.0F, 4000.0F,
        1800.0F, 10, 140.0F, 8.0F,
        true, true
    },
    {
        EnemyId::Robot,
        QStringLiteral("机器人"),
        QStringLiteral("远程射击，高护甲"),
        45.0F, 58.0F, 8.0F, 20.0F, 15.0F,
        QStringLiteral(":/images/robot_3d.png"),
        EnemyBehavior::ShootAndMove,
        200.0F, 0.0F, 0.0F, 1.0F, 0.0F,
        2200.0F, 1, 190.0F, 7.0F,
        true, false
    },
    {
        EnemyId::XenoBeast,
        QStringLiteral("异形兽"),
        QStringLiteral("冲锋近战，高速"),
        55.0F, 105.0F, 10.0F, 18.0F, 15.0F,
        QStringLiteral(":/images/alien_monster_3d.png"),
        EnemyBehavior::Charge,
        0.0F, 500.0F, 700.0F, 2.8F, 2500.0F
    },
    {
        EnemyId::UFO,
        QStringLiteral("飞碟"),
        QStringLiteral("外星Boss一阶段"),
        500.0F, 35.0F, 8.0F, 32.0F, 120.0F,
        QStringLiteral(":/images/flying_saucer_3d.png"),
        EnemyBehavior::Boss,
        0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
        1500.0F, 12, 130.0F, 7.0F,
        true, true
    },
    {
        EnemyId::AlienPilot,
        QStringLiteral("外星人"),
        QStringLiteral("外星Boss二阶段，第9波"),
        300.0F, 78.0F, 14.0F, 22.0F, 60.0F,
        QStringLiteral(":/images/alien_3d.png"),
        EnemyBehavior::Boss,
        0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
        1200.0F, 10, 170.0F, 9.0F,
        false, false
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
    },
    {
        TraitId::VampiricAura,
        QStringLiteral("吸血光环"),
        QStringLiteral("击败敌人时回复10%最大生命值"),
        1.0F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        1.0F,
        0.10F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F
    },
    {
        TraitId::BouncingBullet,
        QStringLiteral("弹射弹"),
        QStringLiteral("子弹命中后弹射到最近敌人"),
        1.0F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        1.0F,
        0.0F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        0.0F,
        180.0F,
        0.70F
    },
    {
        TraitId::Frostbite,
        QStringLiteral("冰冻减速"),
        QStringLiteral("子弹命中使敌人减速40%持续2秒"),
        1.0F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        1.0F,
        0.0F,
        1.0F,
        0.60F,
        2.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F
    },
    {
        TraitId::CriticalStrike,
        QStringLiteral("暴击专精"),
        QStringLiteral("攻击有15%概率造成1.5倍伤害"),
        1.0F,
        1.0F,
        1.0F,
        0.0F,
        0.15F,
        1.5F,
        0.0F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F
    },
    {
        TraitId::ExperienceBoost,
        QStringLiteral("经验加成"),
        QStringLiteral("击败敌人获得经验+20%"),
        1.0F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        1.0F,
        0.0F,
        1.20F,
        1.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F
    },
    {
        TraitId::Vitality,
        QStringLiteral("生命提升"),
        QStringLiteral("最大生命值固定增加30点"),
        1.0F,
        1.0F,
        1.0F,
        30.0F,
        0.0F,
        1.0F,
        0.0F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F,
        0.0F
    },
    {
        TraitId::Adrenaline,
        QStringLiteral("肾上腺"),
        QStringLiteral("击杀敌人后移速+30%持续3秒"),
        1.0F,
        1.0F,
        1.0F,
        0.0F,
        0.0F,
        1.0F,
        0.0F,
        1.0F,
        1.0F,
        0.0F,
        0.30F,
        3.0F,
        0.0F,
        0.0F
    }
};

inline const UpgradeOptions kUpgradeOptions {
    {
        QStringLiteral("trait.quick_hands"),
        UpgradeOptionKind::Trait,
        QStringLiteral("快手"),
        QStringLiteral("提高攻击节奏，作为阶段2升级选项基线。"),
        TraitId::QuickHands,
        WeaponUpgradeId::ExtraProjectiles,
        UpgradeRarity::Common,
        1,
        QString()
    },
    {
        QStringLiteral("trait.thick_skin"),
        UpgradeOptionKind::Trait,
        QStringLiteral("厚皮"),
        QStringLiteral("提高生存能力，作为阶段2升级选项基线。"),
        TraitId::ThickSkin,
        WeaponUpgradeId::ExtraProjectiles,
        UpgradeRarity::Common,
        1,
        QString()
    },
    {
        QStringLiteral("trait.lucky_shot"),
        UpgradeOptionKind::Trait,
        QStringLiteral("幸运射击"),
        QStringLiteral("提高伤害上限，作为阶段2升级选项基线。"),
        TraitId::LuckyShot,
        WeaponUpgradeId::ExtraProjectiles,
        UpgradeRarity::Common,
        1,
        QString()
    }
};

inline const UpgradeOptions kAllUpgradeOptions{
    {QStringLiteral("trait.quick_hands.1"), UpgradeOptionKind::Trait, QStringLiteral("快手"), QStringLiteral("提高攻击节奏"), TraitId::QuickHands, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/quick_hands.png")},
    {QStringLiteral("trait.thick_skin.1"), UpgradeOptionKind::Trait, QStringLiteral("厚皮"), QStringLiteral("提高生存能力"), TraitId::ThickSkin, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/thick_skin.png")},
    {QStringLiteral("trait.lucky_shot.1"), UpgradeOptionKind::Trait, QStringLiteral("幸运射击"), QStringLiteral("提高伤害上限"), TraitId::LuckyShot, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/lucky_shot.png")},
    {QStringLiteral("trait.vampiric_aura.1"), UpgradeOptionKind::Trait, QStringLiteral("吸血光环"), QStringLiteral("击杀回复10%生命"), TraitId::VampiricAura, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/vampiric_aura.png")},
    {QStringLiteral("trait.bouncing_bullet.1"), UpgradeOptionKind::Trait, QStringLiteral("弹射弹"), QStringLiteral("子弹命中弹射到最近敌人"), TraitId::BouncingBullet, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Rare, 1, QStringLiteral(":/images/upgrade_icons/bouncing_bullet.png")},
    {QStringLiteral("trait.frostbite.1"), UpgradeOptionKind::Trait, QStringLiteral("冰冻减速"), QStringLiteral("子弹命中使敌人减速40%"), TraitId::Frostbite, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/frostbite.png")},
    {QStringLiteral("trait.critical_strike.1"), UpgradeOptionKind::Trait, QStringLiteral("暴击专精"), QStringLiteral("15%概率1.5倍伤害"), TraitId::CriticalStrike, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Rare, 1, QStringLiteral(":/images/upgrade_icons/critical_strike.png")},
    {QStringLiteral("trait.exp_boost.1"), UpgradeOptionKind::Trait, QStringLiteral("经验加成"), QStringLiteral("击败敌人经验+20%"), TraitId::ExperienceBoost, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/exp_boost.png")},
    {QStringLiteral("trait.vitality.1"), UpgradeOptionKind::Trait, QStringLiteral("生命提升"), QStringLiteral("最大生命+30"), TraitId::Vitality, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/vitality.png")},
    {QStringLiteral("trait.adrenaline.1"), UpgradeOptionKind::Trait, QStringLiteral("肾上腺"), QStringLiteral("击杀后移速+30%持续3秒"), TraitId::Adrenaline, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/adrenaline.png")},
    {QStringLiteral("weapon.extra_projectiles.1"), UpgradeOptionKind::Weapon, QStringLiteral("弹道增幅"), QStringLiteral("增加1条额外弹道"), TraitId::QuickHands, WeaponUpgradeId::ExtraProjectiles, UpgradeRarity::Rare, 1, QStringLiteral(":/images/upgrade_icons/extra_projectiles.png")},
    {QStringLiteral("weapon.range_boost.1"), UpgradeOptionKind::Weapon, QStringLiteral("射程提升"), QStringLiteral("子弹飞行距离+30%"), TraitId::QuickHands, WeaponUpgradeId::RangeBoost, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/range_boost.png")},
    {QStringLiteral("weapon.pierce.1"), UpgradeOptionKind::Weapon, QStringLiteral("穿透"), QStringLiteral("子弹穿透1个敌人"), TraitId::QuickHands, WeaponUpgradeId::Pierce, UpgradeRarity::Rare, 1, QStringLiteral(":/images/upgrade_icons/pierce.png")},
    {QStringLiteral("weapon.bullet_size.1"), UpgradeOptionKind::Weapon, QStringLiteral("子弹增幅"), QStringLiteral("子弹碰撞半径+40%"), TraitId::QuickHands, WeaponUpgradeId::BulletSize, UpgradeRarity::Common, 1, QStringLiteral(":/images/upgrade_icons/bullet_size.png")},
    {QStringLiteral("weapon.combo.1"), UpgradeOptionKind::Weapon, QStringLiteral("连击"), QStringLiteral("每3发子弹造成额外50%伤害"), TraitId::QuickHands, WeaponUpgradeId::Combo, UpgradeRarity::Rare, 1, QStringLiteral(":/images/upgrade_icons/combo.png")},
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

[[nodiscard]] inline const WeaponUpgradeConfig *findWeaponUpgradeConfig(WeaponUpgradeId id)
{
    for (const auto &config : kWeaponUpgrades) {
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

[[nodiscard]] inline const UpgradeOption *findUpgradeOptionById(const QString &optionId)
{
    for (const auto &option : kAllUpgradeOptions) {
        if (option.optionId == optionId) {
            return &option;
        }
    }
    return nullptr;
}

[[nodiscard]] inline const UpgradeOption *findUpgradeOptionByWeaponUpgradeId(WeaponUpgradeId id)
{
    for (const auto &option : kAllUpgradeOptions) {
        if (option.kind == UpgradeOptionKind::Weapon && option.weaponUpgradeId == id) {
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
