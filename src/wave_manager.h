#pragma once

#include <QObject>

#include "game_data.h"

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

public slots:
    virtual void resetRun() = 0;
    virtual void startBattle() = 0;
    virtual void stopBattle() = 0;
    virtual void enterUpgrade() = 0;
    virtual void resumeBattleFromUpgrade() = 0;
    virtual void advanceFrame(int deltaMs) = 0;
    virtual void addExperience(int amount) = 0;

signals:
    void battleStateChanged(BattleFlowState state);
    void roundChanged(int roundIndex);
    void roundCompleted(int roundIndex);
    void experienceChanged(int currentExperience, int currentLevel, int experienceToNextLevel);
    void upgradeOptionsPrepared(UpgradeOptions options);
    void upgradeRequested();
};
