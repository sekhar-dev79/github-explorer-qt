#include "thememanager.h"
#include "constants.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QDebug>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager s_instance;
    return s_instance;
}

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent)
    , m_currentTheme(AppConstants::THEME_LIGHT)
{
}

void ThemeManager::applyTheme(const QString& themeName)
{
    const QString css = loadStyleSheet(themeName);

    if (css.isEmpty()) {
        qWarning() << "[ThemeManager] Failed to load stylesheet for theme:" << themeName;
        return;
    }

    // Apply globally to all QWidgets
    qApp->setStyleSheet(css);
    m_currentTheme = themeName;

    // Persist preference
    saveThemePreference(themeName);

    emit themeChanged(themeName);

    qDebug() << "[ThemeManager] Applied theme:" << themeName;
}

void ThemeManager::toggleTheme()
{
    const QString next = (m_currentTheme == AppConstants::THEME_DARK)
    ? AppConstants::THEME_LIGHT
    : AppConstants::THEME_DARK;
    applyTheme(next);
}

void ThemeManager::loadSavedTheme()
{
    const QString saved = loadThemePreference();
    applyTheme(saved);
}

QString ThemeManager::currentTheme() const
{
    return m_currentTheme;
}

bool ThemeManager::isDarkTheme() const
{
    return m_currentTheme == AppConstants::THEME_DARK;
}

QString ThemeManager::loadStyleSheet(const QString& themeName) const
{
    const QString path = QStringLiteral(":/themes/%1.qss").arg(themeName);
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[ThemeManager] Cannot open resource file:" << path;
        return {};
    }

    QTextStream stream(&file);
    return stream.readAll();
}

void ThemeManager::saveThemePreference(const QString& themeName) const
{
    QSettings settings(AppConstants::ORG_NAME, AppConstants::APP_NAME);
    settings.setValue(AppConstants::SETTINGS_THEME_KEY, themeName);
}

QString ThemeManager::loadThemePreference() const
{
    QSettings settings(AppConstants::ORG_NAME, AppConstants::APP_NAME);
    return settings.value(AppConstants::SETTINGS_THEME_KEY,
                          AppConstants::THEME_LIGHT).toString();
}
