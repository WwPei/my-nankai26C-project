#pragma once

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QHash>
#include <QColor>
#include <QStringList>

#include "game_data.h"

class ClassSelectPage : public QWidget
{
    Q_OBJECT

public:
    explicit ClassSelectPage(QWidget *parent = nullptr);
    ~ClassSelectPage() override = default;

signals:
    void classSelected(PlayerClassId classId);
    void backRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUi();
    void setupStyle();
    void setupConnections();
    void selectCard(PlayerClassId id);
    void animateCardSelection(QFrame *card);
    void refreshCardStyle(QFrame *card, bool selected);

    QList<QFrame *> m_cards;
    PlayerClassId m_selectedId{static_cast<PlayerClassId>(-1)};
    QPushButton *m_confirmButton{nullptr};
    QPushButton *m_backButton{nullptr};
    QLabel *m_background{nullptr};
    QLabel *m_titleLabel{nullptr};

    QHash<QFrame *, QSize> m_cardOriginalMinSizes;
    QHash<QFrame *, QSize> m_cardOriginalMaxSizes;

    static QHash<PlayerClassId, QStringList> classFeatures();
    static QHash<PlayerClassId, QColor> classAccentColors();
    static QHash<PlayerClassId, QString> classIcons();
};
