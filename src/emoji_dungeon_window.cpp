#include "emoji_dungeon_window.h"

#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "class_select_page.h"
#include "game_factory.h"
#include "game_main_page.h"
#include "upgrade_page.h"

EmojiDungeonWindow::EmojiDungeonWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_stack(new QStackedWidget(this))
    , m_classSelectPage(new ClassSelectPage(this))
    , m_gameMainPage(new GameMainPage(this))
    , m_upgradePage(new UpgradePage(this))
    , m_factory(new GameFactory(this))
{
    setCentralWidget(m_stack);
    resize(1080, 720);

    m_gameMainPage->setFactory(m_factory);

    buildPages();
    connectNavigation();
    setCurrentPage(PageId::Start);
}

void EmojiDungeonWindow::buildPages()
{
    m_startPage = new QWidget(this);

    auto *layout = new QVBoxLayout(m_startPage);
    layout->setContentsMargins(32, 32, 32, 32);
    layout->setSpacing(16);

    auto *titleLabel = new QLabel(QStringLiteral("Emoji Dungeon"), m_startPage);
    auto *introLabel = new QLabel(
        QStringLiteral("Qt6 Widgets 阶段0骨架：开始页负责进入职业选择流程。"),
        m_startPage);
    auto *startButton = new QPushButton(QStringLiteral("开始游戏"), m_startPage);
    auto *quitButton = new QPushButton(QStringLiteral("退出"), m_startPage);

    introLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addWidget(introLabel);
    layout->addWidget(startButton);
    layout->addWidget(quitButton);
    layout->addStretch();

    connect(startButton, &QPushButton::clicked, this, [this]() {
        setCurrentPage(PageId::ClassSelect);
    });
    connect(quitButton, &QPushButton::clicked, this, &QWidget::close);

    m_stack->addWidget(m_startPage);
    m_stack->addWidget(m_classSelectPage);
    m_stack->addWidget(m_gameMainPage);
    m_stack->addWidget(m_upgradePage);
}

void EmojiDungeonWindow::connectNavigation()
{
    connect(m_classSelectPage, &ClassSelectPage::classSelected, this, [this](PlayerClassId classId) {
        m_currentClassId = classId;
        m_gameMainPage->setSelectedClass(classId);
        setCurrentPage(PageId::GameMain);
    });

    connect(m_classSelectPage, &ClassSelectPage::backRequested, this, [this]() {
        setCurrentPage(PageId::Start);
    });

    connect(m_gameMainPage, &GameMainPage::upgradeRequested, this, [this]() {
        setCurrentPage(PageId::Upgrade);
    });

    connect(m_gameMainPage, &GameMainPage::exitRequested, this, [this]() {
        setCurrentPage(PageId::Start);
    });

    connect(m_upgradePage, &UpgradePage::resumeRequested, this, [this]() {
        if (m_gameMainPage != nullptr) {
            m_gameMainPage->resumeBattleState();
        }
        setCurrentPage(PageId::GameMain);
    });
}

void EmojiDungeonWindow::setCurrentPage(PageId pageId)
{
    if (m_gameMainPage != nullptr && pageId != PageId::GameMain) {
        m_gameMainPage->setBattleActive(false);
    }

    m_currentPage = pageId;

    switch (pageId) {
    case PageId::Start:
        m_stack->setCurrentWidget(m_startPage);
        break;
    case PageId::ClassSelect:
        m_stack->setCurrentWidget(m_classSelectPage);
        break;
    case PageId::GameMain:
        m_stack->setCurrentWidget(m_gameMainPage);
        if (m_gameMainPage != nullptr) {
            m_gameMainPage->setBattleActive(true);
        }
        break;
    case PageId::Upgrade:
        if (m_gameMainPage != nullptr) {
            m_gameMainPage->enterUpgradeState();
        }
        m_stack->setCurrentWidget(m_upgradePage);
        break;
    }

    updateWindowTitle();
}

void EmojiDungeonWindow::updateWindowTitle()
{
    QString pageName;

    switch (m_currentPage) {
    case PageId::Start:
        pageName = QStringLiteral("开始页");
        break;
    case PageId::ClassSelect:
        pageName = QStringLiteral("职业选择");
        break;
    case PageId::GameMain:
        pageName = QStringLiteral("游戏主页面");
        break;
    case PageId::Upgrade:
        pageName = QStringLiteral("升级页面");
        break;
    }

    setWindowTitle(QStringLiteral("Emoji Dungeon - %1").arg(pageName));
}
