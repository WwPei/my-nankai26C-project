#pragma once

#include <QMainWindow>

#include "game_data.h"

class QLabel;
class QStackedWidget;
class QWidget;
class ClassSelectPage;
class GameFactory;
class GameMainPage;
class UpgradePage;

class EmojiDungeonWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EmojiDungeonWindow(QWidget *parent = nullptr);
    ~EmojiDungeonWindow() override = default;

private:
    void buildPages();
    void connectNavigation();
    void setCurrentPage(PageId pageId);
    void updateWindowTitle();

    QStackedWidget *m_stack {nullptr};
    QWidget *m_startPage {nullptr};
    QLabel *m_pageHintLabel {nullptr};
    ClassSelectPage *m_classSelectPage {nullptr};
    GameMainPage *m_gameMainPage {nullptr};
    UpgradePage *m_upgradePage {nullptr};
    GameFactory *m_factory {nullptr};
    PlayerClassId m_currentClassId {PlayerClassId::Warrior};
    PageId m_currentPage {PageId::Start};
};
