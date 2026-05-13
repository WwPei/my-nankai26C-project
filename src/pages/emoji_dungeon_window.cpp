#include "emoji_dungeon_window.h"

#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "class_select_page.h"
#include "game_factory.h"
#include "game_main_page.h"
#include "wave_manager.h"

EmojiDungeonWindow::EmojiDungeonWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_stack(new QStackedWidget(this))
    , m_classSelectPage(new ClassSelectPage(this))
    , m_gameMainPage(new GameMainPage(this))

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
    m_startPage->setObjectName(QStringLiteral("startPage"));

    auto *layout = new QVBoxLayout(m_startPage);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);


    m_pageHintLabel = new QLabel(m_startPage);
    m_pageHintLabel->setObjectName(QStringLiteral("startIntroLabel"));
    m_pageHintLabel->setWordWrap(true);
    m_pageHintLabel->setAlignment(Qt::AlignCenter);

    layout->addStretch();   // 弹簧，把按钮推到底部

    // ---- 底部按钮：左右并列 ----
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(40);
    buttonLayout->setContentsMargins(50, 20, 50, 60);
    buttonLayout->setAlignment(Qt::AlignHCenter);

    auto *startButton = new QPushButton(QStringLiteral("开始游戏"), m_startPage);
    startButton->setObjectName(QStringLiteral("startButton"));
    startButton->setFixedSize(180, 50);
    startButton->setCursor(Qt::PointingHandCursor);

    auto *quitButton = new QPushButton(QStringLiteral("退出"), m_startPage);
    quitButton->setObjectName(QStringLiteral("quitButton"));
    quitButton->setFixedSize(180, 50);
    quitButton->setCursor(Qt::PointingHandCursor);

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(quitButton);
    layout->addLayout(buttonLayout);

    // ---- 信号连接 ----
    connect(startButton, &QPushButton::clicked, this, [this]() {
        setCurrentPage(PageId::ClassSelect);
    });
    connect(quitButton, &QPushButton::clicked, this, &QWidget::close);

    // ---- 样式表（背景改用 border-image） ----
    m_startPage->setStyleSheet(R"(
        QWidget#startPage {
            border-image: url(:/images/emoji_dungeon.png) 0 0 0 0 stretch stretch;
            background-color: transparent;
        }
        QLabel#startTitleLabel {
            font-size: 48px;
            font-weight: 800;
            color: #ffffff;
            background: transparent;
        }
        QLabel#startIntroLabel {
            font-size: 16px;
            color: #d0d0d0;
            background: transparent;
            max-width: 500px;
        }
        QPushButton#startButton, QPushButton#quitButton {
            background: rgba(255, 255, 255, 0.08);
            border: 2px solid rgba(255, 255, 255, 0.6);
            border-radius: 10px;
            color: white;
            font-size: 18px;
            font-weight: 600;
            font-family: "Segoe UI Emoji", "Microsoft YaHei UI", sans-serif;
        }
        QPushButton#startButton:hover, QPushButton#quitButton:hover {
            background: rgba(255, 255, 255, 0.25);
            border-color: #82afff;
            color: #ffffff;
        }
        QPushButton#startButton:pressed, QPushButton#quitButton:pressed {
            background: rgba(255, 255, 255, 0.15);
            border-color: #5a8edb;
        }
    )");

    // 保存提示标签指针（用于结算文本显示）
    // 页面添加到 stack
    m_stack->addWidget(m_startPage);
    m_stack->addWidget(m_classSelectPage);
    m_stack->addWidget(m_gameMainPage);
}

void EmojiDungeonWindow::connectNavigation()
{
    connect(m_classSelectPage, &ClassSelectPage::classSelected, this, [this](PlayerClassId classId) {
        m_currentClassId = classId;
        m_selectedTraits.clear();
        if (m_pageHintLabel != nullptr) {
            m_pageHintLabel->setText(QString());
        }
        m_gameMainPage->setSelectedClass(classId);
        setCurrentPage(PageId::GameMain);
    });

    connect(m_classSelectPage, &ClassSelectPage::backRequested, this, [this]() {
        setCurrentPage(PageId::Start);
    });

    connect(m_gameMainPage, &GameMainPage::traitAcquired, this, [this](TraitId traitId) {
        if (!m_selectedTraits.contains(traitId)) {
            m_selectedTraits.push_back(traitId);
        }
    });

    connect(m_gameMainPage, &GameMainPage::exitRequested, this, [this]() {
        setCurrentPage(PageId::Start);
    });

    connect(m_gameMainPage, &GameMainPage::battleFinished, this, [this](bool victory) {
        if (m_pageHintLabel != nullptr && m_gameMainPage != nullptr
            && m_gameMainPage->waveManager() != nullptr) {
            WaveManager *waveManager = m_gameMainPage->waveManager();
            m_pageHintLabel->setText(
                victory
                    ? QStringLiteral("本局完成：10/10 波，等级 Lv.%1，经验 %2，已获特性 %3。")
                          .arg(QString::number(waveManager->currentLevel()),
                               QString::number(waveManager->currentExperience()),
                               QString::number(m_selectedTraits.size()))
                    : QStringLiteral("本局失败：止步第 %1 波，等级 Lv.%2，经验 %3，已获特性 %4。")
                          .arg(QString::number(waveManager->currentRound()),
                               QString::number(waveManager->currentLevel()),
                               QString::number(waveManager->currentExperience()),
                               QString::number(m_selectedTraits.size())));
        }
        setCurrentPage(PageId::Start);
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
    }

    setWindowTitle(QStringLiteral("Emoji Dungeon - %1").arg(pageName));
}
