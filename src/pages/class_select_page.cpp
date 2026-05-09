#include "class_select_page.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPauseAnimation>
#include <QEasingCurve>
#include <QStyle>

QHash<PlayerClassId, QStringList> ClassSelectPage::classFeatures()
{
    return {
        {PlayerClassId::Warrior, {
            QStringLiteral("近中距离战斗"),
            QStringLiteral("生命值较高"),
            QStringLiteral("攻防兼备，稳健成长")
        }},
        {PlayerClassId::Ranger, {
            QStringLiteral("远距离输出"),
            QStringLiteral("高机动性走位"),
            QStringLiteral("持续伤害，风筝流派")
        }},
        {PlayerClassId::Caster, {
            QStringLiteral("远程高爆发"),
            QStringLiteral("法术轨道体系"),
            QStringLiteral("慢速但伤害惊人")
        }}
    };
}

QHash<PlayerClassId, QColor> ClassSelectPage::classAccentColors()
{
    return {
        {PlayerClassId::Warrior, QColor(QStringLiteral("#7B68EE"))},
        {PlayerClassId::Ranger,  QColor(QStringLiteral("#FFD700"))},
        {PlayerClassId::Caster,  QColor(QStringLiteral("#4169E1"))}
    };
}

QHash<PlayerClassId, QString> ClassSelectPage::classIcons()
{
    return {
        {PlayerClassId::Warrior, QStringLiteral("\u2694\uFE0F")},
        {PlayerClassId::Ranger,  QStringLiteral("\U0001F3F9")},
        {PlayerClassId::Caster,  QStringLiteral("\U0001F52E")}
    };
}

ClassSelectPage::ClassSelectPage(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    setupStyle();
    setupConnections();
}

void ClassSelectPage::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_background = new QLabel(this);
    m_background->setPixmap(QPixmap(QStringLiteral(":/images/class_bg.png")));
    m_background->setScaledContents(true);
    m_background->setGeometry(rect());
    m_background->lower();

    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setContentsMargins(40, 30, 40, 30);
    contentLayout->setSpacing(0);

    contentLayout->addStretch(1);

    m_titleLabel = new QLabel(QStringLiteral("选择你的职业"), this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setObjectName(QStringLiteral("classSelectTitle"));
    QGraphicsDropShadowEffect *titleGlow = new QGraphicsDropShadowEffect(this);
    titleGlow->setBlurRadius(25.0);
    titleGlow->setOffset(0, 0);
    titleGlow->setColor(QColor(255, 255, 255, 80));
    m_titleLabel->setGraphicsEffect(titleGlow);
    contentLayout->addWidget(m_titleLabel);

    contentLayout->addSpacing(30);

    QHBoxLayout *cardsLayout = new QHBoxLayout();
    cardsLayout->setContentsMargins(0, 0, 0, 0);
    cardsLayout->setSpacing(20);

    cardsLayout->addStretch(2);

    const QList<PlayerClassId> classIds = {
        PlayerClassId::Warrior,
        PlayerClassId::Ranger,
        PlayerClassId::Caster
    };

    const QHash<PlayerClassId, QColor> colors = classAccentColors();
    const QHash<PlayerClassId, QString> icons = classIcons();
    const QHash<PlayerClassId, QStringList> features = classFeatures();

    const QSize cardMinSize(220, 320);
    const QSize cardMaxSize(280, 400);

    for (const auto &config : GameConfig::kPlayerClasses) {
        QFrame *card = new QFrame(this);
        card->setObjectName(QStringLiteral("classCard"));
        card->setProperty("classId", static_cast<int>(config.id));
        card->setMinimumSize(cardMinSize);
        card->setMaximumSize(cardMaxSize);
        card->setCursor(Qt::PointingHandCursor);
        card->installEventFilter(this);

        QColor accentColor = colors.value(config.id, QColor(Qt::white));
        QString borderColor = accentColor.name();
        card->setStyleSheet(
            QStringLiteral(
                "QFrame#classCard {"
                "  background-color: rgba(20, 25, 35, 0.85);"
                "  border: 2px solid %1;"
                "  border-radius: 12px;"
                "}"
                "QFrame#classCard[selected=\"true\"] {"
                "  border: 3px solid %1;"
                "  background-color: rgba(30, 35, 50, 0.92);"
                "}"
            ).arg(borderColor)
        );

        QVBoxLayout *cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(16, 20, 16, 20);
        cardLayout->setSpacing(8);

        QLabel *iconLabel = new QLabel(icons.value(config.id), card);
        iconLabel->setAlignment(Qt::AlignCenter);
        QFont iconFont = iconLabel->font();
        iconFont.setPointSize(36);
        iconLabel->setFont(iconFont);
        cardLayout->addWidget(iconLabel);

        QLabel *nameLabel = new QLabel(config.displayName, card);
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setObjectName(QStringLiteral("className"));
        QFont nameFont = nameLabel->font();
        nameFont.setPointSize(16);
        nameFont.setBold(true);
        nameLabel->setFont(nameFont);
        nameLabel->setStyleSheet(QStringLiteral("color: white;"));
        cardLayout->addWidget(nameLabel);

        QFrame *separator = new QFrame(card);
        separator->setFrameShape(QFrame::HLine);
        separator->setFixedHeight(2);
        separator->setStyleSheet(
            QStringLiteral("background-color: %1; border: none;").arg(borderColor)
        );
        cardLayout->addWidget(separator);

        cardLayout->addSpacing(4);

        const QStringList featureList = features.value(config.id);
        for (const QString &feat : featureList) {
            QLabel *featLabel = new QLabel(feat, card);
            featLabel->setAlignment(Qt::AlignCenter);
            featLabel->setStyleSheet(QStringLiteral("color: #aab0c0; font-size: 12px;"));
            cardLayout->addWidget(featLabel);
        }

        cardLayout->addStretch();

        m_cardOriginalMinSizes[card] = cardMinSize;
        m_cardOriginalMaxSizes[card] = cardMaxSize;
        m_cards.append(card);

        cardsLayout->addWidget(card, 1, Qt::AlignVCenter);
    }

    cardsLayout->addStretch(2);

    contentLayout->addLayout(cardsLayout);

    contentLayout->addSpacing(30);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(16);

    buttonLayout->addStretch(1);

    m_confirmButton = new QPushButton(QStringLiteral("确认选择"), this);
    m_confirmButton->setObjectName(QStringLiteral("confirmButton"));
    m_confirmButton->setEnabled(false);
    m_confirmButton->setMinimumSize(140, 44);
    buttonLayout->addWidget(m_confirmButton);

    m_backButton = new QPushButton(QStringLiteral("返回"), this);
    m_backButton->setObjectName(QStringLiteral("backButton"));
    m_backButton->setMinimumSize(120, 44);
    buttonLayout->addWidget(m_backButton);

    buttonLayout->addStretch(1);

    contentLayout->addLayout(buttonLayout);

    contentLayout->addStretch(1);

    mainLayout->addLayout(contentLayout);
}

void ClassSelectPage::setupStyle()
{
    setStyleSheet(QStringLiteral(
        "ClassSelectPage {"
        "  background-color: #0d1117;"
        "}"
        "QLabel#classSelectTitle {"
        "  color: white;"
        "  font-size: 28px;"
        "  font-weight: bold;"
        "  padding: 8px 0px;"
        "}"
        "QPushButton#confirmButton {"
        "  background-color: rgba(30, 35, 50, 0.9);"
        "  color: #5a5f6e;"
        "  border: 2px solid #2a2f3e;"
        "  border-radius: 8px;"
        "  font-size: 15px;"
        "  font-weight: bold;"
        "  padding: 8px 24px;"
        "}"
        "QPushButton#confirmButton:enabled {"
        "  background-color: rgba(40, 50, 70, 0.9);"
        "  color: white;"
        "  border: 2px solid #5a7fef;"
        "}"
        "QPushButton#confirmButton:enabled:hover {"
        "  background-color: rgba(55, 70, 100, 0.9);"
        "  border: 2px solid #7a9fff;"
        "}"
        "QPushButton#confirmButton:enabled:pressed {"
        "  background-color: rgba(25, 35, 55, 0.9);"
        "}"
        "QPushButton#backButton {"
        "  background-color: rgba(30, 35, 50, 0.9);"
        "  color: #aab0c0;"
        "  border: 2px solid #2a2f3e;"
        "  border-radius: 8px;"
        "  font-size: 14px;"
        "  padding: 8px 20px;"
        "}"
        "QPushButton#backButton:hover {"
        "  background-color: rgba(45, 50, 65, 0.9);"
        "  border: 2px solid #4a4f5e;"
        "  color: #ccd0e0;"
        "}"
        "QPushButton#backButton:pressed {"
        "  background-color: rgba(20, 25, 35, 0.9);"
        "}"
    ));
}

void ClassSelectPage::setupConnections()
{
    QObject::connect(m_confirmButton, &QPushButton::clicked, this, [this]() {
        if (m_selectedId != static_cast<PlayerClassId>(-1)) {
            emit classSelected(m_selectedId);
        }
    });

    QObject::connect(m_backButton, &QPushButton::clicked, this, &ClassSelectPage::backRequested);
}

void ClassSelectPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (m_background) {
        m_background->setGeometry(rect());
    }
}

bool ClassSelectPage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QFrame *card = qobject_cast<QFrame *>(watched);
            if (card && m_cards.contains(card)) {
                bool ok = false;
                int classIdInt = card->property("classId").toInt(&ok);
                if (ok) {
                    selectCard(static_cast<PlayerClassId>(classIdInt));
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ClassSelectPage::selectCard(PlayerClassId id)
{
    for (QFrame *card : m_cards) {
        int cardClassId = card->property("classId").toInt();
        if (static_cast<PlayerClassId>(cardClassId) != id) {
            refreshCardStyle(card, false);
            card->setMinimumSize(m_cardOriginalMinSizes.value(card));
            card->setMaximumSize(m_cardOriginalMaxSizes.value(card));
        }
    }

    QFrame *targetCard = nullptr;
    for (QFrame *card : m_cards) {
        int cardClassId = card->property("classId").toInt();
        if (static_cast<PlayerClassId>(cardClassId) == id) {
            targetCard = card;
            break;
        }
    }

    if (!targetCard) {
        return;
    }

    refreshCardStyle(targetCard, true);
    m_selectedId = id;
    m_confirmButton->setEnabled(true);

    animateCardSelection(targetCard);
}

void ClassSelectPage::animateCardSelection(QFrame *card)
{
    QSize originalMin = m_cardOriginalMinSizes.value(card);
    QSize originalMax = m_cardOriginalMaxSizes.value(card);
    QSize enlargedMin(
        static_cast<int>(originalMin.width() * 1.05),
        static_cast<int>(originalMin.height() * 1.05)
    );
    QSize enlargedMax(
        static_cast<int>(originalMax.width() * 1.05),
        static_cast<int>(originalMax.height() * 1.05)
    );

    const int duration = 300;
    const QEasingCurve curve = QEasingCurve::OutBack;

    QPropertyAnimation *minAnim = new QPropertyAnimation(card, "minimumSize", this);
    minAnim->setDuration(duration);
    minAnim->setStartValue(originalMin);
    minAnim->setEndValue(enlargedMin);
    minAnim->setEasingCurve(curve);

    QPropertyAnimation *maxAnim = new QPropertyAnimation(card, "maximumSize", this);
    maxAnim->setDuration(duration);
    maxAnim->setStartValue(originalMax);
    maxAnim->setEndValue(enlargedMax);
    maxAnim->setEasingCurve(curve);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(minAnim);
    group->addAnimation(maxAnim);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    QObject::connect(group, &QParallelAnimationGroup::finished, this, [card, enlargedMin, enlargedMax]() {
        card->setMinimumSize(enlargedMin);
        card->setMaximumSize(enlargedMax);
    });
}

void ClassSelectPage::refreshCardStyle(QFrame *card, bool selected)
{
    card->setProperty("selected", selected);
    card->style()->unpolish(card);
    card->style()->polish(card);
    card->update();
}
