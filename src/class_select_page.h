#pragma once

#include <QWidget>

#include "game_data.h"

class ClassSelectPage : public QWidget
{
    Q_OBJECT

public:
    explicit ClassSelectPage(QWidget *parent = nullptr);
    ~ClassSelectPage() override = default;

signals:
    void classSelected(PlayerClassId classId);
    void backRequested();
};
