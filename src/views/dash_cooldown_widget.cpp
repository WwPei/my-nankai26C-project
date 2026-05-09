#include "dash_cooldown_widget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QtMath>

DashCooldownWidget::DashCooldownWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(52, 52);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    m_flashTimer = new QTimer(this);
    m_flashTimer->setInterval(167);
    connect(m_flashTimer, &QTimer::timeout, this, [this]() {
        m_flashVisible = !m_flashVisible;
        ++m_flashCount;
        if (m_flashCount >= 6) {
            m_flashTimer->stop();
            m_flashVisible = true;
        }
        update();
    });
}

QSize DashCooldownWidget::sizeHint() const
{
    return QSize(52, 52);
}

QSize DashCooldownWidget::minimumSizeHint() const
{
    return QSize(52, 52);
}

void DashCooldownWidget::updateCooldown(float remainingSeconds, float totalSeconds)
{
    const bool wasReady = m_isReady;

    m_remainingSeconds = remainingSeconds;
    m_totalSeconds = totalSeconds;
    m_isReady = (remainingSeconds <= 0.0F);

    if (!wasReady && m_isReady) {
        m_flashCount = 0;
        m_flashVisible = true;
        m_flashTimer->start();
    }

    update();
}

void DashCooldownWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), Qt::transparent);

    const int centerX = width() / 2;
    const int centerY = height() / 2;
    const int radius = 20;
    const QRect circleRect(centerX - radius, centerY - radius, radius * 2, radius * 2);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(30, 30, 40, 180)));
    painter.drawPie(circleRect, 0, 5760);

    if (!m_isReady) {
        const int angleSpan = static_cast<int>(360.0F * (m_remainingSeconds / m_totalSeconds) * 16);

        QColor fillColor;
        if (m_remainingSeconds < 0.5F) {
            fillColor = QColor(255, 200, 50, 200);
        } else {
            fillColor = QColor(100, 100, 120, 180);
        }

        painter.setBrush(QBrush(fillColor));
        painter.drawPie(circleRect, 90 * 16, angleSpan);

        QFont emojiFont(QStringLiteral("Segoe UI Emoji"), 20);
        painter.setFont(emojiFont);
        painter.setPen(QColor(120, 120, 140));
        painter.drawText(circleRect, Qt::AlignCenter, QStringLiteral("\u26A1"));

        QFont numberFont(QStringLiteral("Microsoft YaHei UI"), 9);
        painter.setFont(numberFont);
        painter.setPen(Qt::white);
        const QString secondsText = QString::number(m_remainingSeconds, 'f', 1);
        const QRect numberRect(centerX - radius, centerY + 4, radius * 2, radius * 2);
        painter.drawText(numberRect, Qt::AlignHCenter | Qt::AlignTop, secondsText);
    } else {
        if (m_flashTimer->isActive() && !m_flashVisible) {
        } else {
            painter.setBrush(QBrush(QColor(60, 120, 255, 200)));
            painter.drawPie(circleRect, 0, 5760);
        }

        QFont emojiFont(QStringLiteral("Segoe UI Emoji"), 20);
        painter.setFont(emojiFont);
        painter.setPen(Qt::white);
        painter.drawText(circleRect, Qt::AlignCenter, QStringLiteral("\u26A1"));
    }
}