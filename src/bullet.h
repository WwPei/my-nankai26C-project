#pragma once

#include "bullet_data.h"
#include "game_view.h"

class BulletView : public GameView
{
    Q_OBJECT

public:
    using GameView::GameView;
    ~BulletView() override = default;

    [[nodiscard]] virtual BulletData *model() const = 0;

public slots:
    virtual void bindModel(BulletData *data) = 0;
};
