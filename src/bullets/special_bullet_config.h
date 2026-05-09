#pragma once

#include "game_enums.h"
#include "game_structs.h"
#include "special_bullet_data.h"
#include <QList>

namespace BulletTemplateConfig {

inline const QList<BulletSpecialTemplate> kAllBulletTemplates {
    {BulletStyle::Normal, QStringLiteral("普通弹"), 0.0F, 0.0F, SpecialEffect::None, 0.0F, 0.0F, QString(), QString(), {}, 1},
    {BulletStyle::Dagger, QStringLiteral("飞刀"), 12.0F, 550.0F, SpecialEffect::None, 0.0F, 0.0F, QStringLiteral("\xF0\x9F\x94\xAA"), QStringLiteral(":/images/dagger_3d.png"), {}, 3},
    {BulletStyle::SunOrb, QStringLiteral("太阳法球"), 18.0F, 320.0F, SpecialEffect::None, 0.0F, 0.0F, QStringLiteral("\xE2\x98\x80\xEF\xB8\x8F"), QStringLiteral(":/images/sun_3d.png"), {}, 3},
    {BulletStyle::MoonOrb, QStringLiteral("月亮法球"), 14.0F, 340.0F, SpecialEffect::None, 0.0F, 0.0F, QStringLiteral("\xF0\x9F\x8C\x99"), QStringLiteral(":/images/new_moon_3d.png"), {}, 3},
    {BulletStyle::Hacimi, QStringLiteral("哈基米"), 8.0F, 300.0F, SpecialEffect::TrackAndAttach, 3.0F, 0.15F, QStringLiteral("\xF0\x9F\x90\xB1"), QStringLiteral(":/images/grinning_cat_3d.png"),
     {QStringLiteral(":/images/cat_with_wry_smile_3d.png"), QStringLiteral(":/images/pouting_cat_3d.png"), QStringLiteral(":/images/weary_cat_3d.png"), QStringLiteral(":/images/cat_with_tears_of_joy_3d.png"), QStringLiteral(":/images/grinning_cat_3d.png")}, 3},
    {BulletStyle::ThunderSpear, QStringLiteral("雷霆之矛"), 22.0F, 600.0F, SpecialEffect::None, 0.0F, 0.0F, QStringLiteral("\xE2\x9A\xA1"), QStringLiteral(":/images/high_voltage_3d.png"), {}, 3},
    {BulletStyle::Boomerang, QStringLiteral("回力镖"), 10.0F, 250.0F, SpecialEffect::None, 0.0F, 0.0F, QStringLiteral("\xF0\x9F\xAA\x83"), QStringLiteral(":/images/boomerang_3d.png"), {}, 3},
    {BulletStyle::BloodArrow, QStringLiteral("血之箭"), 16.0F, 450.0F, SpecialEffect::None, 0.0F, 0.0F, QStringLiteral("\xF0\x9F\xA9\xB8"), QStringLiteral(":/images/drop_of_blood_3d.png"), {}, 3},
    {BulletStyle::StunBullet, QStringLiteral("不是哥们！？"), 15.0F, 200.0F, SpecialEffect::Stun, 1.5F, 0.0F, QStringLiteral("\xF0\x9F\xA6\x90"), QStringLiteral(":/images/shrimp_3d.png"), {}, 3},
    {BulletStyle::RandomGift, QStringLiteral("随机礼物"), 5.0F, 350.0F, SpecialEffect::None, 0.0F, 0.0F, QStringLiteral("\xF0\x9F\x8E\x81"), QStringLiteral(":/images/wrapped_gift_3d.png"), {}, 3},
    {BulletStyle::Comet, QStringLiteral("彗星"), 20.0F, 280.0F, SpecialEffect::None, 0.0F, 0.0F, QStringLiteral("\xE2\x98\x84\xEF\xB8\x8F"), QStringLiteral(":/images/comet_3d.png"), {}, 3},
    {BulletStyle::PushBullet, QStringLiteral("带派不老铁"), 5.0F, 400.0F, SpecialEffect::KnockbackWithBonus, 150.0F, 0.3F, QStringLiteral("\xF0\x9F\x91\xA3"), QStringLiteral(":/images/footprints_3d.png"), {}, 3},
    {BulletStyle::Rocket, QStringLiteral("火箭"), 25.0F, 180.0F, SpecialEffect::None, 0.0F, 0.0F, QStringLiteral("\xF0\x9F\x9A\x80"), QStringLiteral(":/images/rocket_3d.png"), {}, 3},
};

[[nodiscard]] inline const BulletSpecialTemplate *findBulletTemplate(BulletStyle style)
{
    for (const auto &t : kAllBulletTemplates) {
        if (t.style == style) return &t;
    }
    return nullptr;
}

[[nodiscard]] inline UpgradeOptions generateSpecialBulletUpgradeOptions()
{
    UpgradeOptions options;
    const QList<int> specialIndices = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    for (int idx : specialIndices) {
        const auto &tmpl = kAllBulletTemplates.at(idx);
        UpgradeOption opt;
        opt.optionId = QStringLiteral("bullet.%1").arg(static_cast<int>(tmpl.style));
        opt.kind = UpgradeOptionKind::Stat;
        opt.displayName = tmpl.displayName;
        opt.summary = QStringLiteral("基础伤害 %1  速度 %2  [特殊子弹]").arg(QString::number(tmpl.baseDamage, 'f', 0), QString::number(tmpl.baseSpeed, 'f', 0));
        opt.rarity = UpgradeRarity::Epic;
        opt.level = 1;
        opt.iconPath = tmpl.defaultImagePath;
        options.append(opt);
    }
    return options;
}

} // namespace BulletTemplateConfig
