#include "avatarwidget.h"
#include "apiclient.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QFont>

AvatarWidget::AvatarWidget(int size, QWidget* parent)
    : QWidget(parent)
    , m_size(size)
{
    setFixedSize(m_size, m_size);
    setAttribute(Qt::WA_TranslucentBackground, false);
}

void AvatarWidget::loadAvatar(const QString& avatarUrl, const QString& username)
{
    // Show placeholder immediately while downloading
    m_loaded  = false;
    m_pixmap  = QPixmap();
    m_initial = username.isEmpty() ? QChar('?') : username.at(0).toUpper();
    m_bgColor = usernameToColor(username);
    update();

    if (avatarUrl.isEmpty()) return;

    // Download avatar asynchronously
    ApiClient::instance().downloadPixmap(avatarUrl,
                                         [this](const QPixmap& pixmap) {
                                             if (!pixmap.isNull()) {
                                                 m_pixmap = pixmap.scaled(m_size, m_size,
                                                                          Qt::IgnoreAspectRatio,
                                                                          Qt::SmoothTransformation);
                                                 m_loaded = true;
                                             }
                                             update();
                                         });
}

void AvatarWidget::reset()
{
    m_loaded  = false;
    m_pixmap  = QPixmap();
    m_initial = "";
    update();
}

void AvatarWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // Clip to circular path
    QPainterPath clipPath;
    clipPath.addEllipse(QRectF(0, 0, m_size, m_size));
    p.setClipPath(clipPath);

    if (m_loaded && !m_pixmap.isNull())
        drawAvatar(p);
    else
        drawPlaceholder(p);
}

void AvatarWidget::drawAvatar(QPainter& p) const
{
    p.drawPixmap(0, 0, m_pixmap);
}

void AvatarWidget::drawPlaceholder(QPainter& p) const
{
    // Fill circle with username-derived color
    p.fillRect(0, 0, m_size, m_size, m_bgColor);

    if (m_initial.isEmpty()) return;

    // Draw initial letter centered
    p.setPen(Qt::white);
    QFont font = p.font();
    font.setPixelSize(m_size / 2);
    font.setWeight(QFont::DemiBold);
    p.setFont(font);
    p.drawText(QRect(0, 0, m_size, m_size),
               Qt::AlignCenter, m_initial);
}

// Maps username to one of 8 deterministic colors; same username always produces same color
QColor AvatarWidget::usernameToColor(const QString& username)
{
    static const QColor palette[] = {
        { "#2563eb" },    // blue
        { "#16a34a" },    // green
        { "#9333ea" },    // purple
        { "#ea580c" },    // orange
        { "#0891b2" },    // cyan
        { "#be185d" },    // pink
        { "#ca8a04" },    // amber
        { "#475569" },    // slate
    };
    constexpr int PALETTE_SIZE = sizeof(palette) / sizeof(palette[0]);

    if (username.isEmpty()) return palette[0];

    int checksum = 0;
    for (const QChar& ch : username)
        checksum += ch.unicode();

    return palette[checksum % PALETTE_SIZE];
}
