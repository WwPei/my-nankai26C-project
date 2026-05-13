#include "upgrade_page.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

#include "game_enums.h"

namespace {

const char *kUpgradePageStyle = R"(
QWidget#upgradePage {
    background-color: #1b1d21;
    color: #f3f4f7;
    font-family: "Segoe UI Emoji", "Microsoft YaHei UI", sans-serif;
}
QFrame#upgradeOptionsPanel,
QFrame#upgradePreviewPanel {
    background-color: #20242b;
    border: 1px solid #323846;
    border-radius: 18px;
}
QLabel#upgradeTitleLabel {
    font-size: 26px;
    font-weight: 700;
    color: #ffffff;
}
QLabel#upgradeHintLabel {
    color: #b8c1cf;
    padding: 4px 0 8px 0;
}
QLabel#upgradeSectionTitleLabel {
    font-size: 15px;
    font-weight: 700;
    color: #f6f8fc;
}
QLabel#upgradePreviewNameLabel {
    font-size: 20px;
    font-weight: 700;
    color: #ffffff;
}
QLabel#upgradePreviewDetailLabel {
    color: #c9d2df;
    line-height: 1.4;
}
QLabel#upgradeTraitsLabel {
    color: #8fc7ff;
    background-color: #262d39;
    border: 1px solid #3a4558;
    border-radius: 10px;
    padding: 10px 12px;
}
/* 旧按钮样式 (向后兼容, rebuildOptionButtons 中会动态覆盖) */
QPushButton#upgradeOptionButton {
    min-height: 92px;
    padding: 14px 16px;
    border-radius: 14px;
    border: 1px solid #3e4657;
    background-color: #252a33;
    color: #f5f7fb;
    text-align: left;
    font-size: 14px;
    font-weight: 600;
}
QPushButton#upgradeOptionButton:hover {
    border-color: #6ea8ff;
    background-color: #2d3440;
}
QPushButton#upgradeOptionButton[selected="true"] {
    border: 2px solid #82afff;
    background-color: #314560;
}
QPushButton#upgradeConfirmButton {
    min-height: 42px;
    border-radius: 12px;
    border: 1px solid #6b88b8;
    background-color: #335b9d;
    color: #f7f9fd;
    font-size: 15px;
    font-weight: 700;
}
QPushButton#upgradeConfirmButton:hover {
    background-color: #4472bc;
}
QPushButton#upgradeConfirmButton:disabled {
    border-color: #485261;
    background-color: #28303b;
    color: #7b8798;
}
QLabel#attributeRowLabel {
    background-color: #252c37;
    border: 1px solid #374153;
    border-radius: 12px;
    padding: 10px 12px;
    color: #edf1f8;
}
/* 卡片容器样式 */
QFrame#cardsContainer {
    background-color: #20242b;
    border: 1px solid #323846;
    border-radius: 18px;
}
)";

struct ComputedPreviewStats {
    QString weaponName;
    float damagePerShot {0.0F};
    float attackPerSecond {0.0F};
    float moveSpeed {0.0F};
    float defenseMultiplier {1.0F};
};

[[nodiscard]] float accumulateTraitMultiplier(const QList<TraitId> &traitIds,
                                             float TraitConfig::*member)
{
    float multiplier = 1.0F;
    for (const TraitId traitId : traitIds) {
        const TraitConfig *config = GameConfig::findTraitConfig(traitId);
        if (config == nullptr) {
            continue;
        }
        multiplier *= config->*member;
    }
    return multiplier;
}

[[nodiscard]] ComputedPreviewStats computePreviewStats(const UpgradePreviewContext &context)
{
    ComputedPreviewStats stats;

    const PlayerClassConfig *classConfig = GameConfig::findPlayerClassConfig(context.classId);
    const WeaponId weaponId = context.weaponId;
    const WeaponConfig *weaponConfig = GameConfig::findWeaponConfig(weaponId);

    stats.weaponName = weaponConfig != nullptr
        ? weaponConfig->displayName
        : QStringLiteral("未知武器");

    const float damageMultiplier = accumulateTraitMultiplier(context.ownedTraits, &TraitConfig::damageMultiplier);
    const float defenseMultiplier = accumulateTraitMultiplier(context.ownedTraits, &TraitConfig::defenseMultiplier);
    const float speedMultiplier = accumulateTraitMultiplier(context.ownedTraits, &TraitConfig::speedMultiplier);

    stats.damagePerShot = weaponConfig != nullptr ? weaponConfig->baseDamage * damageMultiplier : 0.0F;
    stats.attackPerSecond = weaponConfig != nullptr && weaponConfig->fireIntervalMs > 0.0F
        ? 1000.0F * speedMultiplier / weaponConfig->fireIntervalMs
        : 0.0F;
    stats.moveSpeed = classConfig != nullptr ? classConfig->moveSpeed * speedMultiplier : 0.0F;
    stats.defenseMultiplier = std::max(0.1F, defenseMultiplier);

    return stats;
}

[[nodiscard]] UpgradePreviewContext previewContextAfterSelection(const UpgradePreviewContext &current,
                                                                 const UpgradeOption &option)
{
    UpgradePreviewContext next = current;

    if (option.kind == UpgradeOptionKind::Trait && !next.ownedTraits.contains(option.traitId)) {
        next.ownedTraits.push_back(option.traitId);
    }

    return next;
}

[[nodiscard]] QString formatSignedDelta(float value, int decimals, const QString &suffix = QString())
{
    const QString sign = value > 0.0F ? QStringLiteral("+") : QString();
    return QStringLiteral("%1%2%3").arg(sign,
                                        QString::number(value, 'f', decimals),
                                        suffix);
}

[[nodiscard]] QString richDeltaText(const QString &label,
                                    const QString &currentValue,
                                    const QString &nextValue,
                                    const QString &deltaText,
                                    bool changed,
                                    bool positiveChange)
{
    const QString deltaColor = !changed
        ? QStringLiteral("#93a0b4")
        : (positiveChange ? QStringLiteral("#57d18c") : QStringLiteral("#ff7b7b"));
    const QString deltaValue = changed ? deltaText : QStringLiteral("无变化");

    return QStringLiteral("<b>%1</b><br/>"
                          "<span style='color:#9aa8bc;'>当前：</span> %2"
                          "<span style='color:#9aa8bc;'>  →  选择后：</span> %3"
                          "<span style='color:%4;'>  [%5]</span>")
        .arg(label, currentValue, nextValue, deltaColor, deltaValue);
}

[[nodiscard]] QString traitListText(const QList<TraitId> &traitIds)
{
    QStringList names;
    for (const TraitId traitId : traitIds) {
        const TraitConfig *config = GameConfig::findTraitConfig(traitId);
        if (config != nullptr) {
            names.push_back(config->displayName);
        }
    }
    return names.isEmpty() ? QStringLiteral("未获得") : names.join(QStringLiteral(" / "));
}

} // namespace

UpgradePage::UpgradePage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("upgradePage"));
    setStyleSheet(QString::fromUtf8(kUpgradePageStyle));

    // --- 根布局: 标题 + 提示 + 内容区(QHBoxLayout: 左侧卡片 | 右侧预览) + 确认按钮 ---
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(24, 24, 24, 24);
    rootLayout->setSpacing(16);

    // 标题
    auto *titleLabel = new QLabel(QStringLiteral("选择升级"), this);
    titleLabel->setObjectName(QStringLiteral("upgradeTitleLabel"));
    titleLabel->setWordWrap(true);
    rootLayout->addWidget(titleLabel);

    // 提示
    m_hintLabel = new QLabel(QStringLiteral("本波结算后必须选择 1 个升级并确认，清场后才会进入下一波战斗。"), this);
    m_hintLabel->setObjectName(QStringLiteral("upgradeHintLabel"));
    m_hintLabel->setWordWrap(true);
    rootLayout->addWidget(m_hintLabel);

    // --- 内容区 (无滚动, HBox: cards | preview) ---
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(16);

    m_cardsContainer = new QFrame(this);
    m_cardsContainer->setObjectName(QStringLiteral("cardsContainer"));
    QVBoxLayout *cardsOuterLayout = new QVBoxLayout(m_cardsContainer);
    cardsOuterLayout->setContentsMargins(12, 12, 12, 12);
    cardsOuterLayout->setSpacing(4);

    auto *cardsTitleLabel = new QLabel(QStringLiteral("候选项"), m_cardsContainer);
    cardsTitleLabel->setObjectName(QStringLiteral("upgradeSectionTitleLabel"));
    cardsOuterLayout->addWidget(cardsTitleLabel);

    // 卡片列表将插入到这个 layout 中
    QVBoxLayout *cardsInnerLayout = new QVBoxLayout();
    cardsInnerLayout->setSpacing(10);
    cardsOuterLayout->addLayout(cardsInnerLayout);
    cardsOuterLayout->addStretch(1);

    // 把 cardsInnerLayout 当成后续添加卡片的目标 (重命名为内部 layout)
    // 技巧: 用 m_cardsContainer 的子 layout 来管理卡片
    contentLayout->addWidget(m_cardsContainer, 3);

    auto *previewPanel = new QFrame(this);
    previewPanel->setObjectName(QStringLiteral("upgradePreviewPanel"));
    auto *previewLayout = new QVBoxLayout(previewPanel);
    previewLayout->setContentsMargins(18, 18, 18, 18);
    previewLayout->setSpacing(10);

    auto *previewTitle = new QLabel(QStringLiteral("右侧属性变化"), previewPanel);
    previewTitle->setObjectName(QStringLiteral("upgradeSectionTitleLabel"));
    previewLayout->addWidget(previewTitle);

    m_previewTitleLabel = new QLabel(QStringLiteral("未选择升级"), previewPanel);
    m_previewTitleLabel->setObjectName(QStringLiteral("upgradeHintLabel"));
    previewLayout->addWidget(m_previewTitleLabel);

    m_previewNameLabel = new QLabel(QStringLiteral("请选择左侧候选项"), previewPanel);
    m_previewNameLabel->setObjectName(QStringLiteral("upgradePreviewNameLabel"));
    m_previewNameLabel->setWordWrap(true);
    previewLayout->addWidget(m_previewNameLabel);

    m_previewDetailLabel = new QLabel(QStringLiteral("选中后会在这里显示效果说明和数值变化。"), previewPanel);
    m_previewDetailLabel->setObjectName(QStringLiteral("upgradePreviewDetailLabel"));
    m_previewDetailLabel->setWordWrap(true);
    previewLayout->addWidget(m_previewDetailLabel);

    auto *traitsTitle = new QLabel(QStringLiteral("当前已获特性"), previewPanel);
    traitsTitle->setObjectName(QStringLiteral("upgradeSectionTitleLabel"));
    previewLayout->addWidget(traitsTitle);

    m_ownedTraitsLabel = new QLabel(QStringLiteral("未获得"), previewPanel);
    m_ownedTraitsLabel->setObjectName(QStringLiteral("upgradeTraitsLabel"));
    m_ownedTraitsLabel->setWordWrap(true);
    previewLayout->addWidget(m_ownedTraitsLabel);

    auto *attributeTitle = new QLabel(QStringLiteral("属性预览"), previewPanel);
    attributeTitle->setObjectName(QStringLiteral("upgradeSectionTitleLabel"));
    previewLayout->addWidget(attributeTitle);

    m_attributeLayout = new QVBoxLayout();
    m_attributeLayout->setSpacing(8);
    previewLayout->addLayout(m_attributeLayout);
    previewLayout->addStretch();

    contentLayout->addWidget(previewPanel, 2);
    rootLayout->addLayout(contentLayout, 1);

    // --- 确认按钮 ---
    m_confirmButton = new QPushButton(QStringLiteral("请先选择左侧升级项"), this);
    m_confirmButton->setObjectName(QStringLiteral("upgradeConfirmButton"));
    m_confirmButton->setEnabled(false);

    // 确认按钮点击：同时发出新旧信号
    connect(m_confirmButton, &QPushButton::clicked, this, [this]() {
        if (m_selectedOptionIndex < 0 || m_selectedOptionIndex >= m_upgradeOptions.size()) {
            return;
        }

        const UpgradeOption option = m_upgradeOptions.at(m_selectedOptionIndex);

        // 旧信号 (向后兼容)
        if (option.kind == UpgradeOptionKind::Trait) {
            emit traitSelected(option.traitId);
        }
        emit upgradeOptionSelected(option);

        // 新信号
        if (option.kind == UpgradeOptionKind::Weapon) {
            emit weaponUpgradeSelected(option.weaponUpgradeId);
        }
        if (option.kind == UpgradeOptionKind::Stat) {
            const QStringList parts = option.optionId.split('.');
            if (parts.size() >= 2) {
                bool ok = false;
                const int styleInt = parts.at(1).toInt(&ok);
                if (ok) {
                    emit bulletStyleSelected(static_cast<BulletStyle>(styleInt));
                }
            }
        }
        emit confirmed();
    });

    rootLayout->addWidget(m_confirmButton);

    // 初始数据
    setPreviewContext(UpgradePreviewContext {});
    setUpgradeOptions(GameConfig::kUpgradeOptions);
}

void UpgradePage::setUpgradeOptions(const UpgradeOptions &options)
{
    const QString previousOptionId = m_selectedOptionId;
    m_upgradeOptions = options;
    m_selectedOptionIndex = findOptionIndexById(previousOptionId);
    m_selectedOptionId = m_selectedOptionIndex >= 0
        ? m_upgradeOptions.at(m_selectedOptionIndex).optionId
        : QString();
    buildOptionCards();
    updatePreviewPanel();
}

void UpgradePage::setPreviewContext(const UpgradePreviewContext &context)
{
    m_previewContext = context;
    updatePreviewPanel();
}

const UpgradeOptions &UpgradePage::upgradeOptions() const noexcept
{
    return m_upgradeOptions;
}

void UpgradePage::loadOptions(const UpgradeOptions &options, const UpgradePreviewContext &context)
{
    m_upgradeOptions = options;
    m_previewContext = context;
    m_selectedOptionIndex = -1;
    m_selectedOptionId.clear();
    m_confirmButton->setEnabled(false);
    m_confirmButton->setText(QStringLiteral("请先选择左侧升级项"));
    buildOptionCards();
    rebuildPreviewPanel();
}

bool UpgradePage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            for (int i = 0; i < m_cards.size(); ++i) {
                if (watched == m_cards[i]) {
                    selectCard(i);
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void UpgradePage::buildOptionCards()
{
    // 清除旧卡片
    for (auto *card : m_cards) {
        card->removeEventFilter(this);
        card->deleteLater();
    }
    m_cards.clear();

    // 找到 m_cardsContainer 的内部卡片 layout
    QVBoxLayout *containerLayout = qobject_cast<QVBoxLayout *>(m_cardsContainer->layout());
    if (containerLayout == nullptr) return;

    // containerLayout 结构: [titleLabel, innerLayout, stretch]
    // innerLayout 是第2个 (index 1)
    QLayoutItem *innerItem = containerLayout->itemAt(1);
    if (innerItem == nullptr) return;
    QVBoxLayout *cardsLayout = qobject_cast<QVBoxLayout *>(innerItem->layout());
    if (cardsLayout == nullptr) return;

    // 清空旧卡片
    while (cardsLayout->count() > 0) {
        QLayoutItem *item = cardsLayout->takeAt(0);
        delete item;
    }

    for (int i = 0; i < m_upgradeOptions.size(); ++i) {
        const auto &option = m_upgradeOptions[i];

        QFrame *card = new QFrame(m_cardsContainer);
        card->setCursor(Qt::PointingHandCursor);
        card->setMinimumHeight(80);
        card->setProperty("cardIndex", i);

        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(12, 8, 12, 8);
        cardLayout->setSpacing(10);

        // --- 图标 ---
        QLabel *iconLabel = new QLabel(card);
        iconLabel->setFixedSize(56, 56);
        iconLabel->setAlignment(Qt::AlignCenter);
        QPixmap pix(option.iconPath);
        if (pix.isNull()) {
            // 根据种类显示不同 emoji
            switch (option.kind) {
            case UpgradeOptionKind::Trait:  iconLabel->setText(QStringLiteral("\u2B50")); break;
            case UpgradeOptionKind::Weapon: iconLabel->setText(QStringLiteral("\u2694\uFE0F")); break;
            case UpgradeOptionKind::Stat:   iconLabel->setText(QStringLiteral("\U0001F4C8")); break;
            }
            iconLabel->setStyleSheet(QStringLiteral("font-size: 28px; color: #b0b8c8;"));
        } else {
            iconLabel->setPixmap(pix.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        cardLayout->addWidget(iconLabel);

        // --- 文字 ---
        QVBoxLayout *textLayout = new QVBoxLayout();
        textLayout->setSpacing(2);

        QLabel *titleLabel = new QLabel(option.displayName, card);
        titleLabel->setStyleSheet(QStringLiteral("color: white; font-size: 14px; font-weight: bold;"));
        textLayout->addWidget(titleLabel);

        QString rarityText;
        switch (option.rarity) {
        case UpgradeRarity::Common: rarityText = QStringLiteral("普通"); break;
        case UpgradeRarity::Rare:   rarityText = QStringLiteral("稀有"); break;
        case UpgradeRarity::Epic:   rarityText = QStringLiteral("史诗"); break;
        }

        QLabel *summaryLabel = new QLabel(
            QStringLiteral("[%1] %2%3")
                .arg(rarityText, option.summary,
                     option.level > 1 ? QStringLiteral(" Lv.%1").arg(option.level) : QString()),
            card);
        summaryLabel->setStyleSheet(QStringLiteral("color: #8a90a0; font-size: 11px;"));
        summaryLabel->setWordWrap(true);
        textLayout->addWidget(summaryLabel);

        cardLayout->addLayout(textLayout, 1);

        // --- 稀有度边框颜色 ---
        QString borderColor;
        switch (option.rarity) {
        case UpgradeRarity::Common: borderColor = QStringLiteral("#5a6070"); break;
        case UpgradeRarity::Rare:   borderColor = QStringLiteral("#5a8fef"); break;
        case UpgradeRarity::Epic:   borderColor = QStringLiteral("#b45fef"); break;
        }
        card->setStyleSheet(QStringLiteral(
            "QFrame { border: 2px solid %1; border-radius: 10px; background: rgba(20,25,35,0.85); }"
            "QFrame:hover { border: 2px solid #6ea8ff; background: rgba(25,32,42,0.92); }"
            "QFrame[selected=\"true\"] { border: 3px solid #82afff; background: rgba(30,40,55,0.95); }"
        ).arg(borderColor));

        card->installEventFilter(this);
        m_cards.append(card);
        cardsLayout->addWidget(card);
    }

    refreshCardStyles();
}

void UpgradePage::refreshCardStyles()
{
    for (int i = 0; i < m_cards.size(); ++i) {
        QFrame *card = m_cards[i];
        bool selected = (i == m_selectedOptionIndex);
        card->setProperty("selected", selected);
        card->style()->unpolish(card);
        card->style()->polish(card);
        card->update();
    }
}

void UpgradePage::selectCard(int index)
{
    if (index < 0 || index >= m_upgradeOptions.size()) return;
    m_selectedOptionIndex = index;
    m_selectedOptionId = m_upgradeOptions[index].optionId;
    refreshCardStyles();
    updatePreviewPanel();
    m_confirmButton->setEnabled(true);
    m_confirmButton->setText(QStringLiteral("确认选择并返回战斗"));
}

void UpgradePage::rebuildPreviewPanel()
{
    if (m_ownedTraitsLabel != nullptr) {
        m_ownedTraitsLabel->setText(traitListText(m_previewContext.ownedTraits));
    }

    while (m_attributeLayout != nullptr && m_attributeLayout->count() > 0) {
        QLayoutItem *item = m_attributeLayout->takeAt(0);
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    if (m_selectedOptionIndex < 0 || m_selectedOptionIndex >= m_upgradeOptions.size()) {
        if (m_previewTitleLabel != nullptr) {
            m_previewTitleLabel->setText(QStringLiteral("等待选择"));
        }
        if (m_previewNameLabel != nullptr) {
            m_previewNameLabel->setText(QStringLiteral("请选择左侧候选项"));
        }
        if (m_previewDetailLabel != nullptr) {
            m_previewDetailLabel->setText(QStringLiteral("未选择时无法返回战斗。选中后右侧会实时显示属性变化。"));
        }
        if (m_attributeLayout != nullptr) {
            auto *placeholder = new QLabel(QStringLiteral("暂未选择升级项。"), this);
            placeholder->setObjectName(QStringLiteral("attributeRowLabel"));
            placeholder->setWordWrap(true);
            m_attributeLayout->addWidget(placeholder);
        }
        return;
    }

    const UpgradeOption option = m_upgradeOptions.at(m_selectedOptionIndex);
    if (m_previewTitleLabel != nullptr) {
        m_previewTitleLabel->setText(QStringLiteral("已锁定候选项，等待确认"));
    }
    if (m_previewNameLabel != nullptr) {
        m_previewNameLabel->setText(option.displayName);
    }
    if (m_previewDetailLabel != nullptr) {
        m_previewDetailLabel->setText(optionDescriptionText(option));
    }

    const UpgradeAttributePreviews previews = buildAttributePreviews(option);
    for (const UpgradeAttributePreview &preview : previews) {
        auto *rowLabel = new QLabel(richDeltaText(preview.label,
                                                  preview.currentValue,
                                                  preview.nextValue,
                                                  preview.deltaText,
                                                  preview.changed,
                                                  preview.positiveChange),
                                    this);
        rowLabel->setObjectName(QStringLiteral("attributeRowLabel"));
        rowLabel->setWordWrap(true);
        rowLabel->setTextFormat(Qt::RichText);
        m_attributeLayout->addWidget(rowLabel);
    }
}

void UpgradePage::rebuildOptionButtons()
{
    // 新架构中不再使用按钮列表, 委托给 buildOptionCards
    buildOptionCards();
}

void UpgradePage::refreshOptionButtonStyles()
{
    refreshCardStyles();
}

void UpgradePage::setSelectedOptionIndex(int index)
{
    selectCard(index);
}

void UpgradePage::updatePreviewPanel()
{
    if (m_ownedTraitsLabel != nullptr) {
        m_ownedTraitsLabel->setText(traitListText(m_previewContext.ownedTraits));
    }

    while (m_attributeLayout != nullptr && m_attributeLayout->count() > 0) {
        QLayoutItem *item = m_attributeLayout->takeAt(0);
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    if (m_selectedOptionIndex < 0 || m_selectedOptionIndex >= m_upgradeOptions.size()) {
        if (m_previewTitleLabel != nullptr) {
            m_previewTitleLabel->setText(QStringLiteral("等待选择"));
        }
        if (m_previewNameLabel != nullptr) {
            m_previewNameLabel->setText(QStringLiteral("请选择左侧候选项"));
        }
        if (m_previewDetailLabel != nullptr) {
            m_previewDetailLabel->setText(QStringLiteral("未选择时无法返回战斗。选中后右侧会实时显示属性变化。"));
        }
        if (m_confirmButton != nullptr) {
            m_confirmButton->setEnabled(false);
            m_confirmButton->setText(QStringLiteral("请先选择左侧升级项"));
        }

        if (m_attributeLayout != nullptr) {
            auto *placeholder = new QLabel(QStringLiteral("暂未选择升级项。"), this);
            placeholder->setObjectName(QStringLiteral("attributeRowLabel"));
            placeholder->setWordWrap(true);
            m_attributeLayout->addWidget(placeholder);
        }
        return;
    }

    const UpgradeOption option = m_upgradeOptions.at(m_selectedOptionIndex);
    if (m_previewTitleLabel != nullptr) {
        m_previewTitleLabel->setText(QStringLiteral("已锁定候选项，等待确认"));
    }
    if (m_previewNameLabel != nullptr) {
        m_previewNameLabel->setText(option.displayName);
    }
    if (m_previewDetailLabel != nullptr) {
        m_previewDetailLabel->setText(optionDescriptionText(option));
    }
    if (m_confirmButton != nullptr) {
        m_confirmButton->setEnabled(true);
        m_confirmButton->setText(QStringLiteral("确认选择并返回战斗"));
    }

    const UpgradeAttributePreviews previews = buildAttributePreviews(option);
    for (const UpgradeAttributePreview &preview : previews) {
        auto *rowLabel = new QLabel(richDeltaText(preview.label,
                                                  preview.currentValue,
                                                  preview.nextValue,
                                                  preview.deltaText,
                                                  preview.changed,
                                                  preview.positiveChange),
                                    this);
        rowLabel->setObjectName(QStringLiteral("attributeRowLabel"));
        rowLabel->setWordWrap(true);
        rowLabel->setTextFormat(Qt::RichText);
        m_attributeLayout->addWidget(rowLabel);
    }
}

UpgradeAttributePreviews UpgradePage::buildAttributePreviews(const UpgradeOption &option) const
{
    UpgradeAttributePreviews previews;

    const UpgradePreviewContext nextContext = previewContextAfterSelection(m_previewContext, option);
    const ComputedPreviewStats currentStats = computePreviewStats(m_previewContext);
    const ComputedPreviewStats nextStats = computePreviewStats(nextContext);

    previews.push_back({
        QStringLiteral("当前武器"),
        currentStats.weaponName,
        nextStats.weaponName,
        currentStats.weaponName == nextStats.weaponName ? QStringLiteral("无变化") : QStringLiteral("已切换"),
        currentStats.weaponName != nextStats.weaponName,
        true
    });
    previews.push_back({
        QStringLiteral("单发伤害"),
        QString::number(currentStats.damagePerShot, 'f', 1),
        QString::number(nextStats.damagePerShot, 'f', 1),
        formatSignedDelta(nextStats.damagePerShot - currentStats.damagePerShot, 1),
        !qFuzzyCompare(currentStats.damagePerShot, nextStats.damagePerShot),
        nextStats.damagePerShot >= currentStats.damagePerShot
    });
    previews.push_back({
        QStringLiteral("攻击频率"),
        QStringLiteral("%1 发/秒").arg(QString::number(currentStats.attackPerSecond, 'f', 2)),
        QStringLiteral("%1 发/秒").arg(QString::number(nextStats.attackPerSecond, 'f', 2)),
        formatSignedDelta(nextStats.attackPerSecond - currentStats.attackPerSecond, 2),
        !qFuzzyCompare(currentStats.attackPerSecond, nextStats.attackPerSecond),
        nextStats.attackPerSecond >= currentStats.attackPerSecond
    });
    previews.push_back({
        QStringLiteral("移动速度"),
        QString::number(currentStats.moveSpeed, 'f', 0),
        QString::number(nextStats.moveSpeed, 'f', 0),
        formatSignedDelta(nextStats.moveSpeed - currentStats.moveSpeed, 0),
        !qFuzzyCompare(currentStats.moveSpeed, nextStats.moveSpeed),
        nextStats.moveSpeed >= currentStats.moveSpeed
    });
    previews.push_back({
        QStringLiteral("防御倍率"),
        QStringLiteral("%1x").arg(QString::number(currentStats.defenseMultiplier, 'f', 2)),
        QStringLiteral("%1x").arg(QString::number(nextStats.defenseMultiplier, 'f', 2)),
        formatSignedDelta(nextStats.defenseMultiplier - currentStats.defenseMultiplier, 2, QStringLiteral("x")),
        !qFuzzyCompare(currentStats.defenseMultiplier, nextStats.defenseMultiplier),
        nextStats.defenseMultiplier >= currentStats.defenseMultiplier
    });

    if (option.kind == UpgradeOptionKind::Stat) {
        previews.push_back({
            QStringLiteral("直接效果"),
            QStringLiteral("待接入"),
            option.summary,
            QStringLiteral("查看说明"),
            true,
            true
        });
    }

    return previews;
}

QString UpgradePage::optionButtonText(const UpgradeOption &option) const
{
    QString rarityText;
    switch (option.rarity) {
    case UpgradeRarity::Common: rarityText = QStringLiteral("普通"); break;
    case UpgradeRarity::Rare:   rarityText = QStringLiteral("稀有"); break;
    case UpgradeRarity::Epic:   rarityText = QStringLiteral("史诗"); break;
    }

    QString levelText;
    if (option.level > 1) {
        levelText = QStringLiteral(" Lv.%1").arg(option.level);
    }

    return QStringLiteral("[%1][%2] %3%4\n%5")
        .arg(optionKindText(option.kind), rarityText, option.displayName, levelText, option.summary);
}

QString UpgradePage::optionKindText(UpgradeOptionKind kind) const
{
    switch (kind) {
    case UpgradeOptionKind::Trait:
        return QStringLiteral("特性");
    case UpgradeOptionKind::Weapon:
        return QStringLiteral("武器");
    case UpgradeOptionKind::Stat:
        return QStringLiteral("属性");
    }

    return QStringLiteral("未知");
}

QString UpgradePage::optionDescriptionText(const UpgradeOption &option) const
{
    QString description = QStringLiteral("类型：%1\n效果：%2")
                              .arg(optionKindText(option.kind), option.summary);
    if (option.kind == UpgradeOptionKind::Trait) {
        const TraitConfig *traitConfig = GameConfig::findTraitConfig(option.traitId);
        if (traitConfig != nullptr) {
            description.append(QStringLiteral(
                "\n数值：伤害 x%1，防御 x%2，速度 x%3")
                                   .arg(QString::number(traitConfig->damageMultiplier, 'f', 2),
                                        QString::number(traitConfig->defenseMultiplier, 'f', 2),
                                        QString::number(traitConfig->speedMultiplier, 'f', 2)));
        }
    }
    if (option.kind == UpgradeOptionKind::Stat) {
        description = QStringLiteral("类型：特殊子弹\n效果：%1")
                          .arg(option.summary);
    }
    return description;
}

int UpgradePage::findOptionIndexById(const QString &optionId) const
{
    if (optionId.isEmpty()) {
        return -1;
    }

    for (int index = 0; index < m_upgradeOptions.size(); ++index) {
        if (m_upgradeOptions.at(index).optionId == optionId) {
            return index;
        }
    }

    return -1;
}
