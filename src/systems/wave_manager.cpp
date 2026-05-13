#include "wave_manager.h"

#include <algorithm>
#include <cmath>

#include <QMap>
#include <QRandomGenerator>

#include "special_bullet_config.h"

namespace {

class BasicWaveManager final : public WaveManager
{
    Q_OBJECT

public:
    explicit BasicWaveManager(QObject *parent = nullptr)
        : WaveManager(parent)
    {
        resetRun();
    }

    [[nodiscard]] const WaveConfig &config() const override
    {
        return GameConfig::kWaveConfig;
    }

    [[nodiscard]] BattleFlowState battleState() const override
    {
        return m_battleState;
    }

    [[nodiscard]] int currentRound() const override
    {
        return m_currentRound;
    }

    [[nodiscard]] int elapsedRoundMs() const override
    {
        return m_elapsedRoundMs;
    }

    [[nodiscard]] int currentExperience() const override
    {
        return m_currentExperience;
    }

    [[nodiscard]] int currentLevel() const override
    {
        return m_currentLevel;
    }

    [[nodiscard]] int experienceToNextLevel() const override
    {
        return GameConfig::experienceToNextLevel(m_currentExperience);
    }

    [[nodiscard]] bool hasPendingUpgrade() const override
    {
        return m_hasPendingUpgrade;
    }

    [[nodiscard]] UpgradeOptions currentUpgradeOptions() const override
    {
        return m_currentUpgradeOptions;
    }

public slots:
    void resetRun() override
    {
        m_battleState = BattleFlowState::Inactive;
        m_currentRound = 1;
        m_elapsedRoundMs = 0;
        m_currentExperience = 0;
        m_currentLevel = 0;
        m_hasPendingUpgrade = false;
        m_ownedTraitIds.clear();
        m_currentUpgradeOptions = defaultUpgradeOptions();

        // 重置波次追踪与动态难度
        m_currentWave = 0;
        m_enemyHpMultiplier = 1.0F;
        m_enemyDmgMultiplier = 1.0F;

        // 重置 Boss 生成追踪
        m_pendingBosses.clear();
        m_bossDemonLordSpawned = false;
        m_bossBoneLordSpawned = false;
        m_bossUFOSpawned = false;
        m_bossAlienPilotSpawned = false;
        m_activeBossSeries = MonsterSeries::None;
        m_activeBulletStyle = 0;

        emit roundChanged(m_currentRound);
        emit battleStateChanged(m_battleState);
        emit experienceChanged(m_currentExperience, m_currentLevel, experienceToNextLevel());
    }

    void startBattle() override
    {
        if (m_currentRound > config().maxRounds) {
            return;
        }

        if (m_battleState == BattleFlowState::Battle) {
            return;
        }

        if (m_battleState == BattleFlowState::Upgrade && m_hasPendingUpgrade) {
            return;
        }

        // 初始化当前波次的动态难度倍率
        setCurrentWave(m_currentRound);

        setBattleState(BattleFlowState::Battle);
    }

    void stopBattle() override
    {
        setBattleState(BattleFlowState::Inactive);
    }

    void enterUpgrade() override
    {
        if (!m_hasPendingUpgrade || m_currentRound > config().maxRounds) {
            return;
        }

        prepareUpgradeOptions();
        setBattleState(BattleFlowState::Upgrade);
        emit upgradeRequested();
    }

    void resumeBattleFromUpgrade() override
    {
        if (m_hasPendingUpgrade || m_currentRound > config().maxRounds) {
            return;
        }

        setBattleState(BattleFlowState::Battle);
    }

    void advanceFrame(int deltaMs) override
    {
        if (m_battleState != BattleFlowState::Battle || deltaMs <= 0) {
            return;
        }

        m_elapsedRoundMs = std::min(config().roundDurationMs, m_elapsedRoundMs + deltaMs);

        // Boss 生成检测：在对应波次开始后短暂延迟生成 Boss
        checkAndSpawnBoss();
    }

    /// 检测当前波次是否需要生成 Boss，将 Boss 类型加入待生成列表
    /// game_main_page 通过 pendingBosses() 消费并实际生成
    void checkAndSpawnBoss()
    {
        // 需要一小段延迟避免 Boss 在波次第一帧就刷出
        if (m_elapsedRoundMs < 50) {
            return;
        }

        // 波次 4: 怪异 Boss —— 恶魔领主
        if (m_currentRound >= 4 && !m_bossDemonLordSpawned) {
            m_pendingBosses.append(EnemyId::DemonLord);
            m_bossDemonLordSpawned = true;
            m_activeBossSeries = MonsterSeries::Bizarre;
        }

        // 波次 7: 亡灵 Boss —— 骨王
        if (m_currentRound >= 7 && !m_bossBoneLordSpawned) {
            m_pendingBosses.append(EnemyId::BoneLord);
            m_bossBoneLordSpawned = true;
            m_activeBossSeries = MonsterSeries::Undead;
        }

        // 波次 9: 外星 Boss 一阶段 —— 飞碟
        if (m_currentRound >= 9 && !m_bossUFOSpawned) {
            m_pendingBosses.append(EnemyId::UFO);
            m_bossUFOSpawned = true;
            m_activeBossSeries = MonsterSeries::Alien;
        }
    }

    void completeCurrentRound() override
    {
        if (m_currentRound > config().maxRounds) {
            return;
        }

        const int completedRound = m_currentRound;
        emit roundCompleted(completedRound);

        if (completedRound >= config().maxRounds) {
            setBattleState(BattleFlowState::Inactive);
            return;
        }

        m_currentRound = std::min(config().maxRounds, m_currentRound + 1);
        m_elapsedRoundMs = 0;

        // 更新波次动态难度倍率
        setCurrentWave(m_currentRound);

        emit roundChanged(m_currentRound);

        if (m_hasPendingUpgrade) {
            enterUpgrade();
        }
    }

    void addExperience(int amount) override
    {
        if (amount <= 0) {
            return;
        }

        const int previousLevel = m_currentLevel;
        m_currentExperience = std::max(0, m_currentExperience + amount);
        m_currentLevel = GameConfig::levelForExperience(m_currentExperience);

        if (m_currentLevel > previousLevel) {
            m_hasPendingUpgrade = true;
            prepareUpgradeOptions();
            if (m_battleState == BattleFlowState::Battle) {
                setBattleState(BattleFlowState::Upgrade);
                emit upgradeRequested();
            }
        }

        emit experienceChanged(m_currentExperience, m_currentLevel, experienceToNextLevel());
    }

    void confirmUpgradeSelection(UpgradeOption option) override
    {
        if (!m_hasPendingUpgrade) {
            return;
        }

        if (option.kind == UpgradeOptionKind::Trait) {
            m_ownedTraitIds[option.traitId] = option.level;
        }

        if (option.kind == UpgradeOptionKind::Stat) {
            const QStringList parts = option.optionId.split('.');
            if (parts.size() >= 2) {
                bool ok = false;
                const int styleInt = parts.at(1).toInt(&ok);
                if (ok) {
                    m_activeBulletStyle = styleInt;
                }
            }
        }

        m_hasPendingUpgrade = false;
        m_currentUpgradeOptions.clear();
        emit experienceChanged(m_currentExperience, m_currentLevel, experienceToNextLevel());
    }

    /// 设置当前波次并计算动态难度倍率
    /// HP 倍率: 1.05^wave, 伤害倍率: 1.03^wave
    void setCurrentWave(int wave) override
    {
        m_currentWave = wave;
        m_enemyHpMultiplier = std::pow(1.05F, static_cast<float>(wave));
        m_enemyDmgMultiplier = std::pow(1.03F, static_cast<float>(wave));
    }

    /// 根据当前波次返回所属怪物系列
    [[nodiscard]] MonsterSeries currentSeries() const override
    {
        if (m_currentWave <= 4) return MonsterSeries::Bizarre;
        if (m_currentWave <= 7) return MonsterSeries::Undead;
        return MonsterSeries::Alien;
    }

    [[nodiscard]] float enemyHpMultiplier() const override { return m_enemyHpMultiplier; }
    [[nodiscard]] float enemyDmgMultiplier() const override { return m_enemyDmgMultiplier; }

    /// 返回并清空待生成的 Boss 列表
    [[nodiscard]] QList<EnemyId> pendingBosses() override
    {
        QList<EnemyId> result;
        result.swap(m_pendingBosses);
        return result;
    }

    /// 当前是否有 Boss 处于激活系列（Boss 已请求但尚未被击败）
    [[nodiscard]] bool isBossWave() const override
    {
        return m_activeBossSeries != MonsterSeries::None;
    }

    /// 返回当前 Boss 所属系列
    [[nodiscard]] MonsterSeries bossSeries() const override
    {
        return m_activeBossSeries;
    }

    /// game_main_page 通知 Boss 已被击败
    void notifyBossDefeated() override
    {
        // 若当前 Boss 是 UFO，将二阶段 AlienPilot 加入待生成列表
        if (m_activeBossSeries == MonsterSeries::Alien
            && m_bossUFOSpawned
            && !m_bossAlienPilotSpawned) {
            m_pendingBosses.append(EnemyId::AlienPilot);
            m_bossAlienPilotSpawned = true;
        }

        // 清除当前 Boss 系列标记
        m_activeBossSeries = MonsterSeries::None;
    }

private:
    [[nodiscard]] UpgradeOptions defaultUpgradeOptions() const
    {
        UpgradeOptions allOptions = GameConfig::kAllUpgradeOptions;
        return allOptions.mid(0, std::min(static_cast<int>(allOptions.size()), config().upgradeSelectionCount));
    }

    void prepareUpgradeOptions()
    {
        m_currentUpgradeOptions.clear();

        QList<UpgradeOption> traitPool;
        for (const auto &option : GameConfig::kAllUpgradeOptions) {
            if (option.kind != UpgradeOptionKind::Trait) {
                continue;
            }

            UpgradeOption actualOption = option;
            const int currentLevel = m_ownedTraitIds.contains(option.traitId)
                ? m_ownedTraitIds.value(option.traitId) : 0;
            actualOption.level = currentLevel + 1;
            actualOption.displayName = QStringLiteral("%1 Lv.%2")
                .arg(option.displayName, QString::number(actualOption.level));
            actualOption.optionId = QStringLiteral("%1.%2")
                .arg(option.optionId.section('.', 0, -2),
                     QString::number(actualOption.level));

            if (option.traitId == TraitId::QuickHands && actualOption.level >= 2) {
                actualOption.iconPath = QStringLiteral(":/icons/mechanical_arm_3d (1).png");
            }

            traitPool.append(actualOption);
        }

        const int traitTarget = 3;
        while (m_currentUpgradeOptions.size() < traitTarget && !traitPool.isEmpty()) {
            int totalWeight = 0;
            for (const auto &opt : traitPool) {
                switch (opt.rarity) {
                case UpgradeRarity::Common: totalWeight += 60; break;
                case UpgradeRarity::Rare:   totalWeight += 30; break;
                case UpgradeRarity::Epic:   totalWeight += 10; break;
                }
            }

            int roll = QRandomGenerator::global()->bounded(totalWeight);
            int cumulative = 0;
            int selectedIndex = -1;
            for (int i = 0; i < traitPool.size(); ++i) {
                switch (traitPool[i].rarity) {
                case UpgradeRarity::Common: cumulative += 60; break;
                case UpgradeRarity::Rare:   cumulative += 30; break;
                case UpgradeRarity::Epic:   cumulative += 10; break;
                }
                if (roll < cumulative) {
                    selectedIndex = i;
                    break;
                }
            }

            if (selectedIndex >= 0) {
                m_currentUpgradeOptions.append(traitPool.takeAt(selectedIndex));
            }
        }

        UpgradeOptions bulletPool = BulletTemplateConfig::generateSpecialBulletUpgradeOptions();
        const int bulletTarget = 2;
        while (m_currentUpgradeOptions.size() < traitTarget + bulletTarget && !bulletPool.isEmpty()) {
            const int idx = QRandomGenerator::global()->bounded(static_cast<int>(bulletPool.size()));
            m_currentUpgradeOptions.append(bulletPool.takeAt(idx));
        }

        emit upgradeOptionsPrepared(m_currentUpgradeOptions);
    }

    void setBattleState(BattleFlowState state)
    {
        if (m_battleState == state) {
            return;
        }

        m_battleState = state;
        emit battleStateChanged(m_battleState);
    }

    BattleFlowState m_battleState {BattleFlowState::Inactive};
    int m_currentRound {1};
    int m_elapsedRoundMs {0};
    int m_currentExperience {0};
    int m_currentLevel {0};
    bool m_hasPendingUpgrade {false};
    QMap<TraitId, int> m_ownedTraitIds;
    UpgradeOptions m_currentUpgradeOptions;

    // 波次追踪与动态难度
    int m_currentWave{0};
    float m_enemyHpMultiplier{1.0F};
    float m_enemyDmgMultiplier{1.0F};

    // Boss 生成追踪
    QList<EnemyId> m_pendingBosses;
    bool m_bossDemonLordSpawned{false};
    bool m_bossBoneLordSpawned{false};
    bool m_bossUFOSpawned{false};
    bool m_bossAlienPilotSpawned{false};
    MonsterSeries m_activeBossSeries{MonsterSeries::None};
    int m_activeBulletStyle{0};
};

} // namespace

WaveManager *createWaveManager(QObject *parent)
{
    return new BasicWaveManager(parent);
}

#include "wave_manager.moc"
