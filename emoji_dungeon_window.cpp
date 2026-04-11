#include "emoji_dungeon_window.h"
#include <QApplication>
#include<QDebug>
#include <QDir>
//#include <QMediaPlayer>    // 新增：用于背景音乐（Qt6）
//#include <QAudioOutput>    // 新增：Qt6音频输出必须


EmojiDungeonWindow::EmojiDungeonWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 1. 初始化所有页面对象（Qt对象树自动内存管理，无需手动delete）
    m_classSelectPage = new ClassSelectPage(this);
    m_gameMainPage = new GameMainPage(this);
    m_upgradePage = new UpgradePage(this);

    // 2. 搭建窗口框架
    setupWindowFramework();

    // 3. 连接页面跳转信号槽（完全解耦核心，页面间不直接依赖）
    // 职业选择页 -> 游戏主页面（带职业数据）
    connect(m_classSelectPage, &ClassSelectPage::startGameRequested,
            this, static_cast<void(EmojiDungeonWindow::*)(const ClassData&)>(&EmojiDungeonWindow::switchToGameMainPage));
    // 游戏主页面 -> 升级页面（带当前轮次）
    connect(m_gameMainPage, &GameMainPage::openUpgradePageRequested,
            this, &EmojiDungeonWindow::switchToUpgradePage);
    // 升级页面 -> 游戏主页面（无参数，返回）
    connect(m_upgradePage, &UpgradePage::closeUpgradePageRequested,
            this, static_cast<void(EmojiDungeonWindow::*)()>(&EmojiDungeonWindow::switchToGameMainPage));
}

EmojiDungeonWindow::~EmojiDungeonWindow()
{
    // Qt对象树自动管理内存，无需手动delete
}
void EmojiDungeonWindow::setupWindowFramework()
{
    this->resize(1280, 720);
    this->setWindowTitle("Emoji Dungeon");
    this->setFixedSize(1280, 720);

    m_stackedWidget = new QStackedWidget(this);
    this->setCentralWidget(m_stackedWidget);

    // ==========================================
    // 1. 创建开始界面
    // ==========================================
    QWidget *startPage = new QWidget();

    // 【修复1】只保留一个主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(startPage);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // --- (1) 插入背景图片 ---
    QLabel *bgLabel = new QLabel();
    bgLabel->setScaledContents(true);
    bgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // 【修复2】路径改成和真实文件名一致（带空格）
    // 注意：请确保你的 resource.qrc 里也是这个路径！
    QPixmap bgPixmap(":/images/images/emoji_dungeon.png");
    qDebug() << "Resources in root :" << QDir(":/").entryList();
    qDebug() << "Resources in /images :" << QDir(":/images").entryList();

    bgLabel->setPixmap(bgPixmap);
    // 把背景加入主布局
    mainLayout->addWidget(bgLabel);

    // --- (2) 创建按钮 ---
    QPushButton *startBtn = new QPushButton("开始游戏");
    QPushButton *quitBtn = new QPushButton("退出游戏");

    // 按钮美化样式
    QString btnStyle = R"(
        QPushButton {
            background-color: rgba(0, 0, 0, 200);
            color: #ffffff;
            font-size: 26px;
            font-family: "Microsoft YaHei";
            padding: 18px 55px;
            border: 2px solid #ffffff;
            border-radius: 12px;
        }
        QPushButton:hover {
            background-color: rgba(60, 60, 60, 230);
            border-color: #ffd700;
        }
    )";
    startBtn->setStyleSheet(btnStyle);
    quitBtn->setStyleSheet(btnStyle);

    // --- (3) 在背景上放置按钮 ---
    // 【修复3】创建一个新布局放在 bgLabel 上，用于按钮
    QVBoxLayout *btnLayout = new QVBoxLayout(bgLabel);
    btnLayout->addStretch(3); // 上方弹性空间
    btnLayout->addWidget(startBtn, 0, Qt::AlignCenter);
    btnLayout->addSpacing(25);
    btnLayout->addWidget(quitBtn, 0, Qt::AlignCenter);
    btnLayout->addStretch(1); // 下方弹性空间


    // --- (4) 连接按钮信号（✅ 100%正确的Lambda语法） ---
    connect(startBtn, &QPushButton::clicked, this,[this]  {
        m_stackedWidget->setCurrentIndex(PAGE_CLASS_SELECT);
    });
    connect(quitBtn, &QPushButton::clicked, this, [this] {
        QApplication::quit();
    });
    // ==========================================
    // 2. 添加所有页面到 QStackedWidget
    // ==========================================
    m_stackedWidget->addWidget(startPage);
    m_stackedWidget->addWidget(m_classSelectPage);
    m_stackedWidget->addWidget(m_gameMainPage);
    m_stackedWidget->addWidget(m_upgradePage);

    // ==========================================
    // 3. 默认显示开始界面
    // ==========================================
    m_stackedWidget->setCurrentIndex(PAGE_START);
}

// ---------------- 页面跳转实现 ----------------
// 无参版本：从升级页面返回时调用（准备完全按钮）
void EmojiDungeonWindow::switchToGameMainPage()
{
    // 通知游戏页面继续下一轮
    m_gameMainPage->onUpgradePageClosed();
    // 切换回游戏页面
    m_stackedWidget->setCurrentIndex(PAGE_GAME_MAIN);
    // TODO: 后续可以在这里调用 m_gameMainPage->startGameLoop() 启动游戏循环
}

// 带参版本：从职业选择页进入，携带职业数据
void EmojiDungeonWindow::switchToGameMainPage(const ClassData &classData)
{
    // 将职业数据传递给游戏页面
    m_gameMainPage->setPlayerClass(classData);
    // 启动游戏（从第1轮开始）
    m_gameMainPage->startGame();
    // 切换到游戏页面
    m_stackedWidget->setCurrentIndex(PAGE_GAME_MAIN);
}

// 带参版本：从游戏页面进入升级页面，携带当前轮次
void EmojiDungeonWindow::switchToUpgradePage(int currentWave)
{
    // 将轮次传递给升级页面（用于显示）
    m_upgradePage->setCurrentWave(currentWave);
    // 切换到升级页面
    m_stackedWidget->setCurrentIndex(PAGE_UPGRADE);
}

void EmojiDungeonWindow::switchToClassSelectPage()
{
    m_stackedWidget->setCurrentIndex(PAGE_CLASS_SELECT);
}
