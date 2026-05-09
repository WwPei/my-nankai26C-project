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
#include <QCheckBox>
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
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QProgressBar>
#include <QPushButton>
#include <QRandomGenerator>
#include <QTimer>
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
QCheckBox#gridToggle {
    spacing: 8px;
    color: #d7dce5;
}
QCheckBox#gridToggle::indicator {
    width: 18px;
    height: 18px;
    border-radius: 5px;
    border: 1px solid #5b6474;
    background: #1b1f26;
}
QCheckBox#gridToggle::indicator:checked {
    background: #4b7bec;
    border: 1px solid #82afff;
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
    if (qApp != nullptr) {
        qApp->installEventFilter(this);
    }

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(12);

    auto *titleLabel = new QLabel(QStringLiteral("\u9636\u6bb52\u6218\u6597\u9875"), this);
    titleLabel->setObjectName(QStringLiteral("pageTitleLabel"));
    rootLayout->addWidget(titleLabel);

    m_classLabel = new QLabel(QStringLiteral("\u5f53\u524d\u804c\u4e1a\uff1a\u672a\u9009\u62e9"), this);
    m_classLabel->setObjectName(QStringLiteral("classLabel"));
    rootLayout->addWidget(m_classLabel);

    m_statusLabel = new QLabel(QStringLiteral("\u7b49\u5f85\u804c\u4e1a\u9009\u62e9\u5e76\u521b\u5efa\u6218\u6597\u5bf9\u8c61\u3002"), this);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    m_statusLabel->setWordWrap(true);
    rootLayout->addWidget(m_statusLabel);

    auto *middleLayout = new QHBoxLayout();
    middleLayout->setSpacing(12);
    rootLayout->addLayout(middleLayout, 1);

    auto *battlePanel = new QFrame(this);
    battlePanel->setObjectName(QStringLiteral("battlePanel"));
    auto *battleLayout = new QVBoxLayout(battlePanel);
    battleLayout->setContentsMargins(12, 12, 12, 12);
    battleLayout->setSpacing(10);

    auto *battleHeaderLayout = new QHBoxLayout();
    auto *battleTitle = new QLabel(QStringLiteral("\u6218\u6597\u533a"), battlePanel);
    battleTitle->setObjectName(QStringLiteral("panelTitleLabel"));
    m_gridToggle = new QCheckBox(QStringLiteral("\u663e\u793a\u7f51\u683c"), battlePanel);
    m_gridToggle->setObjectName(QStringLiteral("gridToggle"));
    m_gridToggle->setChecked(true);
    battleHeaderLayout->addWidget(battleTitle);
    battleHeaderLayout->addStretch();
    battleHeaderLayout->addWidget(m_gridToggle);
    battleLayout->addLayout(battleHeaderLayout);

    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-320.0, -180.0, 640.0, 360.0);

    m_view = new BattleArenaView(m_scene, battlePanel);
    m_view->setMinimumSize(780, 500);
    m_view->setFocusPolicy(Qt::StrongFocus);
    m_view->setMouseTracking(true);
    m_view->viewport()->setMouseTracking(true);
    m_view->installEventFilter(this);
    m_view->viewport()->installEventFilter(this);
    battleLayout->addWidget(m_view, 1);
    middleLayout->addWidget(battlePanel, 1);

    auto *statusPanel = new QFrame(this);
    statusPanel->setObjectName(QStringLiteral("statusPanel"));
    statusPanel->setFixedWidth(280);
    auto *statusLayout = new QVBoxLayout(statusPanel);
    statusLayout->setContentsMargins(14, 14, 14, 14);
    statusLayout->setSpacing(10);

    auto *panelTitle = new QLabel(QStringLiteral("\u72b6\u6001\u9762\u677f"), statusPanel);
    panelTitle->setObjectName(QStringLiteral("panelTitleLabel"));
    statusLayout->addWidget(panelTitle);

    auto createStatBlock = [statusPanel, statusLayout](const QString &title, QLabel **valueLabel) {
        auto *titleLabel = new QLabel(title, statusPanel);
        titleLabel->setObjectName(QStringLiteral("statTitleLabel"));
        statusLayout->addWidget(titleLabel);

        *valueLabel = new QLabel(QStringLiteral("--"), statusPanel);
        (*valueLabel)->setObjectName(QStringLiteral("statValueLabel"));
        statusLayout->addWidget(*valueLabel);
    };

    auto *healthTitle = new QLabel(QStringLiteral("\u751f\u547d\u503c"), statusPanel);
    healthTitle->setObjectName(QStringLiteral("statTitleLabel"));
    statusLayout->addWidget(healthTitle);

    m_healthBar = new QProgressBar(statusPanel);
    m_healthBar->setObjectName(QStringLiteral("healthBar"));
    m_healthBar->setRange(0, 100);
    m_healthBar->setValue(0);
    m_healthBar->setFormat(QStringLiteral("0 / 0"));
    statusLayout->addWidget(m_healthBar);

    auto *waveTitle = new QLabel(QStringLiteral("\u9636\u6bb5\u8fdb\u5ea6"), statusPanel);
    waveTitle->setObjectName(QStringLiteral("statTitleLabel"));
    statusLayout->addWidget(waveTitle);

    m_waveProgressBar = new QProgressBar(statusPanel);
    m_waveProgressBar->setObjectName(QStringLiteral("waveProgressBar"));
    m_waveProgressBar->setRange(0, GameConfig::kWaveConfig.roundDurationMs);
    m_waveProgressBar->setValue(0);
    m_waveProgressBar->setFormat(QStringLiteral("\u7b2c1/%1\u6ce2 %p%").arg(GameConfig::kWaveConfig.maxRounds));
    statusLayout->addWidget(m_waveProgressBar);

    createStatBlock(QStringLiteral("\u5f53\u524d\u6ce2\u6b21"), &m_roundLabel);
    createStatBlock(QStringLiteral("\u7b49\u7ea7"), &m_levelLabel);
    createStatBlock(QStringLiteral("\u7ecf\u9a8c"), &m_experienceLabel);
    createStatBlock(QStringLiteral("\u6b66\u5668"), &m_weaponLabel);
    createStatBlock(QStringLiteral("\u653b\u51fb\u529b"), &m_attackLabel);
    createStatBlock(QStringLiteral("\u653b\u901f"), &m_attackSpeedLabel);
    createStatBlock(QStringLiteral("\u79fb\u901f"), &m_moveSpeedLabel);
    createStatBlock(QStringLiteral("\u654c\u4eba\u6570"), &m_enemyCountLabel);
    createStatBlock(QStringLiteral("\u5b50\u5f39\u6570"), &m_bulletCountLabel);
    createStatBlock(QStringLiteral("\u5c5e\u6027\u53d8\u5316"), &m_attributeChangeLabel);
    createStatBlock(QStringLiteral("\u5df2\u83b7\u7279\u6027"), &m_traitsLabel);
    createStatBlock(QStringLiteral("\u7784\u51c6"), &m_aimHintLabel);
    m_attributeChangeLabel->setWordWrap(true);
    m_attributeChangeLabel->setTextFormat(Qt::RichText);
    m_traitsLabel->setWordWrap(true);

    auto *controlHint = new QLabel(QStringLiteral("\u64cd\u4f5c\uff1aWASD\u81ea\u7531\u79fb\u52a8\uff0c\u9f20\u6807\u7784\u51c6\uff0c\u6309\u4f4f\u5de6\u952e\u6301\u7eed\u653b\u51fb\u3002"), statusPanel);
    controlHint->setWordWrap(true);
    controlHint->setObjectName(QStringLiteral("statTitleLabel"));
    statusLayout->addWidget(controlHint);
    statusLayout->addStretch();

    m_dashCooldownWidget = new DashCooldownWidget(statusPanel);
    statusLayout->addWidget(m_dashCooldownWidget, 0, Qt::AlignCenter);

    middleLayout->addWidget(statusPanel);

    m_playerMarker = m_scene->addEllipse(-kPlayerMarkerRadius, -kPlayerMarkerRadius,
                                         kPlayerMarkerRadius * 2.0, kPlayerMarkerRadius * 2.0,
                                         QPen(Qt::NoPen), QBrush(Qt::transparent));
    m_playerMarker->setVisible(false);
    m_playerMarker->setZValue(4.0);

    m_playerAvatar = new PlayerAvatarItem();
    m_playerAvatar->setVisible(false);
    m_scene->addItem(m_playerAvatar);

    m_gameLoopTimer = new QTimer(this);
    m_gameLoopTimer->setInterval(GameConfig::kWaveConfig.updateIntervalMs);
    connect(m_gameLoopTimer, &QTimer::timeout, this, &GameMainPage::handleBattleTick);
    connect(m_gridToggle, &QCheckBox::toggled, m_view, [this](bool checked) {
        if (m_view != nullptr) {
            m_view->setGridVisible(checked);
        }
    });

    auto *buttonLayout = new QHBoxLayout();
    m_upgradeButton = new QPushButton(QStringLiteral("\u5f53\u524d\u65e0\u5f85\u9009\u5347\u7ea7"), this);
    m_upgradeButton->setObjectName(QStringLiteral("upgradeButton"));
    auto *exitButton = new QPushButton(QStringLiteral("\u8fd4\u56de\u5f00\u59cb\u9875"), this);
    exitButton->setObjectName(QStringLiteral("exitButton"));
    buttonLayout->addWidget(m_upgradeButton);
    buttonLayout->addWidget(exitButton);
    buttonLayout->addStretch();
    rootLayout->addLayout(buttonLayout);

    connect(m_upgradeButton, &QPushButton::clicked, this, [this]() {
        if (m_waveManager != nullptr && m_waveManager->hasPendingUpgrade()) {
            m_waveManager->enterUpgrade();
            return;
        }
        updateStatusText();
    });
    connect(exitButton, &QPushButton::clicked, this, &GameMainPage::exitRequested);

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
        setBattleActive(false);
        emit upgradeRequested();
    });

    m_upgradeResolver = new UpgradeResolver(nullptr, nullptr, nullptr, this);
    connect(m_upgradeResolver, &UpgradeResolver::statsChanged, this, &GameMainPage::statsChanged);
    m_combatCoordinator = new CombatCoordinator(nullptr, m_scene, nullptr, nullptr, this);
    m_enemyDirector = new EnemyDirector(nullptr, m_scene, this, this);

    updateHealthBarStyle(1.0F);
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

    const auto *config = GameConfig::findPlayerClassConfig(classId);
    if (config == nullptr) {
        m_classLabel->setText(QStringLiteral("\u5f53\u524d\u804c\u4e1a\uff1a\u672a\u77e5"));
        return;
    }

    m_classLabel->setText(QStringLiteral("\u5f53\u524d\u804c\u4e1a\uff1a%1").arg(config->displayName));
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
    m_combatCoordinator->resolveCombatCollisions(m_bullets, m_enemies, m_playerMarker,
                                                  m_playerDamageCooldownRemainingMs, traitCounts);
    m_combatCoordinator->cleanupExpiredBullets(m_bullets);
    m_combatCoordinator->cleanupDefeatedEnemies(m_enemies, m_waveManager, traitCounts);

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
    m_playerAvatar->setHitFlash(kPlayerDamageCooldownMs <= 0.0F
                                    ? 0.0
                                    : m_playerDamageCooldownRemainingMs / kPlayerDamageCooldownMs);
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
    if (m_factory == nullptr) {
        m_statusLabel->setText(QStringLiteral("\u9636\u6bb52\u6218\u6597\u9875\uff1a\u5de5\u5382\u5c1a\u672a\u6ce8\u5165\uff0c\u65e0\u6cd5\u521b\u5efa\u6218\u6597\u5bf9\u8c61\u3002"));
        m_roundLabel->setText(QStringLiteral("--"));
        m_levelLabel->setText(QStringLiteral("--"));
        m_experienceLabel->setText(QStringLiteral("--"));
        m_weaponLabel->setText(QStringLiteral("--"));
        m_attackLabel->setText(QStringLiteral("--"));
        m_attackSpeedLabel->setText(QStringLiteral("--"));
        m_moveSpeedLabel->setText(QStringLiteral("--"));
        m_enemyCountLabel->setText(QStringLiteral("0"));
        m_bulletCountLabel->setText(QStringLiteral("0"));
        m_attributeChangeLabel->setText(QStringLiteral("\u7b49\u5f85\u521d\u59cb\u5316"));
        m_traitsLabel->setText(QStringLiteral("\u672a\u83b7\u5f97"));
        m_aimHintLabel->setText(QStringLiteral("\u7b49\u5f85\u521d\u59cb\u5316"));
        m_waveProgressBar->setValue(0);
        m_healthBar->setValue(0);
        m_healthBar->setFormat(QStringLiteral("0 / 0"));
        updateHealthBarStyle(1.0F);
        if (m_upgradeButton != nullptr) {
            m_upgradeButton->setEnabled(false);
            m_upgradeButton->setText(QStringLiteral("\u5f53\u524d\u65e0\u5f85\u9009\u5347\u7ea7"));
        }
        return;
    }

    if (!m_hasSelectedClass || m_player == nullptr) {
        m_statusLabel->setText(QStringLiteral("\u9636\u6bb52\u6218\u6597\u9875\uff1a\u7b49\u5f85\u804c\u4e1a\u9009\u62e9\u540e\u521b\u5efa\u73a9\u5bb6\u3001\u654c\u4eba\u4e0e\u5b50\u5f39\u3002"));
        m_roundLabel->setText(QStringLiteral("\u7b2c1/%1\u6ce2").arg(GameConfig::kWaveConfig.maxRounds));
        m_levelLabel->setText(QStringLiteral("Lv.0"));
        m_experienceLabel->setText(QStringLiteral("0 / %1").arg(GameConfig::experienceThresholdForLevel(1)));
        m_weaponLabel->setText(QStringLiteral("--"));
        m_attackLabel->setText(QStringLiteral("--"));
        m_attackSpeedLabel->setText(QStringLiteral("--"));
        m_moveSpeedLabel->setText(QStringLiteral("--"));
        m_enemyCountLabel->setText(QStringLiteral("0"));
        m_bulletCountLabel->setText(QStringLiteral("0"));
        m_attributeChangeLabel->setText(QStringLiteral("\u6682\u65e0\u5c5e\u6027\u53d8\u5316"));
        m_traitsLabel->setText(QStringLiteral("\u672a\u83b7\u5f97"));
        m_aimHintLabel->setText(QStringLiteral("\u9f20\u6807\u7784\u51c6"));
        m_waveProgressBar->setValue(0);
        m_healthBar->setValue(0);
        m_healthBar->setFormat(QStringLiteral("0 / 0"));
        updateHealthBarStyle(1.0F);
        if (m_upgradeButton != nullptr) {
            m_upgradeButton->setEnabled(false);
            m_upgradeButton->setText(QStringLiteral("\u5f53\u524d\u65e0\u5f85\u9009\u5347\u7ea7"));
        }
        return;
    }

    const auto enemyCount = m_enemies.size();
    const auto bulletCount = m_bullets.size();
    const auto currentHealth = m_player->currentHealth();
    const auto maxHealth = std::max(1.0F, m_player->maxHealth());
    const auto healthRatio = currentHealth / maxHealth;
    const auto *classConfig = m_factory->playerClassConfig(m_selectedClassId);
    const auto *weaponConfig = m_factory->weaponConfig(m_player->weaponId());
    const auto weaponName = weaponConfig != nullptr ? weaponConfig->displayName : QStringLiteral("\u672a\u77e5\u6b66\u5668");
    float damageMultiplier = 1.0F;
    float defenseMultiplier = 1.0F;
    float speedMultiplier = 1.0F;

    const QList<TraitId> ownedTraits = m_upgradeResolver != nullptr
        ? m_upgradeResolver->ownedTraits()
        : QList<TraitId>{};
    const QMap<TraitId, int> traitCounts = m_upgradeResolver != nullptr
        ? m_upgradeResolver->traitCounts()
        : QMap<TraitId, int>{};

    for (const TraitId traitId : ownedTraits) {
        const TraitConfig *traitConfig = GameConfig::findTraitConfig(traitId);
        if (traitConfig == nullptr) {
            continue;
        }
        damageMultiplier *= traitConfig->damageMultiplier;
        defenseMultiplier *= traitConfig->defenseMultiplier;
        speedMultiplier *= traitConfig->speedMultiplier;
    }

    const auto baseAttackDamage = weaponConfig != nullptr ? weaponConfig->baseDamage : 0.0F;
    const auto baseAttackSpeed = weaponConfig != nullptr && weaponConfig->fireIntervalMs > 0.0F
        ? 1000.0F / weaponConfig->fireIntervalMs
        : 0.0F;
    const auto effectiveAttackDamage = baseAttackDamage * damageMultiplier;
    const auto effectiveAttackSpeed = baseAttackSpeed * speedMultiplier;
    const auto baseMoveSpeed = classConfig != nullptr ? classConfig->moveSpeed : 0.0F;

    QStringList attributeChanges;
    if (!qFuzzyCompare(damageMultiplier, 1.0F)) {
        attributeChanges.push_back(QStringLiteral("\u653b\u51fb %1").arg(signedPercentText(damageMultiplier)));
    }
    if (!qFuzzyCompare(speedMultiplier, 1.0F)) {
        attributeChanges.push_back(QStringLiteral("\u653b\u901f %1").arg(signedPercentText(speedMultiplier)));
        attributeChanges.push_back(QStringLiteral("\u79fb\u901f %1").arg(signedPercentText(speedMultiplier)));
    }
    if (defenseMultiplier > 1.0F) {
        attributeChanges.push_back(QStringLiteral("\u51cf\u4f24 %1").arg(reductionPercentText(defenseMultiplier)));
    }

    const QString hitCooldownText = m_playerDamageCooldownRemainingMs > 0.0F
        ? QStringLiteral("\u53d7\u51fb\u95ea\u70c1")
        : QStringLiteral("\u7a33\u5b9a");
    const int currentRound = m_waveManager != nullptr ? m_waveManager->currentRound() : 1;
    const int currentLevel = m_waveManager != nullptr ? m_waveManager->currentLevel() : 0;
    const int currentExperience = m_waveManager != nullptr ? m_waveManager->currentExperience() : 0;
    const int experienceToNext = m_waveManager != nullptr ? m_waveManager->experienceToNextLevel() : 0;
    const int elapsedRoundMs = m_waveManager != nullptr ? m_waveManager->elapsedRoundMs() : 0;
    const bool hasPendingUpgrade = m_waveManager != nullptr && m_waveManager->hasPendingUpgrade();

    QString battleStateText = QStringLiteral("\u975e\u6218\u6597");
    switch (m_battleState) {
    case BattleFlowState::Inactive:
        battleStateText = QStringLiteral("\u975e\u6218\u6597");
        break;
    case BattleFlowState::Battle:
        battleStateText = QStringLiteral("\u6218\u6597\u4e2d");
        break;
    case BattleFlowState::Upgrade:
        battleStateText = QStringLiteral("\u5347\u7ea7\u4e2d");
        break;
    }

    QStringList ownedTraitNames;
    for (const TraitId traitId : ownedTraits) {
        const TraitConfig *traitConfig = GameConfig::findTraitConfig(traitId);
        const int traitLevel = traitCounts.value(traitId, 1);
        const QString displayName = traitConfig != nullptr ? traitConfig->displayName : QStringLiteral("\u672a\u77e5\u7279\u6027");
        ownedTraitNames.push_back(traitLevel > 1
                                      ? QStringLiteral("%1 Lv.%2").arg(displayName, QString::number(traitLevel))
                                      : displayName);
    }

    const bool clearingRemainingEnemies = elapsedRoundMs >= GameConfig::kWaveConfig.roundDurationMs
        && enemyCount > 0
        && !hasPendingUpgrade;
    QString statusText;
    if (hasPendingUpgrade) {
        statusText = QStringLiteral("\u9636\u6bb52\u6218\u6597\u9875\uff1a\u5f53\u524d\u72b6\u6001[%1\uff5c%2]\uff0c\u89d2\u8272\u5df2\u5347\u7ea7\uff0c\u5fc5\u987b\u5148\u5b8c\u6210\u5c5e\u6027\u9009\u62e9\u624d\u80fd\u7ee7\u7eed\u3002")
                         .arg(hitCooldownText, battleStateText);
    } else if (clearingRemainingEnemies) {
        statusText = QStringLiteral("\u9636\u6bb52\u6218\u6597\u9875\uff1a\u5f53\u524d\u72b6\u6001[%1\uff5c%2]\uff0c\u672c\u6ce2\u5237\u602a\u5df2\u7ed3\u675f\uff0c\u9700\u6e05\u5149\u5269\u4f59 %3 \u4e2a\u654c\u4eba\u540e\u624d\u80fd\u8fdb\u5165\u4e0b\u4e00\u9636\u6bb5\u3002")
                         .arg(hitCooldownText, battleStateText, QString::number(enemyCount));
    } else {
        statusText = QStringLiteral("\u9636\u6bb52\u6218\u6597\u9875\uff1a\u5f53\u524d\u72b6\u6001[%1\uff5c%2\uff5c\u65e0\u5f85\u5347\u7ea7]\u3002")
                         .arg(hitCooldownText, battleStateText);
    }
    m_statusLabel->setText(statusText);
    m_roundLabel->setText(QStringLiteral("\u7b2c%1/%2\u6ce2")
                              .arg(QString::number(currentRound),
                                   QString::number(GameConfig::kWaveConfig.maxRounds)));
    m_levelLabel->setText(QStringLiteral("Lv.%1").arg(QString::number(currentLevel)));
    m_experienceLabel->setText(experienceToNext > 0
                                   ? QStringLiteral("%1\uff08\u8ddd\u4e0b\u7ea7 %2\uff09")
                                         .arg(QString::number(currentExperience),
                                              QString::number(experienceToNext))
                                   : QStringLiteral("%1\uff08\u5df2\u6ee1\u7ea7\uff09")
                                         .arg(QString::number(currentExperience)));
    QString weaponLabelText = weaponName;
    QStringList weaponUpgradeTags;
    if (m_weapon != nullptr) {
        if (m_weapon->extraProjectiles() > 0) {
            weaponUpgradeTags.push_back(QStringLiteral("+%1\u5f39\u9053").arg(m_weapon->extraProjectiles()));
        }
        if (m_weapon->pierceCount() > 0) {
            weaponUpgradeTags.push_back(QStringLiteral("\u7a7f\u900f%1").arg(m_weapon->pierceCount()));
        }
        if (m_weapon->bulletSizeScale() > 1.0F) {
            weaponUpgradeTags.push_back(QStringLiteral("\u5f39\u4f53%1%").arg(
                QString::number(m_weapon->bulletSizeScale() * 100.0F, 'f', 0)));
        }
        if (m_weapon->comboInterval() > 0) {
            weaponUpgradeTags.push_back(QStringLiteral("\u8fde\u51fb/%1").arg(m_weapon->comboInterval()));
        }
        if (m_weapon->rangeMultiplier() > 1.0F) {
            weaponUpgradeTags.push_back(QStringLiteral("\u5c04\u7a0b%1%").arg(
                QString::number(m_weapon->rangeMultiplier() * 100.0F, 'f', 0)));
        }
    }
    if (!weaponUpgradeTags.isEmpty()) {
        weaponLabelText += QStringLiteral(" [+%1]").arg(weaponUpgradeTags.join(QStringLiteral(", ")));
    }
    m_weaponLabel->setText(weaponLabelText);
    m_attackLabel->setText(QStringLiteral("%1 / \u53d1\uff08\u57fa\u51c6 %2\uff09")
                               .arg(QString::number(effectiveAttackDamage, 'f', 1),
                                    QString::number(baseAttackDamage, 'f', 1)));
    m_attackSpeedLabel->setText(QStringLiteral("%1 \u53d1/\u79d2\uff08\u57fa\u51c6 %2\uff09")
                                    .arg(QString::number(effectiveAttackSpeed, 'f', 2),
                                         QString::number(baseAttackSpeed, 'f', 2)));
    m_moveSpeedLabel->setText(QStringLiteral("%1\uff08\u57fa\u51c6 %2\uff09")
                                  .arg(QString::number(m_player->moveSpeed(), 'f', 0),
                                       QString::number(baseMoveSpeed, 'f', 0)));
    m_enemyCountLabel->setText(QString::number(enemyCount));
    m_bulletCountLabel->setText(QString::number(bulletCount));

    const QString lastUpgradeSummary = m_upgradeResolver != nullptr
        ? m_upgradeResolver->lastUpgradeSummary()
        : QString{};
    const QString passiveSummary = attributeChanges.isEmpty()
        ? QStringLiteral("\u7d2f\u8ba1\u5c5e\u6027\uff1a\u6682\u65e0\u53d8\u5316")
        : QStringLiteral("\u7d2f\u8ba1\u5c5e\u6027\uff1a%1").arg(attributeChanges.join(QStringLiteral(" \uff5c ")));
    m_attributeChangeLabel->setText(lastUpgradeSummary.isEmpty()
                                        ? passiveSummary
                                        : QStringLiteral("%1<br/><span style='color:#9aa8bc;'>%2</span>")
                                              .arg(lastUpgradeSummary, passiveSummary));
    m_traitsLabel->setText(ownedTraitNames.isEmpty() ? QStringLiteral("\u672a\u83b7\u5f97")
                                                     : ownedTraitNames.join(QStringLiteral(" / ")));
    m_aimHintLabel->setText(m_firePressed ? QStringLiteral("\u9f20\u6807\u7784\u51c6 \u00b7 \u6301\u7eed\u653b\u51fb")
                                          : QStringLiteral("\u9f20\u6807\u7784\u51c6 \u00b7 \u677e\u5f00\u5f85\u673a"));
    if (m_upgradeButton != nullptr) {
        m_upgradeButton->setEnabled(hasPendingUpgrade);
        m_upgradeButton->setText(hasPendingUpgrade
                                     ? QStringLiteral("\u5fc5\u987b\u9009\u62e9\u5347\u7ea7")
                                     : QStringLiteral("\u5f53\u524d\u65e0\u5f85\u9009\u5347\u7ea7"));
    }

    m_healthBar->setRange(0, static_cast<int>(std::ceil(maxHealth)));
    m_healthBar->setValue(static_cast<int>(std::round(currentHealth)));
    m_healthBar->setFormat(QStringLiteral("%1 / %2")
                               .arg(QString::number(currentHealth, 'f', 0))
                               .arg(QString::number(maxHealth, 'f', 0)));
    updateHealthBarStyle(healthRatio);

    m_waveProgressBar->setRange(0, GameConfig::kWaveConfig.roundDurationMs);
    m_waveProgressBar->setFormat(clearingRemainingEnemies
                                     ? QStringLiteral("\u7b2c%1/%2\u6ce2 \u6e05\u573a\u4e2d\uff08\u5269\u4f59\u654c\u4eba %3\uff09")
                                           .arg(QString::number(currentRound),
                                                QString::number(GameConfig::kWaveConfig.maxRounds),
                                                QString::number(enemyCount))
                                     : QStringLiteral("\u7b2c%1/%2\u6ce2 %p%")
                                           .arg(QString::number(currentRound),
                                                QString::number(GameConfig::kWaveConfig.maxRounds)));
    m_waveProgressBar->setValue(std::min(elapsedRoundMs, GameConfig::kWaveConfig.roundDurationMs));
}
