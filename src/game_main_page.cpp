// FILE_LOCK: @qt6-logic-developer-emojidungeon
// 负责: 游戏主页面最小可玩战斗逻辑
// 最后修改: 2026-04-28
#include "game_main_page.h"

#include "bullet.h"
#include "enemy.h"
#include "player.h"
#include "weapon.h"

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
constexpr qreal kSpreadStepDegrees = 12.0;
constexpr double kPi = 3.14159265358979323846;

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

[[nodiscard]] QPointF rotateVector(const QPointF &vector, qreal angleDegrees)
{
    const double radians = angleDegrees * kPi / 180.0;
    const double cosValue = std::cos(radians);
    const double sinValue = std::sin(radians);
    return QPointF(vector.x() * cosValue - vector.y() * sinValue,
                   vector.x() * sinValue + vector.y() * cosValue);
}

[[nodiscard]] qreal randomBetween(qreal minimum, qreal maximum)
{
    return minimum + (maximum - minimum) * QRandomGenerator::global()->generateDouble();
}

} // namespace

class BattleArenaView final : public QGraphicsView
{
public:
    explicit BattleArenaView(QGraphicsScene *scene, QWidget *parent = nullptr)
        : QGraphicsView(scene, parent)
    {
        setObjectName(QStringLiteral("battleArenaView"));
        setRenderHint(QPainter::Antialiasing, true);
        setRenderHint(QPainter::TextAntialiasing, true);
        setFrameShape(QFrame::NoFrame);
        setAlignment(Qt::AlignCenter);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setTransformationAnchor(QGraphicsView::AnchorViewCenter);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);
    }

    void setGridVisible(bool visible)
    {
        if (m_gridVisible == visible) {
            return;
        }

        m_gridVisible = visible;
        viewport()->update();
    }

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override
    {
        painter->fillRect(rect, QColor(QStringLiteral("#2a2a2a")));

        if (!m_gridVisible) {
            return;
        }

        constexpr qreal majorStep = 80.0;
        constexpr qreal minorStep = 40.0;

        QPen minorPen(QColor(58, 64, 74, 120));
        minorPen.setWidthF(1.0);
        QPen majorPen(QColor(86, 96, 112, 150));
        majorPen.setWidthF(1.0);

        const qreal left = std::floor(rect.left() / minorStep) * minorStep;
        const qreal right = std::ceil(rect.right() / minorStep) * minorStep;
        const qreal top = std::floor(rect.top() / minorStep) * minorStep;
        const qreal bottom = std::ceil(rect.bottom() / minorStep) * minorStep;

        for (qreal x = left; x <= right; x += minorStep) {
            painter->setPen(std::fmod(std::abs(x), majorStep) < 0.1 ? majorPen : minorPen);
            painter->drawLine(QLineF(x, top, x, bottom));
        }

        for (qreal y = top; y <= bottom; y += minorStep) {
            painter->setPen(std::fmod(std::abs(y), majorStep) < 0.1 ? majorPen : minorPen);
            painter->drawLine(QLineF(left, y, right, y));
        }
    }

private:
    bool m_gridVisible {true};
};

class PlayerAvatarItem final : public QGraphicsItem
{
public:
    explicit PlayerAvatarItem(QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
    {
        setZValue(5.0);
    }

    [[nodiscard]] QRectF boundingRect() const override
    {
        return QRectF(-28.0, -28.0, 56.0, 56.0);
    }

    void setAimDirection(const QPointF &direction)
    {
        const QPointF normalized = normalizedVector(direction);
        if (QLineF(normalized, m_aimDirection).length() <= 0.0001) {
            return;
        }

        m_aimDirection = normalized;
        update();
    }

    void setHitFlash(qreal intensity)
    {
        const qreal bounded = std::clamp(intensity, 0.0, 1.0);
        if (qFuzzyCompare(m_hitFlashIntensity, bounded)) {
            return;
        }

        m_hitFlashIntensity = bounded;
        update();
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        painter->setRenderHint(QPainter::Antialiasing, true);

        const QColor shadowColor(30, 120, 255, static_cast<int>(90 + 60 * (1.0 - m_hitFlashIntensity)));
        painter->setPen(Qt::NoPen);
        painter->setBrush(shadowColor);
        painter->drawEllipse(QRectF(-22.0, -22.0, 44.0, 44.0));

        const QColor bodyColor = QColor::fromRgbF(
            0.255 + 0.45 * m_hitFlashIntensity,
            0.412 + 0.16 * m_hitFlashIntensity,
            0.882 + 0.06 * m_hitFlashIntensity,
            1.0);
        painter->setBrush(bodyColor);
        painter->drawEllipse(QRectF(-kPlayerVisualRadius, -kPlayerVisualRadius,
                                    kPlayerVisualRadius * 2.0, kPlayerVisualRadius * 2.0));

        painter->setPen(QPen(QColor(210, 232, 255), 1.4));
        painter->setBrush(QColor(195, 225, 255));
        QPainterPath pointerPath;
        const QPointF forward = m_aimDirection * 23.0;
        const QPointF side(-m_aimDirection.y(), m_aimDirection.x());
        pointerPath.moveTo(forward);
        pointerPath.lineTo(m_aimDirection * 8.0 + side * 6.0);
        pointerPath.lineTo(m_aimDirection * 8.0 - side * 6.0);
        pointerPath.closeSubpath();
        painter->drawPath(pointerPath);
    }

private:
    QPointF m_aimDirection {1.0, 0.0};
    qreal m_hitFlashIntensity {0.0};
};

GameMainPage::GameMainPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("gameMainPage"));
    setStyleSheet(QString::fromUtf8(kBattlePageStyle));

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(16, 16, 16, 16);
    rootLayout->setSpacing(12);

    auto *titleLabel = new QLabel(QStringLiteral("阶段1战斗页"), this);
    titleLabel->setObjectName(QStringLiteral("pageTitleLabel"));
    rootLayout->addWidget(titleLabel);

    m_classLabel = new QLabel(QStringLiteral("当前职业：未选择"), this);
    m_classLabel->setObjectName(QStringLiteral("classLabel"));
    rootLayout->addWidget(m_classLabel);

    m_statusLabel = new QLabel(QStringLiteral("等待职业选择并创建战斗对象。"), this);
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
    auto *battleTitle = new QLabel(QStringLiteral("战斗区"), battlePanel);
    battleTitle->setObjectName(QStringLiteral("panelTitleLabel"));
    m_gridToggle = new QCheckBox(QStringLiteral("显示网格"), battlePanel);
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

    auto *panelTitle = new QLabel(QStringLiteral("状态面板"), statusPanel);
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

    auto *healthTitle = new QLabel(QStringLiteral("生命值"), statusPanel);
    healthTitle->setObjectName(QStringLiteral("statTitleLabel"));
    statusLayout->addWidget(healthTitle);

    m_healthBar = new QProgressBar(statusPanel);
    m_healthBar->setObjectName(QStringLiteral("healthBar"));
    m_healthBar->setRange(0, 100);
    m_healthBar->setValue(0);
    m_healthBar->setFormat(QStringLiteral("0 / 0"));
    statusLayout->addWidget(m_healthBar);

    auto *waveTitle = new QLabel(QStringLiteral("阶段进度"), statusPanel);
    waveTitle->setObjectName(QStringLiteral("statTitleLabel"));
    statusLayout->addWidget(waveTitle);

    m_waveProgressBar = new QProgressBar(statusPanel);
    m_waveProgressBar->setObjectName(QStringLiteral("waveProgressBar"));
    m_waveProgressBar->setRange(0, GameConfig::kWaveConfig.roundDurationMs);
    m_waveProgressBar->setValue(0);
    m_waveProgressBar->setFormat(QStringLiteral("阶段1 %p%"));
    statusLayout->addWidget(m_waveProgressBar);

    createStatBlock(QStringLiteral("武器"), &m_weaponLabel);
    createStatBlock(QStringLiteral("攻击力"), &m_attackLabel);
    createStatBlock(QStringLiteral("攻速"), &m_attackSpeedLabel);
    createStatBlock(QStringLiteral("移速"), &m_moveSpeedLabel);
    createStatBlock(QStringLiteral("敌人数"), &m_enemyCountLabel);
    createStatBlock(QStringLiteral("子弹数"), &m_bulletCountLabel);
    createStatBlock(QStringLiteral("瞄准"), &m_aimHintLabel);

    auto *controlHint = new QLabel(QStringLiteral("操作：WASD移动，鼠标瞄准，按住左键持续攻击。"), statusPanel);
    controlHint->setWordWrap(true);
    controlHint->setObjectName(QStringLiteral("statTitleLabel"));
    statusLayout->addWidget(controlHint);
    statusLayout->addStretch();

    middleLayout->addWidget(statusPanel);

    m_playerMarker = m_scene->addEllipse(-kPlayerMarkerRadius, -kPlayerMarkerRadius,
                                         kPlayerMarkerRadius * 2.0, kPlayerMarkerRadius * 2.0,
                                         QPen(Qt::NoPen), QBrush(Qt::transparent));
    m_playerMarker->setOpacity(0.0);
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
    auto *upgradeButton = new QPushButton(QStringLiteral("打开升级页"), this);
    upgradeButton->setObjectName(QStringLiteral("upgradeButton"));
    auto *exitButton = new QPushButton(QStringLiteral("返回开始页"), this);
    exitButton->setObjectName(QStringLiteral("exitButton"));
    buttonLayout->addWidget(upgradeButton);
    buttonLayout->addWidget(exitButton);
    buttonLayout->addStretch();
    rootLayout->addLayout(buttonLayout);

    connect(upgradeButton, &QPushButton::clicked, this, [this]() {
        enterUpgradeState();
        emit upgradeRequested();
    });
    connect(exitButton, &QPushButton::clicked, this, &GameMainPage::exitRequested);

    updateHealthBarStyle(1.0F);
    updateStatusText();
}

bool GameMainPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_view) {
        switch (event->type()) {
        case QEvent::KeyPress: {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->isAutoRepeat()) {
                return true;
            }

            switch (keyEvent->key()) {
            case Qt::Key_W:
                m_moveUpPressed = true;
                break;
            case Qt::Key_S:
                m_moveDownPressed = true;
                break;
            case Qt::Key_A:
                m_moveLeftPressed = true;
                break;
            case Qt::Key_D:
                m_moveRightPressed = true;
                break;
            default:
                return QWidget::eventFilter(watched, event);
            }

            updateInputDirection();
            return true;
        }
        case QEvent::KeyRelease: {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->isAutoRepeat()) {
                return true;
            }

            switch (keyEvent->key()) {
            case Qt::Key_W:
                m_moveUpPressed = false;
                break;
            case Qt::Key_S:
                m_moveDownPressed = false;
                break;
            case Qt::Key_A:
                m_moveLeftPressed = false;
                break;
            case Qt::Key_D:
                m_moveRightPressed = false;
                break;
            default:
                return QWidget::eventFilter(watched, event);
            }

            updateInputDirection();
            return true;
        }
        case QEvent::FocusOut:
            m_moveUpPressed = false;
            m_moveDownPressed = false;
            m_moveLeftPressed = false;
            m_moveRightPressed = false;
            m_firePressed = false;
            updateInputDirection();
            if (m_weapon != nullptr) {
                m_weapon->stopFiring();
            }
            return QWidget::eventFilter(watched, event);
        default:
            break;
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
        m_classLabel->setText(QStringLiteral("当前职业：未知"));
        return;
    }

    m_classLabel->setText(QStringLiteral("当前职业：%1").arg(config->displayName));
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
    if (m_battleState != BattleFlowState::Upgrade) {
        setBattleState(BattleFlowState::Battle);
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

    setBattleActive(false);
    setBattleState(BattleFlowState::Upgrade);
}

void GameMainPage::resumeBattleState()
{
    setBattleState(BattleFlowState::Battle);
    setBattleActive(true);
}

BattleFlowState GameMainPage::battleState() const noexcept
{
    return m_battleState;
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

    m_elapsedRoundMs = 0;
    m_player->setWorldPosition(QPointF(0.0, 0.0));
    m_player->setMoveDirection(QPointF());
    m_mouseScenePosition = m_player->worldPosition() + QPointF(80.0, 0.0);
    m_enemySpawnAccumulatorMs = 0.0F;
    m_playerDamageCooldownRemainingMs = 0.0F;
    updateInputDirection();

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
        m_gameLoopTimer->stop();
        updatePlayerVisualState();
        updateStatusText();
    });

    m_weapon = m_factory->createStarterWeapon(m_selectedClassId, m_player, this);
    if (m_weapon != nullptr) {
        connect(m_weapon, &Weapon::fireRequested, this, &GameMainPage::handleWeaponFireRequested);
        connect(m_weapon, &Weapon::cooldownChanged, this, [this](float) {
            updateStatusText();
        });
        updateWeaponAim();
    }

    for (int index = 0; index < GameConfig::kWaveConfig.initialEnemyCount; ++index) {
        spawnTestEnemy();
    }

    updatePlayerVisualState();
    updateStatusText();
    setBattleState(BattleFlowState::Battle);
    m_gameLoopTimer->start();
    m_view->setFocus(Qt::OtherFocusReason);
}

void GameMainPage::clearBattleScene()
{
    m_gameLoopTimer->stop();
    m_elapsedRoundMs = 0;
    m_enemySpawnAccumulatorMs = 0.0F;
    m_playerDamageCooldownRemainingMs = 0.0F;
    m_inputDirection = QPointF();
    m_moveUpPressed = false;
    m_moveDownPressed = false;
    m_moveLeftPressed = false;
    m_moveRightPressed = false;
    m_firePressed = false;

    for (const auto &bullet : m_bullets) {
        if (bullet.view != nullptr) {
            m_scene->removeItem(bullet.view);
            bullet.view->deleteLater();
        }
        if (bullet.data != nullptr) {
            bullet.data->deleteLater();
        }
    }
    m_bullets.clear();

    for (const auto &enemy : std::as_const(m_enemies)) {
        if (enemy.view != nullptr) {
            m_scene->removeItem(enemy.view);
            enemy.view->deleteLater();
        }
        if (enemy.data != nullptr) {
            enemy.data->deleteLater();
        }
    }
    m_enemies.clear();

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

    setBattleState(BattleFlowState::Inactive);
}

void GameMainPage::spawnTestEnemy()
{
    if (m_factory == nullptr || m_enemies.size() >= GameConfig::kWaveConfig.maxConcurrentEnemies) {
        return;
    }

    auto enemy = m_factory->createEnemyEntity(randomEnemyId(), randomEnemySpawnPosition(), this);
    if (enemy.data == nullptr || enemy.view == nullptr) {
        if (enemy.view != nullptr) {
            enemy.view->deleteLater();
        }
        if (enemy.data != nullptr) {
            enemy.data->deleteLater();
        }
        return;
    }

    m_scene->addItem(enemy.view);
    m_enemies.push_back(enemy);
}

void GameMainPage::handleBattleTick()
{
    if (m_player == nullptr) {
        m_gameLoopTimer->stop();
        return;
    }

    const float deltaSeconds = static_cast<float>(GameConfig::kWaveConfig.updateIntervalMs) / 1000.0F;
    m_elapsedRoundMs = std::min(GameConfig::kWaveConfig.roundDurationMs,
                                m_elapsedRoundMs + GameConfig::kWaveConfig.updateIntervalMs);
    m_playerDamageCooldownRemainingMs = std::max(
        0.0F,
        m_playerDamageCooldownRemainingMs - static_cast<float>(GameConfig::kWaveConfig.updateIntervalMs));

    updatePlayerMovement(deltaSeconds);
    updateWeaponAim();

    if (m_weapon != nullptr) {
        if (m_firePressed && !m_weapon->isFiring()) {
            m_weapon->startFiring();
        } else if (!m_firePressed && m_weapon->isFiring()) {
            m_weapon->stopFiring();
        }

        m_weapon->advanceCooldown(static_cast<float>(GameConfig::kWaveConfig.updateIntervalMs));
    }

    m_enemySpawnAccumulatorMs += static_cast<float>(GameConfig::kWaveConfig.updateIntervalMs);
    while (m_enemySpawnAccumulatorMs >= static_cast<float>(GameConfig::kWaveConfig.enemySpawnIntervalMs)
           && m_enemies.size() < GameConfig::kWaveConfig.maxConcurrentEnemies) {
        spawnTestEnemy();
        m_enemySpawnAccumulatorMs -= static_cast<float>(GameConfig::kWaveConfig.enemySpawnIntervalMs);
    }

    for (const auto &enemy : std::as_const(m_enemies)) {
        if (enemy.data == nullptr || enemy.data->isDefeated()) {
            continue;
        }
        enemy.data->setTargetPosition(m_player->worldPosition());
        enemy.data->advanceFrame(deltaSeconds);
    }

    for (const auto &bullet : std::as_const(m_bullets)) {
        if (bullet.data == nullptr || bullet.data->isExpired()) {
            continue;
        }
        bullet.data->advanceFrame(deltaSeconds);
    }

    resolveCombatCollisions();
    cleanupExpiredBullets();
    cleanupDefeatedEnemies();
    updatePlayerVisualState();
    updateStatusText();
}

void GameMainPage::handleWeaponFireRequested(WeaponId weaponId,
                                             const QPointF &origin,
                                             const QPointF &direction)
{
    if (m_factory == nullptr) {
        return;
    }

    const WeaponConfig *config = m_factory->weaponConfig(weaponId);
    const int projectileCount = std::max(1, config != nullptr ? config->projectileCount : 1);
    const QPointF normalizedDirectionValue = normalizedVector(direction);

    for (int projectileIndex = 0; projectileIndex < projectileCount; ++projectileIndex) {
        const qreal centeredIndex
            = static_cast<qreal>(projectileIndex) - (static_cast<qreal>(projectileCount - 1) / 2.0);
        const QPointF projectileDirection = projectileCount == 1
            ? normalizedDirectionValue
            : normalizedVector(rotateVector(normalizedDirectionValue, centeredIndex * kSpreadStepDegrees),
                               normalizedDirectionValue);

        auto bullet = m_factory->createBulletEntity(weaponId, origin, projectileDirection, this);
        if (bullet.data == nullptr || bullet.view == nullptr) {
            if (bullet.view != nullptr) {
                bullet.view->deleteLater();
            }
            if (bullet.data != nullptr) {
                bullet.data->deleteLater();
            }
            continue;
        }

        m_scene->addItem(bullet.view);
        m_bullets.push_back(bullet);
    }
}

void GameMainPage::cleanupExpiredBullets()
{
    for (qsizetype index = m_bullets.size() - 1; index >= 0; --index) {
        const auto &bullet = m_bullets.at(index);
        if (bullet.data != nullptr && !bullet.data->isExpired()) {
            continue;
        }

        if (bullet.view != nullptr) {
            m_scene->removeItem(bullet.view);
            bullet.view->deleteLater();
        }
        if (bullet.data != nullptr) {
            bullet.data->deleteLater();
        }
        m_bullets.removeAt(index);
    }
}

void GameMainPage::cleanupDefeatedEnemies()
{
    for (qsizetype index = m_enemies.size() - 1; index >= 0; --index) {
        const auto &enemy = m_enemies.at(index);
        if (enemy.data != nullptr && !enemy.data->isDefeated()) {
            continue;
        }

        if (enemy.view != nullptr) {
            m_scene->removeItem(enemy.view);
            enemy.view->deleteLater();
        }
        if (enemy.data != nullptr) {
            enemy.data->deleteLater();
        }
        m_enemies.removeAt(index);
    }
}

void GameMainPage::resolveCombatCollisions()
{
    for (const auto &bullet : std::as_const(m_bullets)) {
        if (bullet.data == nullptr || bullet.view == nullptr || bullet.data->isExpired()) {
            continue;
        }

        const auto collidingItems = bullet.view->collidingItems();
        for (QGraphicsItem *item : collidingItems) {
            auto *enemyView = qobject_cast<EnemyView *>(item->toGraphicsObject());
            if (enemyView == nullptr || enemyView->model() == nullptr || enemyView->model()->isDefeated()) {
                continue;
            }

            enemyView->model()->receiveDamage(bullet.data->damage());
            bullet.data->expire();
            break;
        }
    }

    if (m_player == nullptr || m_playerMarker == nullptr) {
        return;
    }

    if (m_playerDamageCooldownRemainingMs > 0.0F) {
        return;
    }

    for (const auto &enemy : std::as_const(m_enemies)) {
        if (enemy.data == nullptr || enemy.view == nullptr || enemy.data->isDefeated()) {
            continue;
        }

        if (!enemy.view->collidesWithItem(m_playerMarker)) {
            continue;
        }

        m_player->receiveDamage(enemy.data->contactDamage());
        m_playerDamageCooldownRemainingMs = kPlayerDamageCooldownMs;
        updatePlayerVisualState();
        break;
    }
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
    if (m_player == nullptr || deltaSeconds <= 0.0F || qFuzzyIsNull(QLineF(QPointF(), m_inputDirection).length())) {
        return;
    }

    const QPointF step = m_inputDirection * (m_player->moveSpeed() * deltaSeconds);
    m_player->setWorldPosition(clampPlayerPosition(m_player->worldPosition() + step));
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

QPointF GameMainPage::clampPlayerPosition(const QPointF &position) const
{
    if (m_scene == nullptr) {
        return position;
    }

    const QRectF movableRect = m_scene->sceneRect().adjusted(kPlayerMarkerRadius, kPlayerMarkerRadius,
                                                              -kPlayerMarkerRadius, -kPlayerMarkerRadius);
    return QPointF(std::clamp(position.x(), movableRect.left(), movableRect.right()),
                   std::clamp(position.y(), movableRect.top(), movableRect.bottom()));
}

QPointF GameMainPage::randomEnemySpawnPosition() const
{
    if (m_scene == nullptr) {
        return QPointF();
    }

    const QRectF rect = m_scene->sceneRect().adjusted(kEnemySpawnInset, kEnemySpawnInset,
                                                       -kEnemySpawnInset, -kEnemySpawnInset);
    const int edgeIndex = QRandomGenerator::global()->bounded(4);
    switch (edgeIndex) {
    case 0:
        return QPointF(rect.left(), randomBetween(rect.top(), rect.bottom()));
    case 1:
        return QPointF(rect.right(), randomBetween(rect.top(), rect.bottom()));
    case 2:
        return QPointF(randomBetween(rect.left(), rect.right()), rect.top());
    default:
        return QPointF(randomBetween(rect.left(), rect.right()), rect.bottom());
    }
}

EnemyId GameMainPage::randomEnemyId() const
{
    const int enemyIndex = QRandomGenerator::global()->bounded(GameConfig::kEnemies.size());
    return GameConfig::kEnemies.at(enemyIndex).id;
}

void GameMainPage::updateStatusText()
{
    if (m_factory == nullptr) {
        m_statusLabel->setText(QStringLiteral("阶段1战斗页：工厂尚未注入，无法创建战斗对象。"));
        m_weaponLabel->setText(QStringLiteral("--"));
        m_attackLabel->setText(QStringLiteral("--"));
        m_attackSpeedLabel->setText(QStringLiteral("--"));
        m_moveSpeedLabel->setText(QStringLiteral("--"));
        m_enemyCountLabel->setText(QStringLiteral("0"));
        m_bulletCountLabel->setText(QStringLiteral("0"));
        m_aimHintLabel->setText(QStringLiteral("等待初始化"));
        m_waveProgressBar->setValue(0);
        m_healthBar->setValue(0);
        m_healthBar->setFormat(QStringLiteral("0 / 0"));
        updateHealthBarStyle(1.0F);
        return;
    }

    if (!m_hasSelectedClass || m_player == nullptr) {
        m_statusLabel->setText(QStringLiteral("阶段1战斗页：等待职业选择后创建玩家、敌人与子弹。"));
        m_weaponLabel->setText(QStringLiteral("--"));
        m_attackLabel->setText(QStringLiteral("--"));
        m_attackSpeedLabel->setText(QStringLiteral("--"));
        m_moveSpeedLabel->setText(QStringLiteral("--"));
        m_enemyCountLabel->setText(QStringLiteral("0"));
        m_bulletCountLabel->setText(QStringLiteral("0"));
        m_aimHintLabel->setText(QStringLiteral("鼠标瞄准"));
        m_waveProgressBar->setValue(0);
        m_healthBar->setValue(0);
        m_healthBar->setFormat(QStringLiteral("0 / 0"));
        updateHealthBarStyle(1.0F);
        return;
    }

    const auto enemyCount = m_enemies.size();
    const auto bulletCount = m_bullets.size();
    const auto currentHealth = m_player->currentHealth();
    const auto maxHealth = std::max(1.0F, m_player->maxHealth());
    const auto healthRatio = currentHealth / maxHealth;
    const auto *weaponConfig = m_factory->weaponConfig(m_player->weaponId());
    const auto weaponName = weaponConfig != nullptr ? weaponConfig->displayName : QStringLiteral("未知武器");
    const auto attackDamage = weaponConfig != nullptr ? weaponConfig->baseDamage : 0.0F;
    const auto attackSpeed = weaponConfig != nullptr && weaponConfig->fireIntervalMs > 0.0F
        ? 1000.0F / weaponConfig->fireIntervalMs
        : 0.0F;

    const QString hitCooldownText = m_playerDamageCooldownRemainingMs > 0.0F
        ? QStringLiteral("受击闪烁")
        : QStringLiteral("稳定");

    QString battleStateText = QStringLiteral("非战斗");
    switch (m_battleState) {
    case BattleFlowState::Inactive:
        battleStateText = QStringLiteral("非战斗");
        break;
    case BattleFlowState::Battle:
        battleStateText = QStringLiteral("战斗中");
        break;
    case BattleFlowState::Upgrade:
        battleStateText = QStringLiteral("升级中");
        break;
    }

    m_statusLabel->setText(QStringLiteral("阶段1战斗页：玩家、敌人、子弹基础外观已接入，当前状态[%1｜%2]。")
                               .arg(hitCooldownText, battleStateText));
    m_weaponLabel->setText(weaponName);
    m_attackLabel->setText(QStringLiteral("%1 / 发").arg(QString::number(attackDamage, 'f', 0)));
    m_attackSpeedLabel->setText(QStringLiteral("%1 发/秒").arg(QString::number(attackSpeed, 'f', 2)));
    m_moveSpeedLabel->setText(QStringLiteral("%1").arg(QString::number(m_player->moveSpeed(), 'f', 0)));
    m_enemyCountLabel->setText(QString::number(enemyCount));
    m_bulletCountLabel->setText(QString::number(bulletCount));
    m_aimHintLabel->setText(m_firePressed ? QStringLiteral("鼠标瞄准 · 持续攻击")
                                          : QStringLiteral("鼠标瞄准 · 松开待机"));

    m_healthBar->setRange(0, static_cast<int>(std::ceil(maxHealth)));
    m_healthBar->setValue(static_cast<int>(std::round(currentHealth)));
    m_healthBar->setFormat(QStringLiteral("%1 / %2")
                               .arg(QString::number(currentHealth, 'f', 0))
                               .arg(QString::number(maxHealth, 'f', 0)));
    updateHealthBarStyle(healthRatio);

    m_waveProgressBar->setValue(m_elapsedRoundMs);
}
