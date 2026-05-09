#pragma once

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QList>
#include <QVector>
#include <QVBoxLayout>

#include "game_data.h"

class UpgradePage : public QWidget
{
    Q_OBJECT

public:
    explicit UpgradePage(QWidget *parent = nullptr);
    ~UpgradePage() override = default;

    // --- 向后兼容的旧 API (emoji_dungeon_window.cpp 仍在使用) ---
    void setUpgradeOptions(const UpgradeOptions &options);
    void setPreviewContext(const UpgradePreviewContext &context);
    [[nodiscard]] const UpgradeOptions &upgradeOptions() const noexcept;

    // --- 新统一 API ---
    void loadOptions(const UpgradeOptions &options, const UpgradePreviewContext &context);
    [[nodiscard]] QString selectedOptionId() const { return m_selectedOptionId; }

signals:
    // 旧信号 (向后兼容)
    void upgradeOptionSelected(UpgradeOption option);
    void traitSelected(TraitId traitId);
    void resumeRequested();

    // 新信号 (无参版用 confirmed 替代, 避免与旧版 upgradeOptionSelected(UpgradeOption) 重载歧义)
    void weaponUpgradeSelected(WeaponUpgradeId weaponUpgradeId);
    void bulletStyleSelected(BulletStyle style);
    void confirmed();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    // --- 旧辅助 ---
    [[nodiscard]] int findOptionIndexById(const QString &optionId) const;
    void rebuildOptionButtons();
    void refreshOptionButtonStyles();
    void setSelectedOptionIndex(int index);
    void updatePreviewPanel();
    [[nodiscard]] UpgradeAttributePreviews buildAttributePreviews(const UpgradeOption &option) const;
    [[nodiscard]] QString optionButtonText(const UpgradeOption &option) const;
    [[nodiscard]] QString optionKindText(UpgradeOptionKind kind) const;
    [[nodiscard]] QString optionDescriptionText(const UpgradeOption &option) const;

    // --- 新卡片 UI 辅助 ---
    void buildOptionCards();
    void refreshCardStyles();
    void selectCard(int index);
    void rebuildPreviewPanel();

    // --- 数据 ---
    UpgradeOptions m_upgradeOptions;
    UpgradePreviewContext m_previewContext;
    int m_selectedOptionIndex{-1};
    QString m_selectedOptionId;

    // --- 左侧：新卡片容器 ---
    QFrame *m_cardsContainer{nullptr};
    QList<QFrame *> m_cards;

    // --- 左侧：旧按钮 (向后兼容) ---
    QVBoxLayout *m_optionLayout{nullptr};
    QVector<QPushButton *> m_optionButtons;

    // --- 右侧预览面板 ---
    QLabel *m_hintLabel{nullptr};
    QLabel *m_previewTitleLabel{nullptr};
    QLabel *m_previewNameLabel{nullptr};
    QLabel *m_previewDetailLabel{nullptr};
    QLabel *m_ownedTraitsLabel{nullptr};
    QVBoxLayout *m_attributeLayout{nullptr};
    QPushButton *m_confirmButton{nullptr};
};
