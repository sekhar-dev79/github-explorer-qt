#pragma once
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QList>
#include "repository.h"

class AppSettings
{
public:
    // Settings keys
    static constexpr char KEY_THEME[]          = "ui/theme";
    static constexpr char KEY_AUTH_TOKEN[]     = "auth/token";
    static constexpr char KEY_WINDOW_GEO[]     = "window/geometry";
    static constexpr char KEY_WINDOW_STATE[]   = "window/state";
    static constexpr char KEY_SEARCH_HISTORY[] = "search/history";
    static constexpr char KEY_BOOKMARKS[]      = "data/bookmarks";

    static constexpr char DEFAULT_THEME[] = "dark";
    static constexpr int  MAX_HISTORY     = 20;

    // Theme
    static void    saveTheme(const QString& theme);
    static QString loadTheme();

    // Auth token
    static void    saveAuthToken(const QString& token);
    static QString loadAuthToken();
    static void    clearAuthToken();

    // Window geometry
    static void       saveWindowGeometry(const QByteArray& geo);
    static QByteArray loadWindowGeometry();
    static void       saveWindowState(const QByteArray& state);
    static QByteArray loadWindowState();

    // Search history
    static void        saveSearchHistory(const QStringList& history);
    static QStringList loadSearchHistory();
    static void        addToSearchHistory(const QString& query);

    // Bookmarks
    static bool isBookmarked(int repoId);
    static void saveBookmark(const Repository& repo, bool bookmarked);
    static QList<Repository> loadBookmarks();

private:
    AppSettings() = delete;
    static QSettings makeSettings();
};
