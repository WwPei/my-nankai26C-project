#pragma once

#include <QObject>
#include <QString>

#include "game_data.h"

class Player;

class Trait : public QObject
{
    Q_OBJECT

public:
    using QObject::QObject;
    ~Trait() override = default;

    [[nodiscard]] virtual TraitId id() const = 0;
    [[nodiscard]] virtual QString displayName() const = 0;
    [[nodiscard]] virtual QString description() const = 0;

public slots:
    virtual void applyToPlayer(Player *player) = 0;
};
