#pragma once

#include <QTimer>
#include <QWidget>

class DashCooldownWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit DashCooldownWidget(QWidget *parent = nullptr);
    ~DashCooldownWidget() override = default;

    [[nodiscard]] QSize sizeHint() const override;
    [[nodiscard]] QSize minimumSizeHint() const override;

public slots:
    void updateCooldown(float remainingSeconds, float totalSeconds);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    float m_remainingSeconds{0.0F};
    float m_totalSeconds{3.0F};
    bool m_isReady{true};
    QTimer *m_flashTimer{nullptr};
    int m_flashCount{0};
    bool m_flashVisible{true};
};