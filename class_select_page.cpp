#include "class_select_page.h"
#include <QGridLayout>
#include <QFrame>
#include <QPixmap>
#include <QDir>
#include <QDebug>

// ==================== 构造函数 / 析构函数 ====================
ClassSelectPage::ClassSelectPage(QWidget *parent)
    : QWidget(parent)
    , m_selectedClass("")
    , m_classGroup(nullptr)
   // , m_clickSound(nullptr)
{
    setupUI();          // 创建界面
    connectSignals();   // 连接信号槽
    setupAudio();       // 预留：加载音频
}

ClassSelectPage::~ClassSelectPage()
{
    //delete m_clickSound;   // 释放音频资源（Qt对象树也可自动释放，显式更安全）
}

// ==================== UI 搭建 ====================
void ClassSelectPage::setupUI()
{
    // ----------------------------------------------------------
    // 1. 设置整个页面的背景（可替换为图片或纯色）
    // ----------------------------------------------------------
    this->setStyleSheet("ClassSelectPage { background-image: url(:/images/class_select_bg.png); "
                        "background-position: center; background-repeat: no-repeat; background-size: cover; }");
    // 如果没有背景图，可以使用纯色： this->setStyleSheet("ClassSelectPage { background-color: #2c3e50; }");

    // ----------------------------------------------------------
    // 2. 创建浮动面板（半透明黑底金边）
    // ----------------------------------------------------------
    m_centerPanel = new QWidget(this);
    m_centerPanel->setObjectName("CenterPanel");
    m_centerPanel->setStyleSheet("#CenterPanel { background-color: rgba(0, 0, 0, 180); "
                                 "border-radius: 20px; border: 2px solid #ffd700; }");

    // 面板内部使用垂直布局
    m_panelLayout = new QVBoxLayout(m_centerPanel);
    m_panelLayout->setSpacing(25);
    m_panelLayout->setContentsMargins(40, 40, 40, 40);

    // ----------------------------------------------------------
    // 3. 创建两个职业的选项（每个职业：图标 + 按钮）
    //    后续添加更多职业时，只需在此处增加一组，并添加到 m_classGroup
    // ----------------------------------------------------------
    // 职业数据：名称和预留图标路径（可修改）
    struct ClassInfo {
        QString name;
        QString iconPath;   // 资源路径，如 ":/images/archer_icon.png"
    };
    QList<ClassInfo> classes = {
        {"远程", ":/images/archer_icon.png"},
        {"近战", ":/images/warrior_icon.png"}
    };

    m_classGroup = new QButtonGroup(this);
    m_classGroup->setExclusive(true);   // 互斥选中

    for (int i = 0; i < classes.size(); ++i) {
        const ClassInfo &info = classes[i];

        // 每个职业的容器（水平布局：图标 + 按钮）
        QWidget *classWidget = new QWidget();
        QHBoxLayout *classLayout = new QHBoxLayout(classWidget);
        classLayout->setSpacing(20);
        classLayout->setContentsMargins(0, 0, 0, 0);

        // 职业图标（预留）
        QLabel *iconLabel = new QLabel();
        iconLabel->setFixedSize(80, 80);
        iconLabel->setScaledContents(true);
        iconLabel->setAlignment(Qt::AlignCenter);
        // 尝试加载图标，如果失败则显示占位圆形
        QPixmap iconPixmap(info.iconPath);
        if (iconPixmap.isNull()) {
            // 占位：灰色圆形
            iconLabel->setStyleSheet("background-color: #888; border-radius: 40px;");
        } else {
            iconLabel->setPixmap(iconPixmap);
        }
        classLayout->addWidget(iconLabel);

        // 职业按钮（可选中）
        QPushButton *classBtn = new QPushButton(info.name);
        classBtn->setFixedSize(180, 60);
        classBtn->setCheckable(true);
        classBtn->setStyleSheet(R"(
            QPushButton {
                background-color: rgba(30, 30, 30, 200);
                color: white;
                font-size: 22px;
                font-weight: bold;
                border: 2px solid #aaa;
                border-radius: 15px;
            }
            QPushButton:hover {
                background-color: rgba(80, 80, 80, 220);
                border-color: #ffd700;
            }
            QPushButton:checked {
                background-color: rgba(0, 120, 0, 220);
                border-color: #ffd700;
                border-width: 3px;
            }
        )");
        classLayout->addWidget(classBtn);
        classLayout->addStretch();  // 右侧弹性空间，使内容靠左

        // 添加到面板布局
        m_panelLayout->addWidget(classWidget);

        // 记录控件，便于后续扩展（如动态修改图标、文本）
        m_classIcons.append(iconLabel);
        m_classBtns.append(classBtn);

        // 添加到按钮组，ID 即为索引 i
        m_classGroup->addButton(classBtn, i);
    }

    // 添加弹性空间，使职业列表垂直居中（如果职业较少）
    m_panelLayout->addStretch();

    // ----------------------------------------------------------
    // 4. 将浮动面板放置到整个页面的中央，并设置大小比例
    // ----------------------------------------------------------
    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_centerPanel, 0, 0, 1, 1, Qt::AlignCenter);

    // ----------------------------------------------------------
    // 5. 底部按钮区域（确认选择 / 后退）
    // ----------------------------------------------------------
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(50);

    m_confirmBtn = new QPushButton("确认选择");
    m_backBtn = new QPushButton("后退");
    QString btnStyle = R"(
        QPushButton {
            background-color: rgba(0, 0, 0, 200);
            color: white;
            font-size: 20px;
            padding: 12px 30px;
            border: 2px solid #ffd700;
            border-radius: 12px;
        }
        QPushButton:hover {
            background-color: rgba(60, 60, 60, 230);
        }
    )";
    m_confirmBtn->setStyleSheet(btnStyle);
    m_backBtn->setStyleSheet(btnStyle);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_confirmBtn);
    buttonLayout->addWidget(m_backBtn);
    buttonLayout->addStretch();

    // 将按钮布局添加到底部，并设置行拉伸（面板占2/3，按钮区域占1/3）
    mainLayout->addLayout(buttonLayout, 1, 0, Qt::AlignBottom);
    mainLayout->setRowStretch(0, 2);
    mainLayout->setRowStretch(1, 1);
}

// ==================== 信号槽连接 ====================
void ClassSelectPage::connectSignals()
{
    // 职业按钮组：当选中改变时触发
    connect(m_classGroup, QOverload<int>::of(&QButtonGroup::idClicked),
            this, &ClassSelectPage::onClassSelected);
    // 确认按钮
    connect(m_confirmBtn, &QPushButton::clicked, this, &ClassSelectPage::onConfirmClicked);
    // 后退按钮
    connect(m_backBtn, &QPushButton::clicked, this, &ClassSelectPage::onBackClicked);
}

// ==================== 预留音频功能 ====================
void ClassSelectPage::setupAudio()
{
    // ==========================================================
    // 预留：音频资源加载位置
    // 后续添加音效时：
    //   1. 将音频文件（如 click.wav）添加到 .qrc 资源文件
    //   2. 取消下方代码注释
    //   3. 在 CMakeLists.txt 中添加 Qt6::Multimedia 模块
    // ==========================================================
    /*
    m_clickSound = new QSoundEffect(this);
    m_clickSound->setSource(QUrl("qrc:/sounds/click.wav"));
    m_clickSound->setLoopCount(1);
    m_clickSound->setVolume(0.5f);
    */
}

void ClassSelectPage::playClickSound()
{
    // 预留：播放点击音效
    /*
    if (m_clickSound && m_clickSound->isLoaded()) {
        m_clickSound->play();
    }
    */
}

// ==================== 槽函数实现 ====================
void ClassSelectPage::onClassSelected(int id)
{
    // 根据按钮组ID获取对应的职业名称
    if (id >= 0 && id < m_classBtns.size()) {
        m_selectedClass = m_classBtns[id]->text();
        qDebug() << "选中职业：" << m_selectedClass << "(ID=" << id << ")";
        // 预留：播放选中音效
        // playClickSound();   // 可以换专门的选中音效
    } else {
        m_selectedClass = "";
        qDebug() << "取消选中（不应发生，因为互斥组总会有一个选中）";
    }
}

void ClassSelectPage::onConfirmClicked()
{
    // 预留：播放确认音效
    // playClickSound();

    // 获取当前选中的职业名称
    QString selectedClassName;
    for (QPushButton* btn : m_classBtns) {
        if (btn->isChecked()) {
            selectedClassName = btn->text();
            break;
        }
    }

    // 如果没有选中任何职业，默认选中第一个
    if (selectedClassName.isEmpty() && !m_classBtns.isEmpty()) {
        selectedClassName = m_classBtns.first()->text();
        m_classBtns.first()->setChecked(true);
        qDebug() << "未显式选择，默认选中：" << selectedClassName;
    }

    if (selectedClassName.isEmpty()) {
        qDebug() << "错误：没有可用的职业！";
        return;
    }

    // 根据职业名称构建 ClassData 对象（数值可在此调整）
    ClassData data;
    data.name = selectedClassName;
    data.iconPath = "";  // 预留图标路径，暂时为空

    if (selectedClassName == "远程") {
        data.hp = 80;
        data.strength = 12;
        data.attackSpeed = 1.5;
    } else if (selectedClassName == "近战") {
        data.hp = 120;
        data.strength = 18;
        data.attackSpeed = 0.8;
    } else {
        // 默认值（如果以后添加新职业，在此处补充）
        data.hp = 100;
        data.strength = 10;
        data.attackSpeed = 1.0;
    }

    // 发射完整的职业数据
    emit startGameRequested(data);
}
void ClassSelectPage::onBackClicked()
{
    // 预留：播放后退音效
    // playClickSound();
    emit backToStartRequested();
}

// ==================== 窗口大小自适应 ====================
void ClassSelectPage::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // 使浮动面板宽度为窗口宽度的 1/3，高度为窗口高度的 2/3
    int panelWidth = this->width() / 3;
    int panelHeight = this->height() * 2 / 3;
    if (panelWidth > 0 && panelHeight > 0) {
        m_centerPanel->setFixedSize(panelWidth, panelHeight);
    }
}
