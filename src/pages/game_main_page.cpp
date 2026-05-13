#include "game_main_page.h"

#include "battle_arena_view.h"
#include "class_select_page.h"
#include "combat_coordinator.h"
#include "dash_cooldown_widget.h"
#include "emoji_dungeon_window.h"
#include "enemy.h"
#include "enemy_director.h"
#include "game_data.h"
#include "player.h"
#include "player_avatar_item.h"
#include "special_bullet_config.h"
#include "special_bullet_data.h"
#include "special_bullet_view.h"
#include "upgrade_page.h"
#include "upgrade_resolver.h"
#include "wave_manager.h"
#include "weapon.h"

#include <QApplication>
#include <QAudioOutput>
#include <QEvent>
#include <QFrame>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineF>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QProgressBar>
#include <QPushButton>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QStyle>
#include <QTimer>
#include <QVariantAnimation>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <utility>

namespace {

constexpr qreal kPlayerMarkerRadius = 14.0;
constexpr qreal kPlayerVisualRadius = 16.0;
constexpr float kPlayerDamageCooldownMs = 600.0F;
constexpr qreal kEnemySpawnInset = 24.0;
constexpr qreal kMaxWorldWidth = 5000.0;
constexpr qreal kMaxWorldHeight = 3000.0;

const char *kBattlePageStyle = R"(
QWidget#gameMainPage {
    background-color: #1b1d21;
    color: #f3f4f7;
    font-family: "Segoe UI Emoji", "Microsoft YaHei UI", sans-serif;
}
QLabel#pageTitleLabel {
    font-size: 22px;
    font-weight: 700;
    color: #f7f8fb;
}
QLabel#classLabel {
    color: #8fc7ff;
    font-size: 15px;
    font-weight: 600;
}
QLabel#statusLabel {
    color: #c8d0dc;
    padding: 10px 12px;
    background-color: #242932;
    border: 1px solid #343b48;
    border-radius: 10px;
}
QFrame#battlePanel,
QFrame#statusPanel {
    background-color: #20242b;
    border: 1px solid #323846;
    border-radius: 14px;
}
QLabel#panelTitleLabel {
    font-size: 15px;
    font-weight: 700;
    color: #ffffff;
}
QLabel#statTitleLabel {
    color: #93a0b4;
    font-size: 12px;
}
QLabel#statValueLabel {
    color: #f4f6fa;
    font-size: 16px;
    font-weight: 700;
}

QProgressBar {
    border: 1px solid #3a4352;
    border-radius: 6px;
    background-color: #15181d;
    color: #f6f8fb;
    min-height: 18px;
    text-align: center;
}
QProgressBar#waveProgressBar::chunk {
    border-radius: 5px;
    background-color: #4b7bec;
}
QPushButton {
    min-height: 38px;
    padding: 0 16px;
    border-radius: 10px;
    border: 1px solid #445069;
    background-color: #273246;
    color: #f7f9fc;
    font-weight: 600;
}
QPushButton:hover {
    background-color: #31425d;
    border-color: #6ea8ff;
}
QPushButton:pressed {
    background-color: #223046;
}
QGraphicsView#battleArenaView {
    border: 1px solid #394253;
    border-radius: 12px;
    background: #2a2a2a;
}
)";

[[nodiscard]] QPointF normalizedVector(const QPointF &vector, const QPointF &fallback = QPointF(1.0, 0.0))
{
    const QLineF line(QPointF(), vector);
    if (line.length() <= 0.001) {
        return fallback;
    }

    return QPointF(vector.x() / line.length(), vector.y() / line.length());
}

[[nodiscard]] bool isMovementKey(int key)
{
    return key == Qt::Key_W
        || key == Qt::Key_A
        || key == Qt::Key_S
        || key == Qt::Key_D;
}

[[nodiscard]] QString traitIconForId(TraitId tid)
{
    for (const auto &opt : GameConfig::kAllUpgradeOptions) {
        if (opt.kind == UpgradeOptionKind::Trait && opt.traitId == tid) {
            return opt.iconPath;
        }
    }
    return {};
}

[[nodiscard]] QString signedPercentText(float multiplier)
{
    const float deltaPercent = (multiplier - 1.0F) * 100.0F;
    const QString prefix = deltaPercent >= 0.0F ? QStringLiteral("+") : QString();
    return QStringLiteral("%1%2%")
        .arg(prefix,
             QString::number(deltaPercent, 'f', std::abs(deltaPercent) >= 9.95F ? 0 : 1));
}

[[nodiscard]] QString reductionPercentText(float defenseMultiplier)
{
    if (defenseMultiplier <= 1.0F) {
        return QStringLiteral("0%");
    }

    const float reductionPercent = (1.0F - (1.0F / defenseMultiplier)) * 100.0F;
    return QStringLiteral("%1%").arg(QString::number(reductionPercent, 'f', reductionPercent >= 9.95F ? 0 : 1));
}

} // namespace

GameMainPage::GameMainPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("gameMainPage"));
    setStyleSheet(QString::fromUtf8(kBattlePageStyle));
    setFocusPolicy(Qt::StrongFocus);
    installEventFilter(this);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-320.0, -180.0, 640.0, 360.0);

    m_view = new BattleArenaView(m_scene, this);
    m_view->setFocusPolicy(Qt::StrongFocus);
    m_view->setMouseTracking(true);
    m_view->viewport()->setMouseTracking(true);
    m_view->installEventFilter(this);
    m_view->viewport()->installEventFilter(this);
    rootLayout->addWidget(m_view, 1);

    m_playerMarker = m_scene->addEllipse(-kPlayerMarkerRadius, -kPlayerMarkerRadius,
                                         kPlayerMarkerRadius * 2.0, kPlayerMarkerRadius * 2.0,
                                         QPen(Qt::NoPen), QBrush(Qt::transparent));
    m_playerMarker->setVisible(false);
    m_playerMarker->setZValue(4.0);

    m_playerAvatar = new PlayerAvatarItem();
    m_playerAvatar->setVisible(false);
    m_scene->addItem(m_playerAvatar);

    m_rightHudPanel = new QWidget(this);
    m_rightHudPanel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_rightHudPanel->setStyleSheet(QStringLiteral(
        "background: rgba(20,24,35,0.88); border: 1px solid rgba(60,70,90,0.6); border-radius: 10px;"));
    auto *rightLayout = new QVBoxLayout(m_rightHudPanel);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    rightLayout->setSpacing(6);

    m_healthBar = new QProgressBar(m_rightHudPanel);
    m_healthBar->setRange(0, 100);
    m_healthBar->setValue(0);
    m_healthBar->setFormat(QStringLiteral("0 / 0"));
    m_healthBar->setTextVisible(true);
    m_healthBar->setFixedHeight(22);
    rightLayout->addWidget(m_healthBar);

    m_levelLabel = new QLabel(QStringLiteral("Lv.0"), m_rightHudPanel);
    m_levelLabel->setStyleSheet(
        QStringLiteral("color: #f6f8fc; font-size: 15px; font-weight: 800; background: transparent;"));
    rightLayout->addWidget(m_levelLabel);

    m_expBar = new QProgressBar(m_rightHudPanel);
    m_expBar->setRange(0, 100);
    m_expBar->setValue(0);
    m_expBar->setFormat(QStringLiteral("%p%"));
    m_expBar->setTextVisible(true);
    m_expBar->setFixedHeight(14);
    m_expBar->setStyleSheet(QStringLiteral(
        "QProgressBar { border: 1px solid #3a4352; border-radius: 4px; background: #15181d; color: #aab; }"
        "QProgressBar::chunk { border-radius: 3px; background-color: #6eb5ff; }"));
    rightLayout->addWidget(m_expBar);

    m_weaponIconLabel = new QLabel(m_rightHudPanel);
    m_weaponIconLabel->setFixedSize(50, 50);
    m_weaponIconLabel->setAlignment(Qt::AlignCenter);
    m_weaponIconLabel->setStyleSheet(QStringLiteral("background: transparent;"));
    rightLayout->addWidget(m_weaponIconLabel, 0, Qt::AlignCenter);
    rightLayout->addStretch();

    m_leftHudPanel = new QWidget(this);
    m_leftHudPanel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_leftHudPanel->setStyleSheet(QStringLiteral(
        "background: rgba(20,24,35,0.88); border: 1px solid rgba(60,70,90,0.6); border-radius: 10px;"));
    auto *leftLayout = new QVBoxLayout(m_leftHudPanel);
    leftLayout->setContentsMargins(10, 10, 10, 10);
    leftLayout->setSpacing(8);

    m_dashCooldownWidget = new DashCooldownWidget(m_leftHudPanel);
    leftLayout->addWidget(m_dashCooldownWidget, 0, Qt::AlignCenter);

    m_leftExpBar = new QProgressBar(m_leftHudPanel);
    m_leftExpBar->setRange(0, 100);
    m_leftExpBar->setValue(0);
    m_leftExpBar->setFormat(QStringLiteral("经验 0/0"));
    m_leftExpBar->setTextVisible(true);
    m_leftExpBar->setFixedHeight(18);
    m_leftExpBar->setStyleSheet(QStringLiteral(
        "QProgressBar { border: 1px solid #3a4352; border-radius: 4px; background: rgba(21,24,29,0.8);"
        "color: #d6dff0; font-size: 11px; font-weight: 600; }"
        "QProgressBar::chunk { border-radius: 3px; background-color: #f7b731; }"));
    leftLayout->addWidget(m_leftExpBar);

    auto *traitTag = new QLabel(QStringLiteral("特性"), m_leftHudPanel);
    traitTag->setStyleSheet(QStringLiteral("color: #93a0b4; font-size: 10px; background: transparent;"));
    leftLayout->addWidget(traitTag);

    m_traitIconsLayout = new QHBoxLayout();
    m_traitIconsLayout->setSpacing(3);
    leftLayout->addLayout(m_traitIconsLayout);
    leftLayout->addStretch();

    m_waveProgressBar = new QProgressBar(this);
    m_waveProgressBar->setObjectName(QStringLiteral("waveProgressBar"));
    m_waveProgressBar->setRange(0, GameConfig::kWaveConfig.roundDurationMs);
    m_waveProgressBar->setValue(0);
    m_waveProgressBar->setTextVisible(true);
    m_waveProgressBar->setStyleSheet(QStringLiteral(
        "QProgressBar { border: 1px solid #3a4352; border-radius: 6px; background: rgba(21,24,29,0.85);"
        "color: #f6f8fb; text-align: center; min-height:22px; }"
        "QProgressBar::chunk { border-radius: 5px; background-color: #4b7bec; }"));

    m_upgradeOverlay = new QWidget(this);
    m_upgradeOverlay->setVisible(false);
    m_upgradeOverlay->setStyleSheet(
        QStringLiteral("background: rgba(10, 12, 18, 0.88);"));
    auto *upgradeRoot = new QVBoxLayout(m_upgradeOverlay);
    upgradeRoot->setAlignment(Qt::AlignCenter);

    auto *upgradeTitle = new QLabel(QStringLiteral("选择升级"), m_upgradeOverlay);
    upgradeTitle->setStyleSheet(
        QStringLiteral("color: white; font-size: 28px; font-weight: 700; background: transparent;"));
    upgradeTitle->setAlignment(Qt::AlignCenter);
    upgradeRoot->addWidget(upgradeTitle);

    m_upgradeCardsGrid = new QGridLayout();
    m_upgradeCardsGrid->setSpacing(14);
    m_upgradeCardsGrid->setContentsMargins(60, 24, 60, 24);
    upgradeRoot->addLayout(m_upgradeCardsGrid);

    m_upgradeConfirmButton = new QPushButton(QStringLiteral("确认选择"), m_upgradeOverlay);
    m_upgradeConfirmButton->setFixedSize(220, 46);
    m_upgradeConfirmButton->setEnabled(false);
    m_upgradeConfirmButton->setStyleSheet(QStringLiteral(
        "QPushButton { background: #335b9d; color: white; border: 1px solid #6b88b8;"
        "border-radius: 10px; font-size: 16px; font-weight: 700; }"
        "QPushButton:hover { background: #4472bc; }"
        "QPushButton:disabled { background: #28303b; color: #7b8798; border-color: #485261; }"));
    upgradeRoot->addWidget(m_upgradeConfirmButton, 0, Qt::AlignCenter);

    connect(m_upgradeConfirmButton, &QPushButton::clicked, this, &GameMainPage::confirmUpgrade);

    m_gameLoopTimer = new QTimer(this);
    m_gameLoopTimer->setInterval(GameConfig::kWaveConfig.updateIntervalMs);
    connect(m_gameLoopTimer, &QTimer::timeout, this, &GameMainPage::handleBattleTick);

    m_waveManager = createWaveManager(this);
    connect(m_waveManager, &WaveManager::battleStateChanged, this, [this](BattleFlowState state) {
        setBattleState(state);
        if (state != BattleFlowState::Battle) {
            if (m_weapon != nullptr) {
                m_weapon->stopFiring();
            }
            if (m_gameLoopTimer != nullptr) {
                m_gameLoopTimer->stop();
            }
        }
        updateStatusText();
    });
    connect(m_waveManager, &WaveManager::roundChanged, this, [this](int) {
        updateStatusText();
    });
    connect(m_waveManager, &WaveManager::experienceChanged, this, [this](int, int, int) {
        updateStatusText();
    });
    connect(m_waveManager, &WaveManager::roundCompleted, this, [this](int roundIndex) {
        m_enemySpawnAccumulatorMs = 0.0F;
        if (m_waveManager != nullptr && roundIndex >= m_waveManager->config().maxRounds) {
            emit battleFinished(true);
        }
        updateStatusText();
    });
    connect(m_waveManager, &WaveManager::upgradeRequested, this, [this]() {
        setBattleState(BattleFlowState::Upgrade);
        setBattleActive(false);
        QTimer::singleShot(0, this, [this]() {
            showUpgradeOverlay();
        });
    });

    m_upgradeResolver = new UpgradeResolver(nullptr, nullptr, nullptr, this);
    connect(m_upgradeResolver, &UpgradeResolver::statsChanged, this, &GameMainPage::statsChanged);
    m_combatCoordinator = new CombatCoordinator(nullptr, m_scene, nullptr, nullptr, this);
    m_enemyDirector = new EnemyDirector(nullptr, m_scene, this, this);

    updateHealthBarStyle(1.0F);
    setupBgMusic();

    m_hitSoundPlayer = new QMediaPlayer(this);
    m_hitSoundOutput = new QAudioOutput(this);
    m_hitSoundOutput->setVolume(0.7F);
    m_hitSoundPlayer->setAudioOutput(m_hitSoundOutput);
    m_hitSoundPlayer->setSource(QUrl(QStringLiteral("qrc:/musics/bullet_in_normal.mp3")));

    m_killSoundPlayer = new QMediaPlayer(this);
    m_killSoundOutput = new QAudioOutput(this);
    m_killSoundOutput->setVolume(0.8F);
    m_killSoundPlayer->setAudioOutput(m_killSoundOutput);
    m_killSoundPlayer->setSource(QUrl(QStringLiteral("qrc:/musics/bullet_in_kill.mp3")));

    updateStatusText();
}

bool GameMainPage::eventFilter(QObject *watched, QEvent *event)
{
    if ((event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
        && isVisible()
        && m_battleState == BattleFlowState::Battle
        && m_player != nullptr) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (isMovementKey(keyEvent->key())) {
            if (keyEvent->isAutoRepeat()) {
                return true;
            }

            const bool pressed = event->type() == QEvent::KeyPress;
            switch (keyEvent->key()) {
            case Qt::Key_W:
                m_moveUpPressed = pressed;
                break;
            case Qt::Key_S:
                m_moveDownPressed = pressed;
                break;
            case Qt::Key_A:
                m_moveLeftPressed = pressed;
                break;
            case Qt::Key_D:
                m_moveRightPressed = pressed;
                break;
            default:
                break;
            }

            updateInputDirection();
            return true;
        }

        if (keyEvent->key() == Qt::Key_Space && !keyEvent->isAutoRepeat()) {
            if (keyEvent->type() == QEvent::KeyPress && m_battleState == BattleFlowState::Battle) {
                m_player->dash();
                return true;
            }
        }
    }

    if (watched == this && (event->type() == QEvent::Hide || event->type() == QEvent::WindowDeactivate)) {
        m_moveUpPressed = false;
        m_moveDownPressed = false;
        m_moveLeftPressed = false;
        m_moveRightPressed = false;
        m_firePressed = false;
        updateInputDirection();
        if (m_weapon != nullptr) {
            m_weapon->stopFiring();
        }
    }

    if (m_view != nullptr && watched == m_view->viewport()) {
        switch (event->type()) {
        case QEvent::MouseMove: {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            m_mouseScenePosition = m_view->mapToScene(mouseEvent->position().toPoint());
            updateWeaponAim();
            return true;
        }
        case QEvent::MouseButtonPress: {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() != Qt::LeftButton) {
                break;
            }

            m_view->setFocus(Qt::MouseFocusReason);
            m_mouseScenePosition = m_view->mapToScene(mouseEvent->position().toPoint());
            m_firePressed = true;
            updateWeaponAim();
            if (m_weapon != nullptr) {
                m_weapon->startFiring();
            }
            return true;
        }
        case QEvent::MouseButtonRelease: {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() != Qt::LeftButton) {
                break;
            }

            m_firePressed = false;
            if (m_weapon != nullptr) {
                m_weapon->stopFiring();
            }
            updatePlayerVisualState();
            return true;
        }
        default:
            break;
        }
    }

    // Handle mouse click on upgrade cards
    if (event->type() == QEvent::MouseButtonPress) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QWidget *cardWidget = qobject_cast<QWidget *>(watched);
            if (cardWidget != nullptr) {
                QVariant prop = cardWidget->property("cardIndex");
                if (prop.isValid()) {
                    onUpgradeCardClicked(prop.toInt());
                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void GameMainPage::setFactory(GameFactory *factory)
{
    m_factory = factory;
    if (m_hasSelectedClass) {
        rebuildBattleScene();
    }
}

void GameMainPage::setSelectedClass(PlayerClassId classId)
{
    m_selectedClassId = classId;
    m_hasSelectedClass = true;
    rebuildBattleScene();
}

void GameMainPage::setBattleActive(bool active)
{
    if (m_gameLoopTimer == nullptr) {
        return;
    }

    if (!active) {
        m_moveUpPressed = false;
        m_moveDownPressed = false;
        m_moveLeftPressed = false;
        m_moveRightPressed = false;
        m_firePressed = false;
        updateInputDirection();
        if (m_weapon != nullptr) {
            m_weapon->stopFiring();
        }
        m_gameLoopTimer->stop();
        if (m_waveManager != nullptr && m_battleState != BattleFlowState::Upgrade) {
            m_waveManager->stopBattle();
        }
        if (m_battleState != BattleFlowState::Upgrade) {
            setBattleState(BattleFlowState::Inactive);
        }
        updatePlayerVisualState();
        updateStatusText();
        return;
    }

    if (m_player == nullptr || (m_player != nullptr && m_player->currentHealth() <= 0.0F)) {
        updateStatusText();
        return;
    }

    if (!m_gameLoopTimer->isActive()) {
        m_gameLoopTimer->start();
    }
    if (m_battleState != BattleFlowState::Upgrade && m_waveManager != nullptr) {
        m_waveManager->startBattle();
        setBattleState(m_waveManager->battleState());
    }
    if (m_view != nullptr) {
        m_view->setFocus(Qt::OtherFocusReason);
    }
    updateStatusText();
}

void GameMainPage::enterUpgradeState()
{
    if (m_battleState == BattleFlowState::Upgrade) {
        return;
    }

    if (m_waveManager != nullptr) {
        m_waveManager->enterUpgrade();
    }
    setBattleActive(false);
    setBattleState(BattleFlowState::Upgrade);
}

void GameMainPage::resumeBattleState()
{
    if (m_waveManager != nullptr) {
        m_waveManager->resumeBattleFromUpgrade();
        setBattleState(m_waveManager->battleState());
    } else {
        setBattleState(BattleFlowState::Battle);
    }
    setBattleActive(true);
}

BattleFlowState GameMainPage::battleState() const noexcept
{
    return m_battleState;
}

WaveManager *GameMainPage::waveManager() const noexcept
{
    return m_waveManager;
}

WeaponId GameMainPage::currentWeaponId() const noexcept
{
    if (m_player != nullptr) {
        return m_player->weaponId();
    }

    const PlayerClassConfig *config = GameConfig::findPlayerClassConfig(m_selectedClassId);
    return config != nullptr ? config->starterWeaponId : WeaponId::PeaShooter;
}

void GameMainPage::applyTrait(TraitId traitId)
{
    if (m_upgradeResolver != nullptr) {
        m_upgradeResolver->applyTrait(traitId);
    }
    updatePlayerVisualState();
    updateStatusText();
}

void GameMainPage::applyWeaponUpgrade(WeaponUpgradeId upgradeId)
{
    if (m_upgradeResolver != nullptr) {
        m_upgradeResolver->applyWeaponUpgrade(upgradeId);
    }
    updateStatusText();
}

void GameMainPage::setActiveBulletStyle(BulletStyle style)
{
    m_activeBulletStyle = style;
    if (m_combatCoordinator != nullptr) {
        m_combatCoordinator->setActiveBulletStyle(style);
    }
}

BulletStyle GameMainPage::activeBulletStyle() const noexcept
{
    return m_activeBulletStyle;
}

void GameMainPage::resolveCombatCollisions()
{
    if (m_combatCoordinator == nullptr) {
        return;
    }
    m_combatCoordinator->resolveCombatCollisions(
        m_bullets, m_enemies, m_playerMarker,
        m_playerDamageCooldownRemainingMs,
        m_upgradeResolver != nullptr ? m_upgradeResolver->traitCounts() : QMap<TraitId, int>{});
}

void GameMainPage::setBattleState(BattleFlowState state)
{
    if (m_battleState == state) {
        return;
    }

    m_battleState = state;
    emit battleStateChanged(m_battleState);
}

void GameMainPage::rebuildBattleScene()
{
    clearBattleScene();

    if (m_factory == nullptr || !m_hasSelectedClass) {
        updateStatusText();
        return;
    }

    m_player = m_factory->createPlayer(m_selectedClassId, this);
    if (m_player == nullptr) {
        updateStatusText();
        return;
    }

    m_player->setWorldPosition(QPointF(0.0, 0.0));
    m_player->setMoveDirection(QPointF());
    m_mouseScenePosition = m_player->worldPosition() + QPointF(80.0, 0.0);
    m_enemySpawnAccumulatorMs = 0.0F;
    m_playerDamageCooldownRemainingMs = 0.0F;
    updateInputDirection();

    if (m_upgradeResolver != nullptr) {
        m_upgradeResolver->deleteLater();
    }
    m_upgradeResolver = new UpgradeResolver(m_player, nullptr, m_factory, this);
    m_upgradeResolver->setClassId(m_selectedClassId);
    connect(m_upgradeResolver, &UpgradeResolver::statsChanged, this, &GameMainPage::statsChanged);
    connect(m_upgradeResolver, &UpgradeResolver::statsChanged, this, [this]() {
        updateStatusText();
    });

    if (m_playerMarker != nullptr) {
        m_playerMarker->setPos(m_player->worldPosition());
        m_playerMarker->setVisible(true);
    }
    if (m_playerAvatar != nullptr) {
        m_playerAvatar->setPos(m_player->worldPosition());
        m_playerAvatar->setVisible(true);
        switch (m_selectedClassId) {
        case PlayerClassId::Warrior:
            m_playerAvatar->setAimArrow(QStringLiteral(":/weapon/crossed_swords_3d.png"));
            break;
        case PlayerClassId::Ranger:
            m_playerAvatar->setAimArrow(QStringLiteral(":/weapon/bow_and_arrow_3d.png"));
            break;
        case PlayerClassId::Caster:
            m_playerAvatar->setAimArrow(QStringLiteral(":/weapon/magic_wand_3d.png"));
            break;
        }
    }

    connect(m_player, &Player::moved, this, [this](const QPointF &position) {
        if (m_playerMarker != nullptr) {
            m_playerMarker->setPos(position);
        }
        if (m_playerAvatar != nullptr) {
            m_playerAvatar->setPos(position);
        }
        updatePlayerVisualState();
    });
    connect(m_player, &Player::healthChanged, this, [this](float, float) {
        updatePlayerVisualState();
        updateStatusText();
    });
    connect(m_player, &Player::defeated, this, [this]() {
        if (m_weapon != nullptr) {
            m_weapon->stopFiring();
        }
        if (m_waveManager != nullptr) {
            m_waveManager->stopBattle();
        }
        m_gameLoopTimer->stop();
        updatePlayerVisualState();
        updateStatusText();
        emit battleFinished(false);
    });

    connect(m_player, &Player::dashCooldownChanged, m_dashCooldownWidget, &DashCooldownWidget::updateCooldown);
    m_dashCooldownWidget->updateCooldown(m_player->dashCooldownRemaining(), m_player->dashCooldownTotal());

    m_weapon = m_factory->createStarterWeapon(m_selectedClassId, m_player, this);
    if (m_weapon != nullptr) {
        connect(m_weapon, &Weapon::fireRequested, this,
                [this](WeaponId weaponId, const QPointF &origin, const QPointF &direction) {
                    if (m_combatCoordinator != nullptr) {
                        m_combatCoordinator->handleWeaponFireRequested(weaponId, origin, direction, m_bullets);
                    }
                });
        connect(m_weapon, &Weapon::cooldownChanged, this, [this](float) {
            updateStatusText();
        });
        updateWeaponAim();
    }

    if (m_combatCoordinator != nullptr) {
        m_combatCoordinator->deleteLater();
    }
    m_combatCoordinator = new CombatCoordinator(m_factory, m_scene, m_player, m_weapon, this);

    if (m_enemyDirector != nullptr) {
        m_enemyDirector->deleteLater();
    }
    m_enemyDirector = new EnemyDirector(m_factory, m_scene, this, this);
    m_enemyDirector->setStateVectors(&m_enemies, &m_bullets);

    if (m_waveManager != nullptr) {
        m_waveManager->resetRun();
    }

    m_backgroundTechActive = false;
    m_demonLordDefeated = false;
    m_currentNormalBgm = QStringLiteral("sery1_normal");
    updateBattleBackground();
    switchBgMusic(QStringLiteral("sery1_normal"));

    for (int index = 0; index < GameConfig::kWaveConfig.initialEnemyCount; ++index) {
        m_enemyDirector->spawnTestEnemy(GameConfig::kWaveConfig.maxConcurrentEnemies,
                                        m_bossIsActive, m_currentBossId);
    }

    updatePlayerVisualState();
    updateStatusText();
    if (m_waveManager != nullptr) {
        m_waveManager->startBattle();
        setBattleState(m_waveManager->battleState());
    } else {
        setBattleState(BattleFlowState::Battle);
    }
    m_gameLoopTimer->start();
    m_view->setFocus(Qt::OtherFocusReason);
}

void GameMainPage::clearBattleScene()
{
    m_gameLoopTimer->stop();
    m_bossIsActive = false;
    m_bossSpawnPosition = QPointF();
    m_enemySpawnAccumulatorMs = 0.0F;
    m_playerDamageCooldownRemainingMs = 0.0F;
    m_inputDirection = QPointF();
    m_moveUpPressed = false;
    m_moveDownPressed = false;
    m_moveLeftPressed = false;
    m_moveRightPressed = false;
    m_firePressed = false;

    clearActiveBullets();
    clearActiveEnemies(false);

    if (m_weapon != nullptr) {
        m_weapon->stopFiring();
        m_weapon->deleteLater();
        m_weapon = nullptr;
    }

    if (m_player != nullptr) {
        m_player->deleteLater();
        m_player = nullptr;
    }

    if (m_playerMarker != nullptr) {
        m_playerMarker->setVisible(false);
        m_playerMarker->setPos(QPointF());
    }
    if (m_playerAvatar != nullptr) {
        m_playerAvatar->setVisible(false);
        m_playerAvatar->setPos(QPointF());
    }

    if (m_waveManager != nullptr) {
        m_waveManager->stopBattle();
    } else {
        setBattleState(BattleFlowState::Inactive);
    }
}

void GameMainPage::handleBattleTick()
{
    if (m_player == nullptr) {
        m_gameLoopTimer->stop();
        return;
    }

    const WaveConfig &waveConfig = m_waveManager != nullptr ? m_waveManager->config() : GameConfig::kWaveConfig;
    if (m_waveManager != nullptr && m_waveManager->battleState() != BattleFlowState::Battle) {
        m_gameLoopTimer->stop();
        updateStatusText();
        return;
    }

    const float deltaSeconds = static_cast<float>(waveConfig.updateIntervalMs) / 1000.0F;
    m_playerDamageCooldownRemainingMs = std::max(
        0.0F,
        m_playerDamageCooldownRemainingMs - static_cast<float>(waveConfig.updateIntervalMs));

    updatePlayerMovement(deltaSeconds);
    if (m_inputDirection != QPointF(0.0F, 0.0F)) {
        m_lastMoveDirection = m_inputDirection;
    }
    updateWeaponAim();

    if (m_weapon != nullptr) {
        if (m_firePressed && !m_weapon->isFiring()) {
            m_weapon->startFiring();
        } else if (!m_firePressed && m_weapon->isFiring()) {
            m_weapon->stopFiring();
        }

        m_weapon->advanceCooldown(static_cast<float>(waveConfig.updateIntervalMs));
    }

    m_enemyDirector->spawnBossIfPending(m_waveManager, m_bossIsActive, m_currentBossId);

    if (m_bossIsActive) {
        if (m_currentBossId == EnemyId::DemonLord && !m_demonLordDefeated) {
            switchBgMusic(QStringLiteral("sery1-boss (mp3cut)"));
        } else if (m_currentBossId == EnemyId::BoneLord) {
            switchBgMusic(QStringLiteral("sery2_boss"));
        } else if (m_currentBossId == EnemyId::UFO || m_currentBossId == EnemyId::AlienPilot) {
            switchBgMusic(QStringLiteral("ser3_alien_boss"));
        }
    }

    const int elapsedRoundMs = m_waveManager != nullptr ? m_waveManager->elapsedRoundMs() : 0;
    if (elapsedRoundMs < waveConfig.roundDurationMs) {
        m_enemySpawnAccumulatorMs += static_cast<float>(waveConfig.updateIntervalMs);
        while (m_enemySpawnAccumulatorMs >= static_cast<float>(waveConfig.enemySpawnIntervalMs)
               && m_enemies.size() < waveConfig.maxConcurrentEnemies) {
            m_enemyDirector->spawnTestEnemy(waveConfig.maxConcurrentEnemies,
                                            m_bossIsActive, m_currentBossId);
            m_enemySpawnAccumulatorMs -= static_cast<float>(waveConfig.enemySpawnIntervalMs);
        }
    }

    for (const auto &enemy : std::as_const(m_enemies)) {
        if (enemy.data == nullptr || enemy.data->isDefeated()) {
            continue;
        }
        enemy.data->updateAI(deltaSeconds, m_player->worldPosition());
        enemy.data->advanceFrame(deltaSeconds);
        enemy.view->syncFromData();
    }

    for (const auto &bullet : std::as_const(m_bullets)) {
        if (bullet.data == nullptr || bullet.data->isExpired()) {
            continue;
        }
        bullet.data->advanceFrame(deltaSeconds);
        const QRectF sceneBounds = m_scene->sceneRect().adjusted(-40, -40, 40, 40);
        if (!sceneBounds.contains(bullet.data->worldPosition())) {
            bullet.data->expire();
        }
    }

    const QMap<TraitId, int> traitCounts = m_upgradeResolver != nullptr
        ? m_upgradeResolver->traitCounts()
        : QMap<TraitId, int>{};

    const int enemiesBeforeKill = static_cast<int>(m_enemies.size());

    QVector<float> enemyHpBefore;
    enemyHpBefore.reserve(m_enemies.size());
    for (const auto &enemy : std::as_const(m_enemies)) {
        enemyHpBefore.append(enemy.data != nullptr ? enemy.data->currentHealth() : -1.0F);
    }

    const float playerHpBefore = m_player != nullptr ? m_player->currentHealth() : 0.0F;
    m_combatCoordinator->resolveCombatCollisions(m_bullets, m_enemies, m_playerMarker,
                                                  m_playerDamageCooldownRemainingMs, traitCounts);

    if (m_player != nullptr && m_player->currentHealth() < playerHpBefore - 0.001F) {
        m_playerDamageCooldownRemainingMs = std::max(m_playerDamageCooldownRemainingMs, kPlayerDamageCooldownMs);
    }

    bool anyEnemyHit = false;
    for (int i = 0; i < m_enemies.size() && i < enemyHpBefore.size(); ++i) {
        if (enemyHpBefore[i] < 0.0F) continue;
        if (m_enemies[i].data == nullptr || m_enemies[i].data->isDefeated()) continue;
        if (m_enemies[i].data->currentHealth() < enemyHpBefore[i] - 0.001F) {
            anyEnemyHit = true;
            break;
        }
    }
    if (anyEnemyHit) {
        playHitSound();
    }

    m_combatCoordinator->cleanupExpiredBullets(m_bullets);
    m_combatCoordinator->cleanupDefeatedEnemies(m_enemies, m_waveManager, traitCounts);

    if (static_cast<int>(m_enemies.size()) < enemiesBeforeKill) {
        playKillSound();
    }

    checkBossDefeat();

    if (m_waveManager != nullptr) {
        m_waveManager->advanceFrame(waveConfig.updateIntervalMs);
        if (m_waveManager->battleState() != BattleFlowState::Battle) {
            updatePlayerVisualState();
            updateStatusText();
            return;
        }
        if (m_waveManager->elapsedRoundMs() >= waveConfig.roundDurationMs && m_enemies.isEmpty()) {
            m_waveManager->completeCurrentRound();
        }
    }

    if (m_waveManager != nullptr && m_waveManager->currentRound() >= 8 && !m_backgroundTechActive) {
        m_backgroundTechActive = true;
        updateBattleBackground();
        m_currentNormalBgm = QStringLiteral("ser3_alien");
        switchBgMusic(QStringLiteral("ser3_alien"));
    }

    updatePlayerVisualState();
    updateStatusText();
}

void GameMainPage::clearActiveBullets()
{
    for (const auto &bullet : std::as_const(m_bullets)) {
        if (bullet.view != nullptr) {
            m_scene->removeItem(bullet.view);
            bullet.view->deleteLater();
        }
        if (bullet.data != nullptr) {
            bullet.data->deleteLater();
        }
    }
    m_bullets.clear();
}

void GameMainPage::clearActiveEnemies(bool grantExperience)
{
    for (const auto &enemy : std::as_const(m_enemies)) {
        if (grantExperience && enemy.data != nullptr && m_waveManager != nullptr && !enemy.data->isDefeated()) {
            m_waveManager->addExperience(GameConfig::kWaveConfig.experiencePerEnemyDefeat);
        }
        if (enemy.view != nullptr) {
            m_scene->removeItem(enemy.view);
            enemy.view->deleteLater();
        }
        if (enemy.data != nullptr) {
            enemy.data->deleteLater();
        }
    }
    m_enemies.clear();
}

void GameMainPage::updateInputDirection()
{
    const qreal horizontal = (m_moveRightPressed ? 1.0 : 0.0) - (m_moveLeftPressed ? 1.0 : 0.0);
    const qreal vertical = (m_moveDownPressed ? 1.0 : 0.0) - (m_moveUpPressed ? 1.0 : 0.0);
    m_inputDirection = normalizedVector(QPointF(horizontal, vertical), QPointF());

    if (m_player != nullptr) {
        m_player->setMoveDirection(m_inputDirection);
    }
}

void GameMainPage::updatePlayerMovement(float deltaSeconds)
{
    if (m_player == nullptr || deltaSeconds <= 0.0F) {
        return;
    }

    QPointF finalStep;
    if (m_player->isDashing()) {
        finalStep = m_lastMoveDirection * (m_player->moveSpeed() * 4.0F * deltaSeconds);
    } else {
        if (qFuzzyIsNull(QLineF(QPointF(), m_inputDirection).length())) {
            m_player->updateDash(deltaSeconds);
            return;
        }
        finalStep = m_inputDirection * (m_player->moveSpeed() * deltaSeconds);
    }

    m_player->setWorldPosition(m_player->worldPosition() + finalStep);
    expandSceneToFitPlayer(m_player->worldPosition());
    m_player->updateDash(deltaSeconds);
}

void GameMainPage::updateWeaponAim()
{
    if (m_player == nullptr || m_weapon == nullptr) {
        return;
    }

    const QPointF aimDirection = normalizedVector(m_mouseScenePosition - m_player->worldPosition());
    m_weapon->setAimDirection(aimDirection);
    updatePlayerVisualState();
}

void GameMainPage::updatePlayerVisualState()
{
    if (m_playerAvatar == nullptr || m_player == nullptr) {
        return;
    }

    m_playerAvatar->setPos(m_player->worldPosition());
    m_playerAvatar->setAimDirection(m_mouseScenePosition - m_player->worldPosition());
    m_playerAvatar->setHitFlash(m_playerDamageCooldownRemainingMs <= 0.0F
                                    ? 0.0
                                    : m_playerDamageCooldownRemainingMs / kPlayerDamageCooldownMs);

    const float healthRatio = m_player->currentHealth() / std::max(1.0F, m_player->maxHealth());
    m_playerAvatar->setHealthRatio(healthRatio);
    m_playerAvatar->setIsDashing(m_player->isDashing());
    m_playerAvatar->setIsHurt(m_playerDamageCooldownRemainingMs > 0.0F);
}

void GameMainPage::updateHealthBarStyle(float healthRatio)
{
    QString chunkColor = QStringLiteral("#32c766");
    if (healthRatio < 0.25F) {
        chunkColor = QStringLiteral("#e05a5a");
    } else if (healthRatio < 0.5F) {
        chunkColor = QStringLiteral("#f0c24b");
    }

    m_healthBar->setStyleSheet(QStringLiteral(
        "QProgressBar {"
        "border: 1px solid #3a4352;"
        "border-radius: 6px;"
        "background-color: #15181d;"
        "color: #f6f8fb;"
        "min-height: 18px;"
        "text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "border-radius: 5px;"
        "background-color: %1;"
        "}").arg(chunkColor));
}

void GameMainPage::expandSceneToFitPlayer(const QPointF &position) const
{
    if (m_scene == nullptr) {
        return;
    }

    QRectF expandedRect = m_scene->sceneRect();
    expandedRect = expandedRect.united(QRectF(position.x() - 520.0, position.y() - 320.0, 1040.0, 640.0));
    expandedRect = expandedRect.intersected(QRectF(-kMaxWorldWidth / 2.0, -kMaxWorldHeight / 2.0,
                                                    kMaxWorldWidth, kMaxWorldHeight));
    if (expandedRect != m_scene->sceneRect()) {
        m_scene->setSceneRect(expandedRect);
    }
}

QPointF GameMainPage::bossSpawnPosition() const
{
    return m_bossSpawnPosition;
}

void GameMainPage::applyDynamicDifficulty(int wave, EnemyData *enemy)
{
    if (enemy == nullptr) return;
    Q_UNUSED(wave);
}

void GameMainPage::updateStatusText()
{
    if (m_factory == nullptr || !m_hasSelectedClass || m_player == nullptr) {
        m_healthBar->setValue(0);
        m_healthBar->setFormat(QStringLiteral("0 / 0"));
        updateHealthBarStyle(1.0F);
        m_levelLabel->setText(QStringLiteral("Lv.0"));
        m_expBar->setValue(0);
        m_leftExpBar->setValue(0);
        m_leftExpBar->setFormat(QStringLiteral("经验 0/0"));
        m_waveProgressBar->setValue(0);
        m_waveProgressBar->setFormat(QString());
        return;
    }

    const auto currentHealth = m_player->currentHealth();
    const auto maxHealth = std::max(1.0F, m_player->maxHealth());
    const auto healthRatio = currentHealth / maxHealth;
    const int currentRound = m_waveManager != nullptr ? m_waveManager->currentRound() : 1;
    const int currentLevel = m_waveManager != nullptr ? m_waveManager->currentLevel() : 0;
    const int currentExperience = m_waveManager != nullptr ? m_waveManager->currentExperience() : 0;
    const int experienceToNext = m_waveManager != nullptr ? m_waveManager->experienceToNextLevel() : 0;
    const int elapsedRoundMs = m_waveManager != nullptr ? m_waveManager->elapsedRoundMs() : 0;
    const int prevLevelExp = currentLevel >= 1 ? GameConfig::experienceThresholdForLevel(currentLevel - 1) : 0;
    const int currLevelRequired = experienceToNext;
    const int levelProgress = std::max(0, currentExperience - prevLevelExp);
    const int enemyCount = static_cast<int>(m_enemies.size());
    const bool clearingRemaining = elapsedRoundMs >= GameConfig::kWaveConfig.roundDurationMs && enemyCount > 0;

    m_healthBar->setRange(0, static_cast<int>(std::ceil(maxHealth)));
    m_healthBar->setValue(static_cast<int>(std::round(currentHealth)));
    m_healthBar->setFormat(QStringLiteral("%1 / %2")
                               .arg(QString::number(currentHealth, 'f', 0))
                               .arg(QString::number(maxHealth, 'f', 0)));
    updateHealthBarStyle(healthRatio);

    m_levelLabel->setText(QStringLiteral("Lv.%1").arg(QString::number(currentLevel)));

    if (currLevelRequired > 0) {
        m_expBar->setRange(0, currLevelRequired);
        m_expBar->setValue(std::min(levelProgress, currLevelRequired));
        m_expBar->setFormat(QStringLiteral("\u7ecf\u9a8c %1/%2").arg(levelProgress).arg(currLevelRequired));
    } else {
        m_expBar->setRange(0, 1);
        m_expBar->setValue(1);
        m_expBar->setFormat(QStringLiteral("\u5df2\u6ee1\u7ea7"));
    }
    m_expBar->setVisible(true);

    if (m_weaponIconLabel != nullptr) {
        QString weaponIconPath;
        switch (m_selectedClassId) {
        case PlayerClassId::Warrior:
            weaponIconPath = QStringLiteral(":/weapon/crossed_swords_3d.png");
            break;
        case PlayerClassId::Ranger:
            weaponIconPath = QStringLiteral(":/weapon/bow_and_arrow_3d.png");
            break;
        case PlayerClassId::Caster:
            weaponIconPath = QStringLiteral(":/weapon/magic_wand_3d.png");
            break;
        }
        QPixmap wepPix(weaponIconPath);
        if (!wepPix.isNull()) {
            m_weaponIconLabel->setPixmap(wepPix.scaled(44, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }

    if (currLevelRequired > 0) {
        m_leftExpBar->setRange(0, currLevelRequired);
        m_leftExpBar->setValue(std::min(levelProgress, currLevelRequired));
        m_leftExpBar->setFormat(QStringLiteral("\u7ecf\u9a8c %1/%2").arg(levelProgress).arg(currLevelRequired));
    } else {
        m_leftExpBar->setRange(0, 1);
        m_leftExpBar->setValue(1);
        m_leftExpBar->setFormat(QStringLiteral("\u5df2\u6ee1\u7ea7"));
    }

    // Rebuild trait icons
    QLayoutItem *traitItem;
    while ((traitItem = m_traitIconsLayout->takeAt(0)) != nullptr) {
        if (traitItem->widget() != nullptr) {
            traitItem->widget()->deleteLater();
        }
        delete traitItem;
    }
    const QList<TraitId> ownedTraits = m_upgradeResolver != nullptr
        ? m_upgradeResolver->ownedTraits()
        : QList<TraitId>{};
    for (const TraitId tid : ownedTraits) {
        const TraitConfig *cfg = GameConfig::findTraitConfig(tid);
        if (cfg == nullptr) continue;
        const QString iconPath = traitIconForId(tid);
        if (iconPath.isEmpty()) continue;
        QPixmap icon(iconPath);
        if (icon.isNull()) continue;
        auto *iconLabel = new QLabel(m_leftHudPanel);
        iconLabel->setFixedSize(30, 30);
        iconLabel->setPixmap(icon.scaled(28, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        iconLabel->setToolTip(cfg->displayName);
        iconLabel->setStyleSheet(QStringLiteral("background: transparent;"));
        m_traitIconsLayout->addWidget(iconLabel);
    }
    m_traitIconsLayout->addStretch();

    m_waveProgressBar->setRange(0, GameConfig::kWaveConfig.roundDurationMs);
    m_waveProgressBar->setFormat(clearingRemaining
                                     ? QStringLiteral("\u7b2c%1/%2\u6ce2 \u6e05\u573a\u4e2d\u2026")
                                           .arg(QString::number(currentRound),
                                                QString::number(GameConfig::kWaveConfig.maxRounds))
                                     : QStringLiteral("\u7b2c%1/%2\u6ce2 %p%")
                                           .arg(QString::number(currentRound),
                                                QString::number(GameConfig::kWaveConfig.maxRounds)));
    m_waveProgressBar->setValue(std::min(elapsedRoundMs, GameConfig::kWaveConfig.roundDurationMs));
    m_waveProgressBar->setVisible(true);

    positionHudElements();
}

void GameMainPage::setupBgMusic()
{
    m_audioOutput = new QAudioOutput(this);
    m_audioOutput->setVolume(0.5F);
    m_bgMusicPlayer = new QMediaPlayer(this);
    m_bgMusicPlayer->setAudioOutput(m_audioOutput);
    m_bgMusicPlayer->setLoops(QMediaPlayer::Infinite);
}

void GameMainPage::updateBattleBackground()
{
    const QString imageName = m_backgroundTechActive
        ? QStringLiteral("background_images/tech.jpg")
        : QStringLiteral("background_images/magic.jpg");

    const QString resourcePath = QStringLiteral(":/%1").arg(imageName);
    const QPixmap bg(resourcePath);
    if (m_view != nullptr) {
        m_view->setBackgroundImage(bg);
    }
}

void GameMainPage::switchBgMusic(const QString &trackName)
{
    if (m_bgMusicPlayer == nullptr) {
        return;
    }

    const QString resourcePath = QStringLiteral("qrc:/musics/%1.mp3").arg(trackName);
    const QUrl url(resourcePath);

    if (m_bgMusicPlayer->source() == url && m_bgMusicPlayer->playbackState() == QMediaPlayer::PlayingState) {
        return;
    }

    m_bgMusicPlayer->stop();
    m_bgMusicPlayer->setSource(url);
    m_bgMusicPlayer->play();
}

void GameMainPage::checkBossDefeat()
{
    if (!m_bossIsActive) {
        return;
    }

    bool bossAlive = false;
    for (const auto &enemy : std::as_const(m_enemies)) {
        if (enemy.data != nullptr && !enemy.data->isDefeated()
            && enemy.data->id() == m_currentBossId) {
            bossAlive = true;
            break;
        }
    }

    if (bossAlive) {
        return;
    }

    const EnemyId defeatedBossId = m_currentBossId;

    if (m_waveManager != nullptr) {
        m_waveManager->notifyBossDefeated();
    }

    m_bossIsActive = false;
    m_currentBossId = EnemyId::Ogre;

    switch (defeatedBossId) {
    case EnemyId::DemonLord:
        if (!m_demonLordDefeated) {
            m_demonLordDefeated = true;
            m_currentNormalBgm = QStringLiteral("sery2_skeleon (mp3cut)");
            switchBgMusic(m_currentNormalBgm);
        }
        break;
    case EnemyId::BoneLord:
        switchBgMusic(m_currentNormalBgm);
        break;
    case EnemyId::UFO:
        break;
    case EnemyId::AlienPilot:
        switchBgMusic(m_currentNormalBgm);
        break;
    default:
        switchBgMusic(m_currentNormalBgm);
        break;
    }
}

void GameMainPage::playHitSound()
{
    if (m_hitSoundPlayer != nullptr) {
        m_hitSoundPlayer->setPosition(0);
        m_hitSoundPlayer->play();
    }
}

void GameMainPage::playKillSound()
{
    if (m_killSoundPlayer != nullptr) {
        m_killSoundPlayer->setPosition(0);
        m_killSoundPlayer->play();
    }
}

void GameMainPage::positionHudElements()
{
    const int w = width();
    const int h = height();
    static constexpr int kHudMargin = 14;
    static constexpr int kRightHudW = 180;
    static constexpr int kLeftHudW = 160;

    if (m_rightHudPanel != nullptr) {
        m_rightHudPanel->setGeometry(w - kRightHudW - kHudMargin, kHudMargin,
                                     kRightHudW, m_rightHudPanel->sizeHint().height());
    }

    if (m_leftHudPanel != nullptr) {
        m_leftHudPanel->setGeometry(kHudMargin, kHudMargin,
                                    kLeftHudW, m_leftHudPanel->sizeHint().height());
    }

    if (m_waveProgressBar != nullptr) {
        const int barW = std::min(440, w - kLeftHudW - kRightHudW - kHudMargin * 6);
        const int barX = (w - barW) / 2;
        m_waveProgressBar->setGeometry(barX, kHudMargin, barW, 22);
    }

    if (m_enemyDirector != nullptr) {
        m_enemyDirector->positionBossBarFixed((w - 300) / 2, kHudMargin + 28);
    }

    if (m_upgradeOverlay != nullptr && m_upgradeOverlay->isVisible()) {
        m_upgradeOverlay->setGeometry(0, 0, w, h);
    }
}

void GameMainPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    positionHudElements();
}

void GameMainPage::showUpgradeOverlay()
{
    if (m_waveManager == nullptr || m_upgradeOverlay == nullptr) return;
    if (m_upgradeOverlay->isVisible()) return;

    m_pendingUpgradeOptions = m_waveManager->currentUpgradeOptions();
    if (m_pendingUpgradeOptions.empty()) return;

    buildUpgradeCards(m_pendingUpgradeOptions);
    m_selectedUpgradeIndex = -1;
    if (m_upgradeConfirmButton != nullptr) {
        m_upgradeConfirmButton->setEnabled(false);
    }

    m_upgradeOverlay->setGeometry(0, 0, width(), height());
    m_upgradeOverlay->raise();
    m_upgradeOverlay->setVisible(true);

    auto *anim = new QVariantAnimation(m_upgradeOverlay);
    anim->setDuration(350);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        const int alpha = qBound(0, static_cast<int>(value.toReal() * 0.88 * 255.0), 255);
        m_upgradeOverlay->setStyleSheet(
            QStringLiteral("background: rgba(10, 12, 18, %1);").arg(alpha));
    });
    connect(anim, &QVariantAnimation::finished, this, [this]() {
        m_upgradeOverlay->setStyleSheet(
            QStringLiteral("background: rgba(10, 12, 18, 0.88);"));
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void GameMainPage::hideUpgradeOverlay()
{
    if (m_upgradeOverlay == nullptr) return;

    auto *anim = new QVariantAnimation(m_upgradeOverlay);
    anim->setDuration(280);
    anim->setStartValue(1.0);
    anim->setEndValue(0.0);
    anim->setEasingCurve(QEasingCurve::InCubic);
    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        const int alpha = qBound(0, static_cast<int>(value.toReal() * 0.88 * 255.0), 255);
        m_upgradeOverlay->setStyleSheet(
            QStringLiteral("background: rgba(10, 12, 18, %1);").arg(alpha));
    });
    connect(anim, &QVariantAnimation::finished, this, [this]() {
        m_upgradeOverlay->setVisible(false);
    });
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void GameMainPage::buildUpgradeCards(const UpgradeOptions &options)
{
    while (!m_upgradeCards.isEmpty()) {
        QFrame *card = m_upgradeCards.takeLast();
        m_upgradeCardsGrid->removeWidget(card);
        card->deleteLater();
    }

    const int count = static_cast<int>(options.size());
    if (count < 1) return;

    const int topCount = std::min(3, count);
    const int bottomCount = count - topCount;

    auto buildCard = [this](int index, const UpgradeOption &opt) -> QFrame * {
        auto *card = new QFrame(m_upgradeOverlay);
        card->setFixedSize(162, 196);
        card->setCursor(Qt::PointingHandCursor);
        card->setStyleSheet(QStringLiteral(
            "QFrame { background: rgba(40,48,62,0.92); border: 2px solid rgba(75,85,105,0.5);"
            "border-radius: 12px; }"
            "QFrame:hover { border-color: #6b88b8; background: rgba(50,60,78,0.92); }"
            "QFrame[selected=\"true\"] { border-color: #4b7bec; border-width: 2px;"
            "background: rgba(35,45,65,0.95); }"));

        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(6, 10, 6, 10);
        layout->setSpacing(6);
        layout->setAlignment(Qt::AlignCenter);

        auto *iconLabel = new QLabel(card);
        iconLabel->setFixedSize(56, 56);
        iconLabel->setAlignment(Qt::AlignCenter);
        QPixmap icon(opt.iconPath);
        if (!icon.isNull()) {
            iconLabel->setPixmap(icon.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        iconLabel->setStyleSheet(QStringLiteral("background: transparent;"));
        layout->addWidget(iconLabel, 0, Qt::AlignCenter);

        auto *nameLabel = new QLabel(opt.displayName, card);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setWordWrap(true);
        nameLabel->setStyleSheet(QStringLiteral(
            "color: #f0f4fa; font-size: 14px; font-weight: 700; background: transparent;"));
        layout->addWidget(nameLabel);

        auto *descLabel = new QLabel(opt.summary, card);
        descLabel->setAlignment(Qt::AlignCenter);
        descLabel->setWordWrap(true);
        descLabel->setStyleSheet(QStringLiteral(
            "color: #93a0b4; font-size: 11px; background: transparent;"));
        layout->addWidget(descLabel);
        layout->addStretch();

        card->setProperty("cardIndex", index);
        card->installEventFilter(this);

        return card;
    };

    for (int i = 0; i < topCount; ++i) {
        auto *card = buildCard(i, options[static_cast<size_t>(i)]);
        m_upgradeCards.append(card);
        m_upgradeCardsGrid->addWidget(card, 0, i, Qt::AlignCenter);
    }

    for (int i = 0; i < bottomCount; ++i) {
        const int idx = topCount + i;
        auto *card = buildCard(idx, options[static_cast<size_t>(idx)]);
        m_upgradeCards.append(card);
        m_upgradeCardsGrid->addWidget(card, 1, i, Qt::AlignCenter);
    }
}

void GameMainPage::onUpgradeCardClicked(int index)
{
    if (index < 0 || index >= m_upgradeCards.size()) return;

    m_selectedUpgradeIndex = index;

    for (int i = 0; i < m_upgradeCards.size(); ++i) {
        m_upgradeCards[i]->setProperty("selected", i == index);
        m_upgradeCards[i]->style()->unpolish(m_upgradeCards[i]);
        m_upgradeCards[i]->style()->polish(m_upgradeCards[i]);
    }

    if (m_upgradeConfirmButton != nullptr) {
        m_upgradeConfirmButton->setEnabled(true);
    }
}

void GameMainPage::confirmUpgrade()
{
    if (m_selectedUpgradeIndex < 0
        || static_cast<size_t>(m_selectedUpgradeIndex) >= m_pendingUpgradeOptions.size()
        || m_waveManager == nullptr) {
        return;
    }

    const UpgradeOption &opt = m_pendingUpgradeOptions[static_cast<size_t>(m_selectedUpgradeIndex)];
    m_waveManager->confirmUpgradeSelection(opt);

    if (opt.kind == UpgradeOptionKind::Trait) {
        applyTrait(opt.traitId);
        emit traitAcquired(opt.traitId);
    } else if (opt.kind == UpgradeOptionKind::Weapon) {
        applyWeaponUpgrade(opt.weaponUpgradeId);
        emit statsChanged();
    } else if (opt.kind == UpgradeOptionKind::Stat) {
        const QStringList parts = opt.optionId.split('.');
        if (parts.size() >= 2) {
            bool ok = false;
            const int styleInt = parts.at(1).toInt(&ok);
            if (ok) {
                setActiveBulletStyle(static_cast<BulletStyle>(styleInt));
                emit statsChanged();
            }
        }
    }

    hideUpgradeOverlay();
    resumeBattleState();
    updateStatusText();
}
