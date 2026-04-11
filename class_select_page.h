#ifndef CLASS_SELECT_PAGE_H
#define CLASS_SELECT_PAGE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>     // 用于管理职业互斥选中
//#include <QSoundEffect>     // 预留音频，49行有相关代码（以注释）
#include<QresizeEvent>
#include"game_data.h"
class ClassSelectPage : public QWidget
{
    Q_OBJECT

public:
    explicit ClassSelectPage(QWidget *parent = nullptr);
    ~ClassSelectPage();

signals:
    // 开始游戏信号，携带选中的职业名称
    void startGameRequested(const ClassData &className);
    // 返回开始界面信号
    void backToStartRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;   // 窗口大小改变时调整浮动面板尺寸

private slots:
    void onClassSelected(int id);        // 处理职业选中（使用按钮组ID）
    void onConfirmClicked();
    void onBackClicked();

private:
    void setupUI();
    void connectSignals();
    void setupAudio();          // 预留：初始化音频资源
    void playClickSound();      // 预留：播放点击音效

    // UI 控件
    QWidget    *m_centerPanel;           // 浮动面板
    QVBoxLayout *m_panelLayout;          // 面板内布局，方便后续添加更多职业
    QButtonGroup *m_classGroup;          // 职业按钮组，管理互斥选中
    QList<QPushButton*> m_classBtns;     // 存储职业按钮，便于扩展
    QList<QLabel*> m_classIcons;         // 存储职业图标，便于扩展
    QPushButton *m_confirmBtn;
    QPushButton *m_backBtn;

    QString     m_selectedClass;         // 当前选中的职业名称

    // 音频相关（预留）
   // QSoundEffect *m_clickSound;          // 按钮点击音效
    // 可扩展其他音效，如 m_selectSound, m_confirmSound 等
};

#endif // CLASS_SELECT_PAGE_H
