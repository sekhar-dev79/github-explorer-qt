#pragma once
#include <QObject>
#include <QString>

// Manages theme loading, application, and persistence
// Loads QSS files, applies globally via QApplication::setStyleSheet(),
// persists preference via QSettings, and emits themeChanged() signal
class ThemeManager : public QObject
{
    Q_OBJECT
public:
    static ThemeManager& instance();

    // Apply a named theme ("light" or "dark")
    void applyTheme(const QString& themeName);

    // Toggle between light and dark
    void toggleTheme();

    // Restore last saved theme from QSettings (call once at startup)
    void loadSavedTheme();

    // Returns "light" or "dark"
    [[nodiscard]] QString currentTheme() const;

    // Returns true if current theme is dark
    [[nodiscard]] bool isDarkTheme() const;

signals:
    // Emitted after stylesheet is applied; widgets update icons/assets here
    void themeChanged(const QString& themeName);

private:
    explicit ThemeManager(QObject* parent = nullptr);

    ThemeManager(const ThemeManager&)            = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    // Load QSS content from Qt resources
    [[nodiscard]] QString loadStyleSheet(const QString& themeName) const;

    void saveThemePreference(const QString& themeName) const;
    [[nodiscard]] QString loadThemePreference() const;

    QString m_currentTheme;
};
