#include "upgrade_page.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

UpgradePage::UpgradePage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto *titleLabel = new QLabel(QStringLiteral("升级页占位"), this);
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    m_optionLayout = new QVBoxLayout();
    m_optionLayout->setSpacing(8);
    layout->addLayout(m_optionLayout);

    auto *resumeButton = new QPushButton(QStringLiteral("稍后再选，返回战斗页"), this);
    connect(resumeButton, &QPushButton::clicked, this, &UpgradePage::resumeRequested);
    layout->addWidget(resumeButton);
    layout->addStretch();

    setUpgradeOptions(GameConfig::kUpgradeOptions);
}

void UpgradePage::setUpgradeOptions(const UpgradeOptions &options)
{
    m_upgradeOptions = options;
    rebuildOptionButtons();
}

const UpgradeOptions &UpgradePage::upgradeOptions() const noexcept
{
    return m_upgradeOptions;
}

void UpgradePage::rebuildOptionButtons()
{
    if (m_optionLayout == nullptr) {
        return;
    }

    while (QLayoutItem *item = m_optionLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    for (const auto &option : m_upgradeOptions) {
        auto *button = new QPushButton(
            QStringLiteral("%1 - %2").arg(option.displayName, option.summary),
            this);
        connect(button, &QPushButton::clicked, this, [this, option]() {
            emit upgradeOptionSelected(option);
            if (option.kind == UpgradeOptionKind::Trait) {
                emit traitSelected(option.traitId);
            }
            emit resumeRequested();
        });
        m_optionLayout->addWidget(button);
    }
}
