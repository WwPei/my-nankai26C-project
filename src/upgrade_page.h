#pragma once

#include <QObject>
#include <QWidget>

#include "game_data.h"

class QVBoxLayout;

class UpgradePage : public QWidget
{
    Q_OBJECT

public:
    explicit UpgradePage(QWidget *parent = nullptr);
    ~UpgradePage() override = default;

    void setUpgradeOptions(const UpgradeOptions &options);
    [[nodiscard]] const UpgradeOptions &upgradeOptions() const noexcept;

signals:
    void upgradeOptionSelected(UpgradeOption option);
    void traitSelected(TraitId traitId);
    void resumeRequested();

private:
    void rebuildOptionButtons();

    QVBoxLayout *m_optionLayout {nullptr};
    UpgradeOptions m_upgradeOptions;
};
