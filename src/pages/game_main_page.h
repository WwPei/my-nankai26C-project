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
class QAudioOutput;
class QFrame;
class QGridLayout;
class QHBoxLayout;
class QLabel;
class QMediaPlayer;
class PlayerAvatarItem;
class Player;
class UpgradeResolver;
class Weapon;
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
    void traitAcquired(TraitId traitId);

protected:
    void resizeEvent(QResizeEvent *event) override;

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
    void setupBgMusic();
    void updateBattleBackground();
    void switchBgMusic(const QString &trackName);
    void checkBossDefeat();
    void playHitSound();
    void playKillSound();
    void positionHudElements();
    void showUpgradeOverlay();
    void hideUpgradeOverlay();
    void buildUpgradeCards(const UpgradeOptions &options);
    void onUpgradeCardClicked(int index);
    void confirmUpgrade();

    QWidget *m_rightHudPanel {nullptr};
    QProgressBar *m_healthBar {nullptr};
    QLabel *m_levelLabel {nullptr};
    QProgressBar *m_expBar {nullptr};
    QLabel *m_weaponIconLabel {nullptr};

    QWidget *m_leftHudPanel {nullptr};
    QProgressBar *m_leftExpBar {nullptr};
    QHBoxLayout *m_traitIconsLayout {nullptr};

    QProgressBar *m_waveProgressBar {nullptr};

    QWidget *m_upgradeOverlay {nullptr};
    QGridLayout *m_upgradeCardsGrid {nullptr};
    QList<QFrame *> m_upgradeCards;
    QPushButton *m_upgradeConfirmButton {nullptr};
    int m_selectedUpgradeIndex {-1};
    UpgradeOptions m_pendingUpgradeOptions;
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

    QMediaPlayer *m_bgMusicPlayer{nullptr};
    QAudioOutput *m_audioOutput{nullptr};
    QMediaPlayer *m_hitSoundPlayer{nullptr};
    QAudioOutput *m_hitSoundOutput{nullptr};
    QMediaPlayer *m_killSoundPlayer{nullptr};
    QAudioOutput *m_killSoundOutput{nullptr};
    bool m_backgroundTechActive{false};
    bool m_demonLordDefeated{false};
    QString m_currentNormalBgm{QStringLiteral("sery1_normal")};
};
