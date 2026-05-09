#include "class_select_page.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

ClassSelectPage::ClassSelectPage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto *titleLabel = new QLabel(QStringLiteral("职业选择页占位"), this);
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);

    for (const auto &config : GameConfig::kPlayerClasses) {
        auto *button = new QPushButton(
            QStringLiteral("%1 - %2").arg(config.displayName, config.summary),
            this);
        connect(button, &QPushButton::clicked, this, [this, classId = config.id]() {
            emit classSelected(classId);
        });
        layout->addWidget(button);
    }

    auto *backButton = new QPushButton(QStringLiteral("返回开始页"), this);
    connect(backButton, &QPushButton::clicked, this, &ClassSelectPage::backRequested);
    layout->addWidget(backButton);
    layout->addStretch();
}
