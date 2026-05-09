// FILE_LOCK: @qt6-logic-developer-emojidungeon
// 负责: 游戏主页面战斗、波次推进与状态栏逻辑
// 最后修改: 2026-05-08
#pragma once

#include <QPointF>
#include <QVector>
#include <QWidget>

#include "game_data.h"
#include "game_factory.h"

class BattleArenaView;
class CombatCoordinator;
class DashCooldownWidget;
class EnemyData;
class EnemyDirector;
class QFrame;
class QLabel;
class PlayerAvatarItem;
class Player;
class UpgradeResolver;
class Weapon;
class QCheckBox;
class QGraphicsScene;
class QGraphicsEllipseItem;
class QProgressBar;
class QPushButton;
class QTimer;
class GameFactory;
class WaveManager;

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
    [[nodiscard]] WaveManager *waveManager() const noexcept;
    [[nodiscard]] WeaponId currentWeaponId() const noexcept;
    void applyTrait(TraitId traitId);
    void applyWeaponUpgrade(WeaponUpgradeId upgradeId);
    void setActiveBulletStyle(BulletStyle style);
    [[nodiscard]] BulletStyle activeBulletStyle() const noexcept;
    void resolveCombatCollisions();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void upgradeRequested();
    void exitRequested();
    void battleStateChanged(BattleFlowState state);
    void battleFinished(bool victory);
    void statsChanged();

private:
    void setBattleState(BattleFlowState state);
    void rebuildBattleScene();
    void clearBattleScene();
    void handleBattleTick();
    void updateInputDirection();
    void updatePlayerMovement(float deltaSeconds);
    void updateWeaponAim();
    void updatePlayerVisualState();
    void updateHealthBarStyle(float healthRatio);
    void clearActiveBullets();
    void clearActiveEnemies(bool grantExperience);
    void expandSceneToFitPlayer(const QPointF &position) const;
    void applyDynamicDifficulty(int wave, EnemyData *enemy);
    void updateStatusText();
    [[nodiscard]] QPointF bossSpawnPosition() const;

    QLabel *m_classLabel {nullptr};
    QLabel *m_statusLabel {nullptr};
    QLabel *m_weaponLabel {nullptr};
    QLabel *m_attackLabel {nullptr};
    QLabel *m_attackSpeedLabel {nullptr};
    QLabel *m_moveSpeedLabel {nullptr};
    QLabel *m_roundLabel {nullptr};
    QLabel *m_levelLabel {nullptr};
    QLabel *m_experienceLabel {nullptr};
    QLabel *m_enemyCountLabel {nullptr};
    QLabel *m_bulletCountLabel {nullptr};
    QLabel *m_attributeChangeLabel {nullptr};
    QLabel *m_traitsLabel {nullptr};
    QLabel *m_aimHintLabel {nullptr};
    QCheckBox *m_gridToggle {nullptr};
    QProgressBar *m_healthBar {nullptr};
    QProgressBar *m_waveProgressBar {nullptr};
    QPushButton *m_upgradeButton {nullptr};
    QGraphicsScene *m_scene {nullptr};
    BattleArenaView *m_view {nullptr};
    QGraphicsEllipseItem *m_playerMarker {nullptr};
    PlayerAvatarItem *m_playerAvatar {nullptr};
    QTimer *m_gameLoopTimer {nullptr};
    GameFactory *m_factory {nullptr};
    WaveManager *m_waveManager {nullptr};
    Player *m_player {nullptr};
    Weapon *m_weapon {nullptr};
    PlayerClassId m_selectedClassId {PlayerClassId::Warrior};
    bool m_hasSelectedClass {false};
    QVector<GameFactory::EnemyEntity> m_enemies;
    QVector<GameFactory::BulletEntity> m_bullets;
    QPointF m_inputDirection;
    QPointF m_mouseScenePosition {80.0, 0.0};
    QPointF m_lastMoveDirection{0.0F, -1.0F};
    float m_enemySpawnAccumulatorMs {0.0F};
    float m_playerDamageCooldownRemainingMs {0.0F};
    BattleFlowState m_battleState {BattleFlowState::Inactive};
    int m_currentWave{0};
    bool m_bossIsActive{false};
    EnemyId m_currentBossId{EnemyId::Ogre};
    QPointF m_bossSpawnPosition;
    bool m_moveUpPressed {false};
    bool m_moveDownPressed {false};
    bool m_moveLeftPressed {false};
    bool m_moveRightPressed {false};
    bool m_firePressed {false};
    BulletStyle m_activeBulletStyle{BulletStyle::Normal};

    UpgradeResolver *m_upgradeResolver{nullptr};
    CombatCoordinator *m_combatCoordinator{nullptr};
    EnemyDirector *m_enemyDirector{nullptr};
    DashCooldownWidget *m_dashCooldownWidget{nullptr};
};
