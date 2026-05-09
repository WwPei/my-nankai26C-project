#pragma once

#include <QMetaType>

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
    Ogre,
    Jester,
    DemonLord,
    SkeletonNew,
    Ghost,
    BoneLord,
    Robot,
    XenoBeast,
    UFO,
    AlienPilot
};

enum class EnemyBehavior {
    Chase,
    KeepDistance,
    Charge,
    ShootAndMove,
    SuicideBomb,
    Boss
};

enum class TraitId {
    QuickHands,
    ThickSkin,
    LuckyShot,
    VampiricAura,
    BouncingBullet,
    Frostbite,
    CriticalStrike,
    ExperienceBoost,
    Vitality,
    Adrenaline
};

enum class UpgradeOptionKind {
    Trait,
    Weapon,
    Stat
};

enum class UpgradeRarity {
    Common,
    Rare,
    Epic
};

enum class WeaponUpgradeId {
    ExtraProjectiles,
    RangeBoost,
    Pierce,
    BulletSize,
    Combo
};

enum class DamageVisualType {
    Neutral,
    Rapid,
    Arcane
};

enum class BulletStyle {
    Normal,
    Dagger,
    SunOrb,
    MoonOrb,
    Hacimi,
    ThunderSpear,
    Boomerang,
    BloodArrow,
    StunBullet,
    RandomGift,
    Comet,
    PushBullet,
    Rocket
};

enum class SpecialEffect {
    None,
    TrackAndAttach,
    Stun,
    KnockbackWithBonus
};

Q_DECLARE_METATYPE(BattleFlowState)
Q_DECLARE_METATYPE(UpgradeOptionKind)
Q_DECLARE_METATYPE(DamageVisualType)
Q_DECLARE_METATYPE(UpgradeRarity)
Q_DECLARE_METATYPE(WeaponUpgradeId)
Q_DECLARE_METATYPE(EnemyBehavior)
Q_DECLARE_METATYPE(BulletStyle)
Q_DECLARE_METATYPE(SpecialEffect)
