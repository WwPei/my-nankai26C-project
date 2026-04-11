#ifndef GAME_MAIN_PAGE_H
#define GAME_MAIN_PAGE_H

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QGraphicsItem>
#include <QLabel>
#include <QProgressBar>
#include <QGraphicsPixmapItem>
#include <QList>
#include <QPointF>

#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "weapon.h"
#include "trait.h"
#include "game_data.h"
// 前置声明自定义图形项（避免循环依赖）
class PlayerItem;
class EnemyItem;
class BulletItem;
class Trait;        // 新增：特性前置声明
class GameFactory;
class EnemyData;
class EnemyView;
class BulletData;
class BulletView;
// ==========================================
// GameMainPage 类：游戏的核心战斗界面
// 核心作用：管理2D游戏场景、驱动游戏循环、处理玩家输入、调度所有游戏对象
// 技术框架：基于 Qt Graphics View 框架（专门用于2D交互式图形）
// 设计原则：完全开放，不预设任何游戏业务逻辑，仅提供基础框架
// ==========================================
class GameMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit GameMainPage(QWidget *parent = nullptr);

    // ==========================================
    // 通用游戏对象管理接口（完全开放，不限制对象类型）
    // 支持所有 QGraphicsItem 子类：玩家、敌人、子弹、召唤物、近战攻击判定、道具等
    // ==========================================
    void addItemToScene(QGraphicsItem *item);    // 添加任意游戏对象到场景
    void removeItemFromScene(QGraphicsItem *item);// 从场景中移除任意游戏对象
    void clearGameScene();                         // 清空整个场景（重新开始游戏时用）

    // ==========================================
    // 游戏循环控制接口
    // ==========================================
    void startGameLoop();  // 启动游戏循环（职业选择页跳转过来后调用）
    void pauseGameLoop();  // 暂停游戏循环（按暂停键时调用）
    void resumeGameLoop(); // 继续游戏循环（按继续键时调用）

    // ==========================================
    // 新增：外部调用接口（职业数据、开始游戏、升级返回）
    // ==========================================
    void setPlayerClass(const ClassData &data);   // 设置玩家职业属性
    void startGame();                             // 开始新游戏（从第1轮开始）
    void onUpgradePageClosed();                   // 升级页面关闭后调用，开始下一轮

signals:
    // ==========================================
    // 页面跳转信号（已修改：携带当前轮次）
    // ==========================================
    void openUpgradePageRequested(int currentWave); // 请求打开升级页（波次结束/按E键时发送），传递当前轮次

    // ==========================================
    // 预留游戏状态信号（后续按需添加）
    // ==========================================
    void gamePaused();           // 游戏已暂停（可连接到UI显示暂停界面）
    void gameResumed();          // 游戏已继续
    void gameOver(bool victory); // 游戏结束（玩家死亡传false，通关传true）

protected:
    // ==========================================
    // 全量玩家输入处理接口（完全开放）
    // 后续在这里实现：WASD移动、空格暂停、E打开升级、鼠标瞄准、点击射击等
    // ==========================================
    void keyPressEvent(QKeyEvent *event) override;   // 键盘按下事件
    void keyReleaseEvent(QKeyEvent *event) override; // 键盘松开事件
    void mouseMoveEvent(QMouseEvent *event) override; // 鼠标移动事件（用于瞄准）
    void mousePressEvent(QMouseEvent *event) override;// 鼠标点击事件（用于射击）
    void mouseReleaseEvent(QMouseEvent *event) override;// 鼠标松开事件（用于停止射击）
    void resizeEvent(QResizeEvent *event) override;   // 窗口大小调整时更新UI面板位置

private slots:
    // ==========================================
    // 游戏循环核心接口（每帧自动调用一次，默认约60帧/秒）
    // 后续在这里按顺序实现：
    // 1. 更新玩家位置/状态
    // 2. 更新AI敌人位置/技能（弱AI：朝玩家移动）
    // 3. 更新远程弹道/召唤物位置（飞行方向、穿透逻辑）
    // 4. 碰撞检测（用 QGraphicsItem::collidingItems()）
    // 5. 游戏状态判断（血量、得分、波次触发、BOSS生成）
    // ==========================================
    void updateGame();
    void onCountdownTimeout();   // 倒计时每秒触发
    void onEnemySpawnTimeout();  // 敌人生成定时器（每10秒）

private:
    // ==========================================
    // 2D游戏图形核心组件
    // ==========================================
    QGraphicsView *m_gameView;   // 游戏视图：相当于"显示器"，负责把场景渲染到窗口上
    QGraphicsScene *m_gameScene; // 游戏场景：相当于"游戏世界"，管理所有游戏对象，内置碰撞检测
    QTimer *m_gameLoopTimer;     // 游戏循环定时器：驱动 updateGame() 每帧调用
    QTimer *m_countdownTimer;    // 倒计时定时器（1秒间隔）
    QTimer *m_enemySpawnTimer;   // 敌人生成定时器（10秒间隔）
    QGraphicsPixmapItem *m_backgroundItem; // 背景图片项（预留）

    // ==========================================
    // 游戏对象
    // ==========================================
    PlayerItem *m_player;                 // 玩家对象
    // 【修改】替换原来的 QList<EnemyItem*> m_enemies;
    QList<EnemyData*> m_enemyDataList;
    QList<EnemyView*> m_enemyViewList;

    // 【修改】替换原来的 QList<BulletItem*> m_bullets;
    QList<BulletData*> m_bulletDataList;
    QList<BulletView*> m_bulletViewList;
    QList<Trait*> m_activeTraits;          // 新增：激活的特性列表（最多4个）

    // ==========================================
    // 游戏状态变量
    // ==========================================
    bool m_isGamePaused;      // 游戏暂停标志位：true时 updateGame() 不执行任何逻辑
    bool m_isGameActive;      // 游戏是否进行中（未结束/未胜利）
    int m_currentWave;        // 当前轮次（1~10）
    int m_remainingTime;      // 当前轮剩余秒数
    int m_enemiesSpawnedInWave; // 本轮已生成的敌人数（最多3个）
    bool m_waveEnding;        // 防止重复触发轮次结束

    // ==========================================
    // 玩家输入标志
    // ==========================================
    bool m_keyW, m_keyS, m_keyA, m_keyD;  // WASD移动标志
    bool m_mouseLeftPressed;              // 鼠标左键是否按住
    QPointF m_mouseScenePos;              // 鼠标在场景中的坐标

    // ==========================================
    // 玩家属性（从职业数据获取）
    // ==========================================
    int m_maxHp;               // 最大血量
    int m_currentHp;           // 当前血量
    int m_strength;            // 力量（影响攻击伤害）
    double m_attackSpeed;      // 攻击速度（每秒攻击次数）
    double m_attackCooldown;   // 攻击冷却剩余时间（秒）

    // ==========================================
    // UI控件（右上角属性面板）
    // ==========================================
    QWidget *m_statsPanel;
    QLabel *m_classNameLabel;
    QLabel *m_hpValueLabel;
    QLabel *m_strengthLabel;
    QLabel *m_attackSpeedLabel;
    QLabel *m_waveLabel;
    QLabel *m_timerLabel;
    QProgressBar *m_playerHpBar;

    // ==========================================
    // 辅助函数
    // ==========================================
    void setupUI();        // 初始化 Graphics View 框架：创建场景、视图、定时器、UI面板
    void connectSignals(); // 连接信号槽：把定时器的 timeout 信号连接到 updateGame()
    void setupPlayer();    // 创建玩家对象
    void spawnEnemy();     // 生成一个敌人
    void shootBullet();    // 发射子弹（根据鼠标方向）
    void updateUI();       // 更新右上角属性显示和血条
    void resetForNewWave(); // 重置新一轮（清除敌人、重置倒计时、重置生成计数）
    void endWaveAndOpenUpgrade(); // 结束当前轮次，打开升级页面

    void addTrait(Trait *trait);
    void removeTrait(Trait *trait);

    // 【新增】工厂指针
    GameFactory *m_factory;

};

#endif // GAME_MAIN_PAGE_H
