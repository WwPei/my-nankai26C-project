#include "game_main_page.h"
#include "player.h"
#include "enemy.h"       // 现在包含 EnemyView
#include "enemy_data.h"  // 新增：敌人数据
#include "bullet.h"      // 现在包含 BulletView
#include "bullet_data.h" // 新增：子弹数据
#include "game_factory.h"// 新增：工厂
#include "weapon.h"
#include "trait.h"
#include "game_data.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDebug>
#include <QtMath>
#include <QLineF>
#include <QRandomGenerator>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QPainter>
#include <QGraphicsPixmapItem>

// ==================== GameMainPage 实现 ====================

GameMainPage::GameMainPage(QWidget *parent)
    : QWidget(parent)
    , m_gameView(nullptr)
    , m_gameScene(nullptr)
    , m_backgroundItem(nullptr)
    , m_gameLoopTimer(nullptr)
    , m_countdownTimer(nullptr)
    , m_enemySpawnTimer(nullptr)
    , m_factory(nullptr)     // 【修改】初始化工厂指针
    , m_player(nullptr)
    , m_isGamePaused(false)
    , m_isGameActive(false)
    , m_currentWave(1)
    , m_remainingTime(30)
    , m_enemiesSpawnedInWave(0)
    , m_waveEnding(false)
    , m_keyW(false), m_keyS(false), m_keyA(false), m_keyD(false)
    , m_mouseLeftPressed(false)
    , m_mouseScenePos(640, 360)
    , m_maxHp(100), m_currentHp(100)
    , m_strength(10), m_attackSpeed(2.0), m_attackCooldown(0.0)
{
    setupUI();
    connectSignals();
}

// ==========================================
// UI 初始化
// ==========================================
void GameMainPage::setupUI()
{
    m_gameScene = new QGraphicsScene(this);
    m_gameScene->setSceneRect(0, 0, 1280, 720);

    m_backgroundItem = new QGraphicsPixmapItem();
    m_gameScene->addItem(m_backgroundItem);

    m_gameView = new QGraphicsView(m_gameScene, this);
    m_gameView->setRenderHint(QPainter::Antialiasing);
    m_gameView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gameView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_gameView->setFrameStyle(QFrame::NoFrame);
    m_gameView->setCursor(Qt::CrossCursor);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_gameView);
    setLayout(layout);

    m_gameLoopTimer = new QTimer(this);
    m_gameLoopTimer->setInterval(16);
    m_countdownTimer = new QTimer(this);
    m_countdownTimer->setInterval(1000);
    m_enemySpawnTimer = new QTimer(this);
    m_enemySpawnTimer->setInterval(10000);

    // 【修改】1. 初始化工厂 (必须在 setupPlayer 之前)
    m_factory = new GameFactory(m_gameScene, this);

    // 2. 创建玩家
    setupPlayer();
    m_gameScene->addItem(m_player);
    m_player->setVisible(false);

    // 3. 属性面板
    m_statsPanel = new QWidget(this);
    m_statsPanel->setStyleSheet("background-color: rgba(0,0,0,180); color: white; border-radius: 10px;");
    m_statsPanel->setGeometry(this->width() - 280, 10, 270, 200);

    QFormLayout *form = new QFormLayout(m_statsPanel);
    m_classNameLabel = new QLabel("未知职业");
    m_hpValueLabel = new QLabel(QString("%1/%2").arg(m_currentHp).arg(m_maxHp));
    m_strengthLabel = new QLabel(QString::number(m_strength));
    m_attackSpeedLabel = new QLabel(QString::number(m_attackSpeed));
    m_waveLabel = new QLabel("第 1/10 轮");
    m_timerLabel = new QLabel("30s");
    m_timerLabel->setAlignment(Qt::AlignCenter);
    m_timerLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: yellow;");

    form->addRow("职业:", m_classNameLabel);
    form->addRow("血量:", m_hpValueLabel);
    form->addRow("力量:", m_strengthLabel);
    form->addRow("攻速:", m_attackSpeedLabel);
    form->addRow("轮次:", m_waveLabel);
    form->addRow("倒计时:", m_timerLabel);

    m_playerHpBar = new QProgressBar(m_statsPanel);
    m_playerHpBar->setRange(0, m_maxHp);
    m_playerHpBar->setValue(m_currentHp);
    m_playerHpBar->setStyleSheet("QProgressBar::chunk { background-color: red; }"
                                 "QProgressBar { border: 1px solid black; background-color: black; }");
    form->addRow("血条:", m_playerHpBar);
}

// ==========================================
// 信号连接
// ==========================================
void GameMainPage::connectSignals()
{
    connect(m_gameLoopTimer, &QTimer::timeout, this, &GameMainPage::updateGame);
    connect(m_countdownTimer, &QTimer::timeout, this, &GameMainPage::onCountdownTimeout);
    connect(m_enemySpawnTimer, &QTimer::timeout, this, &GameMainPage::onEnemySpawnTimeout);
}

// ==========================================
// 创建玩家并装备默认武器
// ==========================================
void GameMainPage::setupPlayer()
{
    m_player = new PlayerItem(40, 40);
    m_player->setPos(640 - 20, 360 - 20);

    // 【修改】使用工厂创建默认武器
    Weapon *defaultWeapon = m_factory->createDefaultWeapon();
    m_player->setLeftWeapon(defaultWeapon);
    m_player->setRightWeapon(defaultWeapon);
}

// ==========================================
// 通用游戏对象管理接口 (保留用于兼容，实际逻辑移到 Factory)
// ==========================================
void GameMainPage::addItemToScene(QGraphicsItem *item)
{
    if (item && m_gameScene)
        m_gameScene->addItem(item);
}

void GameMainPage::removeItemFromScene(QGraphicsItem *item)
{
    if (item && m_gameScene)
        m_gameScene->removeItem(item);
}

void GameMainPage::clearGameScene()
{
    // 这个函数需要小心使用，现在最好直接操作具体的列表
    if (m_gameScene) {
        m_gameScene->clear();
        if (m_backgroundItem)
            m_gameScene->addItem(m_backgroundItem);
        if (m_player)
            m_gameScene->addItem(m_player);
    }
}

// ==========================================
// 游戏循环控制
// ==========================================
void GameMainPage::startGameLoop()
{
    if (m_gameLoopTimer && !m_gameLoopTimer->isActive()) {
        m_gameLoopTimer->start();
        m_isGamePaused = false;
    }
}

void GameMainPage::pauseGameLoop()
{
    m_isGamePaused = true;
    emit gamePaused();
}

void GameMainPage::resumeGameLoop()
{
    m_isGamePaused = false;
    emit gameResumed();
}

// ==========================================
// 外部调用接口
// ==========================================
void GameMainPage::setPlayerClass(const ClassData &data)
{
    m_maxHp = data.hp;
    m_currentHp = data.hp;
    m_strength = data.strength;
    m_attackSpeed = data.attackSpeed;

    qDebug() << "职业:" << data.name << " HP:" << m_maxHp
             << " 力量:" << m_strength << " 攻速:" << m_attackSpeed;

    if (m_classNameLabel)
        m_classNameLabel->setText(data.name);
    updateUI();
}

void GameMainPage::startGame()
{
    m_currentWave = 1;
    m_isGameActive = true;
    m_isGamePaused = false;
    m_waveEnding = false;
    resetForNewWave();
    m_player->setVisible(true);
    startGameLoop();
}

void GameMainPage::onUpgradePageClosed()
{
    m_currentWave++;
    if (m_currentWave > 10) {
        m_isGameActive = false;
        m_gameLoopTimer->stop();
        emit gameOver(true);
        return;
    }

    m_isGameActive = true;
    m_isGamePaused = false;
    m_waveEnding = false;
    resetForNewWave();
}

// ==========================================
// 【核心修改】生成敌人
// ==========================================
void GameMainPage::spawnEnemy()
{
    if (!m_isGameActive || m_isGamePaused)
        return;
    if (m_enemiesSpawnedInWave >= 3)
        return;

    // 1. 计算出生位置
    int side = QRandomGenerator::global()->bounded(4);
    int x = 0, y = 0;
    const int sceneW = 1280, sceneH = 720;
    switch (side) {
    case 0: x = QRandomGenerator::global()->bounded(sceneW); y = -40; break;
    case 1: x = sceneW; y = QRandomGenerator::global()->bounded(sceneH); break;
    case 2: x = QRandomGenerator::global()->bounded(sceneW); y = sceneH; break;
    case 3: x = -40; y = QRandomGenerator::global()->bounded(sceneH); break;
    }

    // 2. 使用工厂创建数据对象
    EnemyData *data = m_factory->createEnemyData(1);
    data->setPos(QPointF(x, y));

    // 3. 使用工厂创建视图对象
    EnemyView *view = m_factory->createEnemyView(data);

    // 4. 连接死亡信号 (用于自动清理)
    connect(data, &EnemyData::sigDied, this,[this]  {
        // 标记死亡，在下一帧清理，避免在遍历中删除
        // 这里为了简单，我们先不做立即删除，放在 updateGame 最后统一清理
    });

    // 5. 加入管理列表
    m_enemyDataList.append(data);
    m_enemyViewList.append(view);
    m_enemiesSpawnedInWave++;
}

// ==========================================
// 【核心修改】发射子弹
// ==========================================
void GameMainPage::shootBullet()
{
    if (!m_isGameActive || m_isGamePaused || !m_player)
        return;

    QPointF playerCenter = m_player->getCenter();
    QPointF dir = m_mouseScenePos - playerCenter;
    if (dir.manhattanLength() < 0.1)
        return;
    dir /= QLineF(QPointF(0, 0), dir).length();

    // 辅助 Lambda：发射单只手的子弹
    auto shootHand = [this, &dir](Weapon *weapon, const QPointF &spawnPos){
        // 1. 工厂创建子弹数据 (计算伤害逻辑移到了 Factory 内部)
        BulletData *data = m_factory->createBulletData(spawnPos, dir, m_strength, weapon);

        // 2. 工厂创建子弹视图
        BulletView *view = m_factory->createBulletView(data);

        // 3. 加入管理列表
        m_bulletDataList.append(data);
        m_bulletViewList.append(view);
    };

    // 右手射击
    if (Weapon *rightWeapon = m_player->rightWeapon()) {
        shootHand(rightWeapon, m_player->getRightWeaponMuzzle());
    }
    // 左手射击
    if (Weapon *leftWeapon = m_player->leftWeapon()) {
        shootHand(leftWeapon, m_player->getLeftWeaponMuzzle());
    }
}

void GameMainPage::updateUI()
{
    if (m_hpValueLabel)
        m_hpValueLabel->setText(QString("%1/%2").arg(m_currentHp).arg(m_maxHp));
    if (m_playerHpBar) {
        m_playerHpBar->setMaximum(m_maxHp);
        m_playerHpBar->setValue(m_currentHp);
    }
    if (m_strengthLabel)
        m_strengthLabel->setText(QString::number(m_strength));
    if (m_attackSpeedLabel)
        m_attackSpeedLabel->setText(QString::number(m_attackSpeed));
    if (m_waveLabel)
        m_waveLabel->setText(QString("第 %1/10 轮").arg(m_currentWave));
    if (m_timerLabel)
        m_timerLabel->setText(QString::number(m_remainingTime) + "s");
}

// ==========================================
// 【核心修改】重置场景
// ==========================================
void GameMainPage::resetForNewWave()
{
    // 1. 清除敌人
    // 注意：view 被 scene 持有，scene clear 时会删，但这里我们手动删更安全
    // 或者利用 Qt 对象树，让 factory 作为 parent，这里只 clear 列表
    for (EnemyView *v : m_enemyViewList) {
        if (v && v->scene()) m_gameScene->removeItem(v);
        delete v;
    }
    for (EnemyData *d : m_enemyDataList) {
        delete d; // 数据也删掉
    }
    m_enemyViewList.clear();
    m_enemyDataList.clear();

    // 2. 清除子弹
    for (BulletView *v : m_bulletViewList) {
        if (v && v->scene()) m_gameScene->removeItem(v);
        delete v;
    }
    for (BulletData *d : m_bulletDataList) {
        delete d;
    }
    m_bulletViewList.clear();
    m_bulletDataList.clear();

    // 3. 重置数据
    m_remainingTime = 30;
    m_enemiesSpawnedInWave = 0;
    m_waveEnding = false;

    if (m_countdownTimer->isActive())
        m_countdownTimer->stop();
    if (m_enemySpawnTimer->isActive())
        m_enemySpawnTimer->stop();

    if (m_isGameActive && !m_isGamePaused) {
        m_countdownTimer->start();
        spawnEnemy();
        m_enemySpawnTimer->start();
    }

    if (m_player) {
        m_player->setPos(640 - 20, 360 - 20);
        m_player->setNormalColor();
    }

    updateUI();
}

void GameMainPage::endWaveAndOpenUpgrade()
{
    if (m_waveEnding)
        return;
    m_waveEnding = true;

    m_countdownTimer->stop();
    m_enemySpawnTimer->stop();
    m_isGameActive = false;

    // 这里不清空场景，保留视觉效果，直到进入下一轮再 clear
    emit openUpgradePageRequested(m_currentWave);
}

// ==========================================
// 特性管理
// ==========================================
void GameMainPage::addTrait(Trait *trait)
{
    if (!trait || m_activeTraits.size() >= 4)
        return;
    m_activeTraits.append(trait);
    trait->onEquip(m_player);
    qDebug() << "装备特性:" << trait->name();
}

void GameMainPage::removeTrait(Trait *trait)
{
    if (!trait)
        return;
    int index = m_activeTraits.indexOf(trait);
    if (index != -1) {
        trait->onUnequip(m_player);
        m_activeTraits.removeAt(index);
        qDebug() << "移除特性:" << trait->name();
    }
}

// ==========================================
// 定时器槽函数
// ==========================================
void GameMainPage::onCountdownTimeout()
{
    if (!m_isGameActive || m_isGamePaused)
        return;
    if (m_remainingTime > 0) {
        m_remainingTime--;
        updateUI();
        if (m_remainingTime == 0) {
            m_enemySpawnTimer->stop();
        }
    }
}

void GameMainPage::onEnemySpawnTimeout()
{
    if (!m_isGameActive || m_isGamePaused)
        return;
    if (m_enemiesSpawnedInWave < 3 && m_remainingTime > 0) {
        spawnEnemy();
    } else {
        m_enemySpawnTimer->stop();
    }
}

// ==========================================
// 【核心修改】游戏主循环
// ==========================================
void GameMainPage::updateGame()
{
    if (m_isGamePaused || !m_isGameActive)
        return;

    const float deltaTime = 0.016f;

    // ----- 1. 攻击冷却 -----
    if (m_attackCooldown > 0)
        m_attackCooldown -= deltaTime;
    if (m_mouseLeftPressed && m_attackCooldown <= 0) {
        shootBullet();
        m_attackCooldown = 1.0 / m_attackSpeed;
    }

    // ----- 2. 玩家移动 -----
    const qreal speed = 5.0;
    qreal dx = 0, dy = 0;
    if (m_keyW) dy -= 1; if (m_keyS) dy += 1;
    if (m_keyA) dx -= 1; if (m_keyD) dx += 1;
    if (dx != 0 || dy != 0) {
        QPointF dir(dx, dy);
        qreal len = std::hypot(dx, dy);
        if (len > 0) dir /= len;
        QPointF newPos = m_player->pos() + dir * speed;
        newPos.setX(qBound(0.0, newPos.x(), 1280.0 - 40));
        newPos.setY(qBound(0.0, newPos.y(), 720.0 - 40));
        m_player->setPos(newPos);
    }

    // ----- 3. 更新武器方向 -----
    QPointF playerCenter = m_player->getCenter();
    QPointF aimDir = m_mouseScenePos - playerCenter;
    if (aimDir.manhattanLength() < 0.1) aimDir = QPointF(1, 0);
    else aimDir /= QLineF(QPointF(0,0), aimDir).length();
    m_player->updateWeaponArrows(aimDir);

    // ----- 4. 【修改】子弹逻辑 (纯数据运算) -----
    for (int i = 0; i < m_bulletDataList.size(); ++i) {
        BulletData *bData = m_bulletDataList[i];

        // 移动数据
        bData->update();

        // 边界检测 (数据层判断)
        QPointF pos = bData->pos();
        const qreal margin = 80.0;
        if (pos.x() + 6 < 0 || pos.x() - 6 > 1280 ||
            pos.y() + 6 < 0 || pos.y() - 6 > 720) {
            bData->deactivate();
        }

        // 碰撞检测 (使用数据坐标进行距离计算，代替 collidesWithItem)
        if (bData->isActive()) {
            for (int j = 0; j < m_enemyDataList.size(); ++j) {
                EnemyData *eData = m_enemyDataList[j];
                if (eData->isDead()) continue;

                // 简单的矩形/圆形碰撞判定 (距离法)
                // 敌人中心 vs 子弹中心
                QPointF eCenter = eData->pos() + QPointF(20, 20); // 假设敌人40x40
                QPointF bCenter = bData->pos();

                // 计算距离
                qreal dist = QLineF(eCenter, bCenter).length();
                if (dist < 26.0) { // 敌人半长20 + 子弹半径6
                    // 命中！
                    eData->takeDamage(bData->damage());
                    bData->deactivate();

                    // 特性触发 (这里需要传入 PlayerItem*，保持原样)
                    for (Trait *trait : m_activeTraits) {
                        if (auto *lifeSteal = dynamic_cast<TraitLifeSteal*>(trait)) {
                            // 注意：这里 lifeSteal 逻辑需要 Player 有 heal 接口，暂时保留原样
                            lifeSteal->onDamageDealt(m_player, bData->damage());
                        }
                    }
                    break;
                }
            }
        }
    }

    // ----- 5. 【修改】敌人 AI (纯数据运算) -----
    QPointF playerPos = m_player->pos() + QPointF(20, 20); // 玩家中心
    for (EnemyData *eData : m_enemyDataList) {
        if (eData->isDead()) continue;

        QPointF delta = playerPos - (eData->pos() + QPointF(20, 20));
        if (delta.manhattanLength() > 0.1) {
            delta /= QLineF(QPointF(0,0), delta).length();
            eData->moveBy(delta.x() * 2.0, delta.y() * 2.0);
        }
    }

    // ----- 6. 【修改】玩家与敌人碰撞 (纯数据运算) -----
    for (int i = 0; i < m_enemyDataList.size(); ++i) {
        EnemyData *eData = m_enemyDataList[i];
        if (eData->isDead()) continue;

        QPointF eCenter = eData->pos() + QPointF(20, 20);
        QPointF pCenter = playerPos;

        qreal dist = QLineF(eCenter, pCenter).length();
        if (dist < 40.0) { // 碰撞距离
            m_currentHp -= eData->damage();
            updateUI();

            m_player->setHurtColor();
            QTimer::singleShot(150,[this](){
                if (m_player) m_player->setNormalColor();
            });

            if (m_currentHp <= 0) {
                m_currentHp = 0;
                updateUI();
                m_isGameActive = false;
                m_gameLoopTimer->stop();
                m_countdownTimer->stop();
                m_enemySpawnTimer->stop();
                emit gameOver(false);
                return;
            }

            // 击退 (修改数据)
            QPointF away = eData->pos() - m_player->pos();
            if (away.manhattanLength() > 0.1) {
                away /= away.manhattanLength();
                eData->setPos(eData->pos() + away * 20);
            }
        }
    }

    // ----- 7. 【修改】同步视图层 (这是唯一操作 View 的地方) -----
    for (BulletView *v : m_bulletViewList) {v->syncPosition();}
    for (EnemyView *v : m_enemyViewList){ v->syncPosition();}
    // 血条等已经通过信号槽同步了，这里只同步位置保证流畅

    // ----- 8. 【修改】垃圾回收 (清理死亡/失效对象) -----
    // 清理子弹
    for (int i = m_bulletDataList.size() - 1; i >= 0; --i) {
        if (!m_bulletDataList[i]->isActive()) {
            delete m_bulletDataList[i];
            delete m_bulletViewList[i];
            m_bulletDataList.removeAt(i);
            m_bulletViewList.removeAt(i);
        }
    }
    // 清理敌人
    for (int i = m_enemyDataList.size() - 1; i >= 0; --i) {
        if (m_enemyDataList[i]->isDead()) {
            delete m_enemyDataList[i];
            delete m_enemyViewList[i];
            m_enemyDataList.removeAt(i);
            m_enemyViewList.removeAt(i);
        }
    }

    // ----- 9. 特性更新 -----
    for (Trait *trait : m_activeTraits) {
        trait->update(m_player, deltaTime);
    }

    // ----- 10. 检查轮次结束 -----
    if (m_remainingTime <= 0 && m_enemyDataList.isEmpty() && !m_waveEnding) {
        endWaveAndOpenUpgrade();
    }
}

// ==========================================
// 输入事件处理 (保持不变)
// ==========================================
void GameMainPage::keyPressEvent(QKeyEvent *event) { /* ... 原代码保持不变 ... */
    int key = event->key();
    if (key == Qt::Key_W) m_keyW = true;
    if (key == Qt::Key_S) m_keyS = true;
    if (key == Qt::Key_A) m_keyA = true;
    if (key == Qt::Key_D) m_keyD = true;
    if (key == Qt::Key_Space) {
        if (m_isGameActive) m_isGamePaused ? resumeGameLoop() : pauseGameLoop();
    }
    if (key == Qt::Key_E && m_isGameActive) endWaveAndOpenUpgrade();
    QWidget::keyPressEvent(event);
}

void GameMainPage::keyReleaseEvent(QKeyEvent *event) {
    int key = event->key();
    if (key == Qt::Key_W) m_keyW = false;
    if (key == Qt::Key_S) m_keyS = false;
    if (key == Qt::Key_A) m_keyA = false;
    if (key == Qt::Key_D) m_keyD = false;
    QWidget::keyReleaseEvent(event);
}

void GameMainPage::mouseMoveEvent(QMouseEvent *event) {
    m_mouseScenePos = m_gameView->mapToScene(event->pos());
    QWidget::mouseMoveEvent(event);
}

void GameMainPage::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_mouseLeftPressed = true;
        if (m_attackCooldown <= 0 && m_isGameActive && !m_isGamePaused) {
            shootBullet();
            m_attackCooldown = 1.0 / m_attackSpeed;
        }
    }
    QWidget::mousePressEvent(event);
}

void GameMainPage::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) m_mouseLeftPressed = false;
    QWidget::mouseReleaseEvent(event);
}

void GameMainPage::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    if (m_statsPanel) m_statsPanel->setGeometry(this->width() - 280, 10, 270, 200);
}
