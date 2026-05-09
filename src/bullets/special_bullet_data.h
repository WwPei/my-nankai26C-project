#pragma once

#include "basic_bullet_data.h"
#include <QString>
#include <QList>

enum class SpecialEffect;

struct BulletSpecialConfig {
    SpecialEffect effect{SpecialEffect::None};
    float param1{0.0F};
    float param2{0.0F};
    QString emojiIcon;
    QString imagePath;
    QList<QString> altImagePaths;
};

class SpecialBulletData final : public BasicBulletData {
    Q_OBJECT
public:
    SpecialBulletData(WeaponId weaponId, const BulletConfig *config,
                      const QPointF &spawnPos, const QPointF &direction,
                      const BulletSpecialConfig &specialConfig,
                      QObject *parent = nullptr);

    [[nodiscard]] SpecialEffect specialEffect() const;
    [[nodiscard]] const BulletSpecialConfig &specialConfig() const;
    [[nodiscard]] QString currentImagePath() const;
    [[nodiscard]] bool isAttached() const;
    void setAttached(bool attached, int targetIndex);
    [[nodiscard]] int attachedTargetIndex() const;
    [[nodiscard]] float attachTimer() const;
    void advanceAttachTimer(float dt);
    void advanceSpecialFrame(float deltaSeconds, const QVector<QPointF> &enemyPositions);

public slots:
    void advanceFrame(float deltaSeconds) override;

private:
    BulletSpecialConfig m_specialConfig;
    bool m_attached{false};
    int m_attachedTarget{-1};
    float m_attachTimer{0.0F};
    int m_imageCycleIndex{0};
};
