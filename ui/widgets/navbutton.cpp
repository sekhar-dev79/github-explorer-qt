#include "navbutton.h"
#include "thememanager.h"
#include <QHBoxLayout>
#include <QPixmap>

NavButton::NavButton(const QString& iconName,
                     const QString& text,
                     QWidget* parent)
    : QPushButton(parent), m_iconName(iconName)
{
    setObjectName("navButton");
    setCheckable(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(32);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    buildLayout(text);
    updateIconColor();
    connect(this, &QPushButton::toggled, this, &NavButton::updateIconColor);
}

void NavButton::buildLayout(const QString& text)
{
    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName("navButtonIcon");
    m_iconLabel->setFixedWidth(24);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_textLabel = new QLabel(text, this);
    m_textLabel->setObjectName("navButtonText");
    m_textLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->setSpacing(8);
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_textLabel);
    layout->addStretch();
}

void NavButton::updateIconColor()
{
    if (m_iconName.isEmpty() || !m_iconLabel) return;

    QPixmap pixmap(QString(":/icons/%1.svg").arg(m_iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);

    QColor tintColor;
    const bool isDark = ThemeManager::instance().isDarkTheme();

    if (isChecked()) {
        tintColor = isDark ? QColor("#2f81f7") : QColor("#0969da");
    } else if (underMouse()) {
        tintColor = isDark ? QColor("#c9d1d9") : QColor("#24292f");
    } else {
        tintColor = isDark ? QColor("#8b949e") : QColor("#57606a");
    }

    painter.fillRect(pixmap.rect(), tintColor);
    painter.end();

    m_iconLabel->setPixmap(pixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void NavButton::enterEvent(QEnterEvent* event)
{
    QPushButton::enterEvent(event);
    updateIconColor();
}

void NavButton::leaveEvent(QEvent* event)
{
    QPushButton::leaveEvent(event);
    updateIconColor();
}

void NavButton::paintEvent(QPaintEvent* event)
{
    QPushButton::paintEvent(event);
    if (!isChecked()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);

    const QColor accent = ThemeManager::instance().isDarkTheme()
                              ? QColor("#2f81f7")
                              : QColor("#0969da");
    p.setBrush(accent);

    const int barW = 3;
    const int barH = height() * 0.6;
    const int barY = (height() - barH) / 2;
    p.drawRoundedRect(0, barY, barW, barH, 1.5, 1.5);
}

void NavButton::setNavText(const QString& text)
{
    if (m_textLabel) m_textLabel->setText(text);
}

QString NavButton::navText() const
{
    return m_textLabel ? m_textLabel->text() : QString{};
}
