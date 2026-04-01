#pragma once
#include <QWidget>
#include <QPixmap>
#include <QString>

// Circular avatar with async image download
// Shows placeholder (first letter + color) while downloading; displays circular avatar when loaded
// All drawing via paintEvent() with QPainter clipping (not QSS border-radius)
class AvatarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AvatarWidget(int size = 60, QWidget* parent = nullptr);

    // Load new avatar and start download (resets to placeholder)
    void loadAvatar(const QString& avatarUrl, const QString& username);

    // Reset to blank placeholder
    void reset();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void drawPlaceholder(QPainter& p) const;
    void drawAvatar(QPainter& p) const;

    // Generate deterministic background color from username
    [[nodiscard]] static QColor usernameToColor(const QString& username);

    QPixmap m_pixmap;       // loaded avatar (empty = use placeholder)
    QString m_initial;      // first character of username, uppercase
    QColor  m_bgColor;      // placeholder background color
    int     m_size;
    bool    m_loaded = false;
};
