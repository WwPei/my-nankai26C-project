#include "upgrade_page.h"
#include <QHBoxLayout>

UpgradePage::UpgradePage(QWidget *parent)
    : QWidget(parent), m_currentWave(1)
{
    setupUI();
    connectSignals();
}

void UpgradePage::setCurrentWave(int wave)
{
    // ==========================================
    // TODO: 后续在这里填充升级/武器选择页的UI
    // 比如：属性加点按钮、武器选择列表、emoji道具展示等
    // ==========================================
    m_currentWave = wave;
    if (m_titleLabel) {
        m_titleLabel->setText(QString("第 %1 轮升级").arg(wave));
    }
}

void UpgradePage::setupUI()
{
    // 主布局：垂直布局，内容在顶部，按钮在底部
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(30);
    mainLayout->setContentsMargins(50, 50, 50, 50);

    // 标题
    m_titleLabel = new QLabel("升级界面", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: gold;");
    mainLayout->addWidget(m_titleLabel);

    // 中间信息区域（后续可添加能力选项）
    m_infoLabel = new QLabel("暂无可选能力，点击准备完全开始下一轮", this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setStyleSheet("font-size: 18px; color: white;");
    mainLayout->addWidget(m_infoLabel);
    mainLayout->addStretch();  // 将下方按钮推到底部

    // 底部水平布局，放置按钮在右下角
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->addStretch();  // 左侧弹性空间，将按钮推到右边
    m_confirmBtn = new QPushButton("准备完全", this);
    m_confirmBtn->setFixedSize(200, 60);
    m_confirmBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(0, 0, 0, 200);
            color: white;
            font-size: 24px;
            border: 2px solid gold;
            border-radius: 15px;
        }
        QPushButton:hover {
            background-color: rgba(60, 60, 60, 230);
        }
    )");
    bottomLayout->addWidget(m_confirmBtn);
    mainLayout->addLayout(bottomLayout);
}

void UpgradePage::onConfirmClicked()
{
    emit closeUpgradePageRequested();
}
void UpgradePage::connectSignals()
{
    // ==========================================
    // TODO: 后续在这里连接信号槽
    // 比如：connect(确认按钮, &QPushButton::clicked, this, &UpgradePage::closeUpgradePageRequested);
    // ==========================================
     connect(m_confirmBtn, &QPushButton::clicked, this, &UpgradePage::onConfirmClicked);
}
