#pragma once

#include <QList>
#include <QObject>

#include "game_data.h"

/// 怪物系列：根据当前波次划分，决定生成哪些类型的敌人
enum class MonsterSeries {
    None,
    Bizarre,    // 怪异系列 (waves 1-4)
    Undead,     // 亡灵系列 (waves 4-7)
    Alien       // 外星系列 (waves 7-10)
};

class WaveManager : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    ~WaveManager() override = default;

    [[nodiscard]] virtual const WaveConfig &config() const = 0;
    [[nodiscard]] virtual BattleFlowState battleState() const = 0;
    [[nodiscard]] virtual int currentRound() const = 0;
    [[nodiscard]] virtual int elapsedRoundMs() const = 0;
    [[nodiscard]] virtual int currentExperience() const = 0;
    [[nodiscard]] virtual int currentLevel() const = 0;
    [[nodiscard]] virtual int experienceToNextLevel() const = 0;
    [[nodiscard]] virtual bool hasPendingUpgrade() const = 0;
    [[nodiscard]] virtual UpgradeOptions currentUpgradeOptions() const = 0;

    /// 设置当前波次，同步更新动态难度倍率
    virtual void setCurrentWave(int wave) = 0;
    /// 获取当前波次所属的怪物系列
    [[nodiscard]] virtual MonsterSeries currentSeries() const = 0;
    /// 获取敌人生命值的动态倍率 (1.05^wave)
    [[nodiscard]] virtual float enemyHpMultiplier() const = 0;
    /// 获取敌人伤害的动态倍率 (1.03^wave)
    [[nodiscard]] virtual float enemyDmgMultiplier() const = 0;

    /// 获取并清空待生成的 Boss 列表（由 game_main_page 每帧轮询消费）
    [[nodiscard]] virtual QList<EnemyId> pendingBosses() = 0;
    /// 当前是否为 Boss 波次（有 Boss 存活或尚未被击败）
    [[nodiscard]] virtual bool isBossWave() const = 0;
    /// 当前 Boss 波次所属的怪物系列
    [[nodiscard]] virtual MonsterSeries bossSeries() const = 0;
    /// game_main_page 在 Boss 被击败后通知 wave_manager
    virtual void notifyBossDefeated() = 0;

public slots:
    virtual void resetRun() = 0;
    virtual void startBattle() = 0;
    virtual void stopBattle() = 0;
    virtual void enterUpgrade() = 0;
    virtual void resumeBattleFromUpgrade() = 0;
    virtual void advanceFrame(int deltaMs) = 0;
    virtual void completeCurrentRound() = 0;
    virtual void addExperience(int amount) = 0;
    virtual void confirmUpgradeSelection(UpgradeOption option) = 0;

signals:
    void battleStateChanged(BattleFlowState state);
    void roundChanged(int roundIndex);
    void roundCompleted(int roundIndex);
    void experienceChanged(int currentExperience, int currentLevel, int experienceToNextLevel);
    void upgradeOptionsPrepared(UpgradeOptions options);
    void upgradeRequested();

    /// 请求生成敌人（由 game_main_page 响应并实例化）
    /// @param enemyTypeId 敌人类型 ID (对应 EnemyId 枚举的 int 值)
    /// @param count 生成数量
    /// @param isBoss 是否为 Boss 敌人
    void enemySpawnRequested(int enemyTypeId, int count, bool isBoss);
};

[[nodiscard]] WaveManager *createWaveManager(QObject *parent = nullptr);
