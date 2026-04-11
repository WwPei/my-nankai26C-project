#ifndef UPGRADE_PAGE_H
#define UPGRADE_PAGE_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
// ==========================================
// UpgradePage 类：升级/武器选择界面
// 核心作用：展示属性加点选项、武器选择列表、道具购买界面，发送关闭信号
// 设计原则：仅做UI展示与选择逻辑，不涉及游戏战斗业务
// ==========================================
class UpgradePage : public QWidget
{
    Q_OBJECT

public:
    explicit UpgradePage(QWidget *parent = nullptr);
     void setCurrentWave(int wave);

signals:
    // ==========================================
    // 跳转信号：玩家点击"确认"或"关闭"后发送，主窗口接收后切回主游戏页
    // 后续开发提示：可以给这个信号加参数，比如传递选中的升级/武器信息
    // ==========================================
    void closeUpgradePageRequested();
private slots:
    void onConfirmClicked();

private:
    // ==========================================
    // 预留接口：后续填充UI与连接信号槽
    // ==========================================
    void setupUI();        // 填充UI：在这里添加属性加点按钮、武器选择列表、emoji道具展示等
    void connectSignals(); // 连接信号槽：在这里把"确认按钮点击"连接到 closeUpgradePageRequested 信号
    QPushButton *m_confirmBtn;
    QLabel      *m_titleLabel;
    QLabel      *m_infoLabel;
    int         m_currentWave;
};

#endif // UPGRADE_PAGE_H
