#pragma once

#include "enemy_data.h"
#include "game_view.h"

class EnemyView : public GameView
{
    Q_OBJECT

public:
    using GameView::GameView;
    ~EnemyView() override = default;

    [[nodiscard]] virtual EnemyData *model() const = 0;

public slots:
    virtual void bindModel(EnemyData *data) = 0;
};
