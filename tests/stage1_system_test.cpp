#include <QtTest>

#define private public
#include "emoji_dungeon_window.h"
#include "game_main_page.h"
#undef private

#include <QGraphicsView>

#include "bullet.h"
#include "bullet_data.h"
#include "enemy.h"
#include "enemy_data.h"
#include "game_factory.h"
#include "player.h"
#include "weapon.h"

namespace {

constexpr int kFrameWaitMs = 80;
constexpr int kSpawnWaitBufferMs = 500;

GameMainPage *createReadyGamePage()
{
    auto *page = new GameMainPage();
    auto *factory = new GameFactory(page);
    page->setFactory(factory);
    page->resize(1080, 720);
    page->show();
    if (!QTest::qWaitForWindowExposed(page)) {
        qFatal("测试窗口未能成功显示。");
    }
    page->setSelectedClass(PlayerClassId::Warrior);
    QCoreApplication::processEvents();
    return page;
}

QGraphicsView *battleView(GameMainPage *page)
{
    auto *view = page->findChild<QGraphicsView *>();
    Q_ASSERT(view != nullptr);
    return view;
}

QPoint rightSideViewportPoint(QGraphicsView *view)
{
    const QRect viewportRect = view->viewport()->rect();
    return QPoint(viewportRect.center().x() + viewportRect.width() / 4,
                  viewportRect.center().y());
}

} // namespace

class Stage1SystemTest : public QObject
{
    Q_OBJECT

private slots:
    void wasdMovementMovesPlayer()
    {
        std::unique_ptr<GameMainPage> page(createReadyGamePage());
        auto *view = battleView(page.get());

        QVERIFY(page->m_player != nullptr);
        const QPointF initialPosition = page->m_player->worldPosition();

        view->setFocus(Qt::OtherFocusReason);
        QVERIFY(view->hasFocus());
        QTest::keyPress(view, Qt::Key_W);
        QTest::qWait(kFrameWaitMs);
        QTest::keyRelease(view, Qt::Key_W);

        const QPointF afterUp = page->m_player->worldPosition();
        QVERIFY2(afterUp.y() < initialPosition.y(),
                 "按下W后玩家未向上移动。");

        QTest::keyPress(view, Qt::Key_D);
        QTest::qWait(kFrameWaitMs);
        QTest::keyRelease(view, Qt::Key_D);

        const QPointF afterRight = page->m_player->worldPosition();
        QVERIFY2(afterRight.x() > afterUp.x(),
                 "按下D后玩家未向右移动。");
    }

    void mouseAimAndLeftClickCreateBullets()
    {
        std::unique_ptr<GameMainPage> page(createReadyGamePage());
        auto *view = battleView(page.get());
        auto *viewport = view->viewport();

        QVERIFY(page->m_weapon != nullptr);
        QVERIFY(page->m_bullets.isEmpty());

        const QPoint viewportPoint = rightSideViewportPoint(view);
        QTest::mouseMove(viewport, viewportPoint);
        QTest::qWait(kFrameWaitMs);

        QTest::mousePress(viewport, Qt::LeftButton, Qt::NoModifier, viewportPoint);
        QTRY_VERIFY_WITH_TIMEOUT(!page->m_bullets.isEmpty(), 700);
        QVERIFY(page->m_firePressed);
        QVERIFY(page->m_weapon->isFiring());

        const auto &firstBullet = page->m_bullets.constFirst();
        QVERIFY(firstBullet.data != nullptr);
        QVERIFY2(firstBullet.data->direction().x() > 0.8,
                 "鼠标向右瞄准时，子弹方向没有明显朝右。");
        QVERIFY2(std::abs(firstBullet.data->direction().y()) < 0.4,
                 "鼠标向右瞄准时，子弹方向在Y轴上的偏移过大。");

        QTest::mouseRelease(viewport, Qt::LeftButton, Qt::NoModifier, viewportPoint);
        QTRY_VERIFY_WITH_TIMEOUT(!page->m_firePressed, 100);
        QVERIFY(!page->m_weapon->isFiring());
    }

    void enemiesSpawnOverTimeAndStayWithinCap()
    {
        std::unique_ptr<GameMainPage> page(createReadyGamePage());

        QCOMPARE(page->m_enemies.size(), 3);
        QTRY_VERIFY_WITH_TIMEOUT(page->m_enemies.size() > 3,
                                 GameConfig::kWaveConfig.enemySpawnIntervalMs + kSpawnWaitBufferMs);

        QVERIFY(page->m_enemies.size() <= GameConfig::kWaveConfig.maxConcurrentEnemies);
    }

    void combatCollisionDealsDamage()
    {
        std::unique_ptr<GameMainPage> page(createReadyGamePage());

        QVERIFY(page->m_player != nullptr);
        QVERIFY(!page->m_enemies.isEmpty());

        auto &enemy = page->m_enemies[0];
        QVERIFY(enemy.data != nullptr);
        QVERIFY(enemy.view != nullptr);

        const float initialEnemyHealth = enemy.data->currentHealth();
        auto bullet = page->m_factory->createBulletEntity(page->m_player->weaponId(),
                                                          enemy.data->worldPosition(),
                                                          QPointF(1.0, 0.0),
                                                          page.get());
        QVERIFY(bullet.data != nullptr);
        QVERIFY(bullet.view != nullptr);
        bullet.data->setWorldPosition(enemy.data->worldPosition());
        page->m_scene->addItem(static_cast<QGraphicsItem *>(bullet.view));
        page->m_bullets.push_back(bullet);

        page->resolveCombatCollisions();

        QVERIFY2(enemy.data->currentHealth() < initialEnemyHealth,
                 "子弹与敌人碰撞后未造成伤害。");
        QVERIFY(bullet.data->isExpired());

        const float initialPlayerHealth = page->m_player->currentHealth();
        enemy.data->setWorldPosition(page->m_player->worldPosition());
        page->m_playerDamageCooldownRemainingMs = 0.0F;

        page->resolveCombatCollisions();

        QVERIFY2(page->m_player->currentHealth() < initialPlayerHealth,
                 "敌人与玩家接触后未造成伤害。");
        QVERIFY(page->m_playerDamageCooldownRemainingMs > 0.0F);
    }

    void leavingGamePagePausesBattleTimer()
    {
        EmojiDungeonWindow window;
        window.resize(1080, 720);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        QVERIFY(window.m_gameMainPage != nullptr);
        window.m_gameMainPage->setSelectedClass(PlayerClassId::Warrior);
        window.setCurrentPage(PageId::GameMain);
        QCoreApplication::processEvents();

        QVERIFY(window.m_gameMainPage->m_gameLoopTimer->isActive());

        window.setCurrentPage(PageId::Start);

        QVERIFY2(!window.m_gameMainPage->m_gameLoopTimer->isActive(),
                 "离开游戏页后战斗计时器仍在后台运行，存在隐藏逻辑继续执行的稳定性问题。");
    }
};

QTEST_MAIN(Stage1SystemTest)

#include "stage1_system_test.moc"
