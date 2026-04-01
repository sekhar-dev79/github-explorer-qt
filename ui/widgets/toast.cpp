#include "toast.h"
#include "thememanager.h"
#include <QPainter>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QFrame>

Toast::Toast(QWidget* parent, const QString& message, const QString& iconName)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_DeleteOnClose);

    auto* containerLayout = new QVBoxLayout(this);
    containerLayout->setContentsMargins(10, 10, 10, 10);

    auto* toastFrame = new QFrame(this);
    toastFrame->setObjectName("toastWidget");

    // Premium Drop Shadow
    auto* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 60));
    toastFrame->setGraphicsEffect(shadow);

    auto* layout = new QHBoxLayout(toastFrame);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(10);

    // Icon
    m_iconLabel = new QLabel(toastFrame);
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#2f81f7") : QColor("#0969da"));
    painter.end();
    m_iconLabel->setPixmap(pixmap.scaled(18, 18, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // Text
    m_textLabel = new QLabel(message, toastFrame);
    m_textLabel->setObjectName("toastText");

    layout->addWidget(m_iconLabel);
    layout->addWidget(m_textLabel);
    containerLayout->addWidget(toastFrame);

    // Opacity Effect for Fade Animation
    auto* opacityEffect = new QGraphicsOpacityEffect(this);
    opacityEffect->setOpacity(0.0);
    setGraphicsEffect(opacityEffect);

    // Position at bottom center
    adjustSize();
    int x = (parent->width() - width()) / 2;
    int y = parent->height() - height() - 30;
    move(x, y);

    m_opacityAnimation = new QPropertyAnimation(opacityEffect, "opacity", this);
    m_opacityAnimation->setDuration(250);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this, opacityEffect]() {
        m_opacityAnimation->setStartValue(1.0);
        m_opacityAnimation->setEndValue(0.0);
        connect(m_opacityAnimation, &QPropertyAnimation::finished, this, &QWidget::close);
        m_opacityAnimation->start();
    });
}

void Toast::startAnimation()
{
    QWidget::show();
    m_opacityAnimation->setStartValue(0.0);
    m_opacityAnimation->setEndValue(1.0);
    m_opacityAnimation->start();
    m_timer->start(2500);
}

void Toast::show(QWidget* parent, const QString& message, const QString& iconName)
{
    if (!parent) return;
    auto* toast = new Toast(parent, message, iconName);
    toast->startAnimation();
}
