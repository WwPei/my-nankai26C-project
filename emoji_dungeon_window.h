#ifndef EMOJI_DUNGEON_WINDOW_H
#define EMOJI_DUNGEON_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include "class_select_page.h"
#include "game_main_page.h"
#include "upgrade_page.h"
#include "game_data.h"

// 页面索引枚举（顺序与 stacked widget 添加顺序一致）
enum PageIndex {
    PAGE_START = 0,      // 开始界面
    PAGE_CLASS_SELECT,   // 职业/初始武器选择页
    PAGE_GAME_MAIN,      // 主游戏页
    PAGE_UPGRADE         // 升级页
};

class EmojiDungeonWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EmojiDungeonWindow(QWidget *parent = nullptr);
    ~EmojiDungeonWindow();

private slots:
    /**
     * @brief 从升级页面返回游戏主页面（无参版本）
     *        通知游戏页面进入下一轮，并切换界面。
     */
    void switchToGameMainPage();

    /**
     * @brief 从职业选择页进入游戏（带职业数据）
     * @param classData 玩家选择的职业/初始武器数据
     */
    void switchToGameMainPage(const ClassData &classData);

    /**
     * @brief 从游戏主页面进入升级页面
     * @param currentWave 当前轮次
     */
    void switchToUpgradePage(int currentWave);

    /**
     * @brief 切换到职业选择页（例如从开始界面点击“开始游戏”）
     */
    void switchToClassSelectPage();

private:
    QStackedWidget *m_stackedWidget;   // 页面堆叠容器

    // 各页面实例
    ClassSelectPage *m_classSelectPage;
    GameMainPage   *m_gameMainPage;
    UpgradePage    *m_upgradePage;

    /**
     * @brief 初始化窗口框架（大小、堆叠控件、开始界面）
     */
    void setupWindowFramework();
};

#endif // EMOJI_DUNGEON_WINDOW_H
