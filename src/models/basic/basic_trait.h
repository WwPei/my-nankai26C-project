#pragma once

#include <QObject>

#include "trait.h"

class BasicTrait final : public Trait
{
    Q_OBJECT

public:
    explicit BasicTrait(const TraitConfig *config, QObject *parent = nullptr);

    [[nodiscard]] TraitId id() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QString description() const override;

public slots:
    void applyToPlayer(Player *player) override;

private:
    const TraitConfig *m_config {nullptr};
};
