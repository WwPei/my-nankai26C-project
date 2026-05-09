// FILE_LOCK: @qt6-logic-developer-emojidungeon
// 负责: 游戏主页面最小可玩战斗逻辑
// 最后修改: 2026-04-28
#pragma once

#include <QPointF>
#include <QVector>
#include <QWidget>

#include "game_data.h"
#include "game_factory.h"

class BattleArenaView;
class QLabel;
class PlayerAvatarItem;
class Player;
class Weapon;
class QCheckBox;
class QGraphicsScene;
class QGraphicsEllipseItem;
class QProgressBar;
class QTimer;
class GameFactory;

class GameMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit GameMainPage(QWidget *parent = nullptr);
    ~GameMainPage() override = default;

    void setFactory(GameFactory *factory);
    void setSelectedClass(PlayerClassId classId);
    void setBattleActive(bool active);
    void enterUpgradeState();
    void resumeBattleState();
    [[nodiscard]] BattleFlowState battleState() const noexcept;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void upgradeRequested();
    void exitRequested();
    void battleStateChanged(BattleFlowState state);

private:
    void setBattleState(BattleFlowState state);
    void rebuildBattleScene();
    void clearBattleScene();
    void spawnTestEnemy();
    void handleBattleTick();
    void handleWeaponFireRequested(WeaponId weaponId, const QPointF &origin, const QPointF &direction);
    void cleanupExpiredBullets();
    void cleanupDefeatedEnemies();
    void resolveCombatCollisions();
    void updateInputDirection();
    void updatePlayerMovement(float deltaSeconds);
    void updateWeaponAim();
    void updatePlayerVisualState();
    void updateHealthBarStyle(float healthRatio);
    [[nodiscard]] QPointF clampPlayerPosition(const QPointF &position) const;
    [[nodiscard]] QPointF randomEnemySpawnPosition() const;
    [[nodiscard]] EnemyId randomEnemyId() const;
    void updateStatusText();

    QLabel *m_classLabel {nullptr};
    QLabel *m_statusLabel {nullptr};
    QLabel *m_weaponLabel {nullptr};
    QLabel *m_attackLabel {nullptr};
    QLabel *m_attackSpeedLabel {nullptr};
    QLabel *m_moveSpeedLabel {nullptr};
    QLabel *m_enemyCountLabel {nullptr};
    QLabel *m_bulletCountLabel {nullptr};
    QLabel *m_aimHintLabel {nullptr};
    QCheckBox *m_gridToggle {nullptr};
    QProgressBar *m_healthBar {nullptr};
    QProgressBar *m_waveProgressBar {nullptr};
    QGraphicsScene *m_scene {nullptr};
    BattleArenaView *m_view {nullptr};
    QGraphicsEllipseItem *m_playerMarker {nullptr};
    PlayerAvatarItem *m_playerAvatar {nullptr};
    QTimer *m_gameLoopTimer {nullptr};
    GameFactory *m_factory {nullptr};
    Player *m_player {nullptr};
    Weapon *m_weapon {nullptr};
    PlayerClassId m_selectedClassId {PlayerClassId::Warrior};
    bool m_hasSelectedClass {false};
    QVector<GameFactory::EnemyEntity> m_enemies;
    QVector<GameFactory::BulletEntity> m_bullets;
    QPointF m_inputDirection;
    QPointF m_mouseScenePosition {80.0, 0.0};
    float m_enemySpawnAccumulatorMs {0.0F};
    float m_playerDamageCooldownRemainingMs {0.0F};
    int m_elapsedRoundMs {0};
    BattleFlowState m_battleState {BattleFlowState::Inactive};
    bool m_moveUpPressed {false};
    bool m_moveDownPressed {false};
    bool m_moveLeftPressed {false};
    bool m_moveRightPressed {false};
    bool m_firePressed {false};
};
