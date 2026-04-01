#include "appsettings.h"
#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

QSettings AppSettings::makeSettings()
{
    return QSettings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());
}

// Theme
void AppSettings::saveTheme(const QString& theme)
{
    auto s = makeSettings();
    s.setValue(KEY_THEME, theme);
}

QString AppSettings::loadTheme()
{
    auto s = makeSettings();
    return s.value(KEY_THEME, DEFAULT_THEME).toString();
}

// Auth token
void AppSettings::saveAuthToken(const QString& token)
{
    auto s = makeSettings();
    s.setValue(KEY_AUTH_TOKEN, token);
}

QString AppSettings::loadAuthToken()
{
    auto s = makeSettings();
    return s.value(KEY_AUTH_TOKEN, QString{}).toString();
}

void AppSettings::clearAuthToken()
{
    auto s = makeSettings();
    s.remove(KEY_AUTH_TOKEN);
}

// Window geometry
void AppSettings::saveWindowGeometry(const QByteArray& geo)
{
    auto s = makeSettings();
    s.setValue(KEY_WINDOW_GEO, geo);
}

QByteArray AppSettings::loadWindowGeometry()
{
    auto s = makeSettings();
    return s.value(KEY_WINDOW_GEO, QByteArray{}).toByteArray();
}

void AppSettings::saveWindowState(const QByteArray& state)
{
    auto s = makeSettings();
    s.setValue(KEY_WINDOW_STATE, state);
}

QByteArray AppSettings::loadWindowState()
{
    auto s = makeSettings();
    return s.value(KEY_WINDOW_STATE, QByteArray{}).toByteArray();
}

// Search history
void AppSettings::saveSearchHistory(const QStringList& history)
{
    auto s = makeSettings();
    s.setValue(KEY_SEARCH_HISTORY, history.mid(0, MAX_HISTORY));
}

QStringList AppSettings::loadSearchHistory()
{
    auto s = makeSettings();
    return s.value(KEY_SEARCH_HISTORY, QStringList{}).toStringList();
}

void AppSettings::addToSearchHistory(const QString& query)
{
    if (query.trimmed().isEmpty()) return;

    QStringList history = loadSearchHistory();

    history.removeAll(query);
    history.prepend(query);

    saveSearchHistory(history);
}

// Bookmarks
bool AppSettings::isBookmarked(int repoId)
{
    auto s = makeSettings();
    QJsonArray arr = QJsonDocument::fromJson(s.value(KEY_BOOKMARKS).toByteArray()).array();
    for (const QJsonValue& val : arr) {
        if (val.toObject()["id"].toInt() == repoId) {
            return true;
        }
    }
    return false;
}

void AppSettings::saveBookmark(const Repository& repo, bool bookmarked)
{
    auto s = makeSettings();
    QJsonArray arr = QJsonDocument::fromJson(s.value(KEY_BOOKMARKS).toByteArray()).array();
    QJsonArray newArr;

    for (const QJsonValue& val : arr) {
        if (val.toObject()["id"].toInt() != repo.id) {
            newArr.append(val);
        }
    }

    // Add it back to the end if we are bookmarking it
    if (bookmarked) {
        QJsonObject obj;
        obj["id"] = repo.id;
        obj["name"] = repo.name;
        obj["fullName"] = repo.fullName;
        obj["description"] = repo.description;
        obj["language"] = repo.language;
        obj["stargazers"] = repo.stargazers;
        obj["forks"] = repo.forks;
        obj["updatedAt"] = repo.updatedAt;
        obj["isPrivate"] = repo.isPrivate;
        newArr.append(obj);
    }

    s.setValue(KEY_BOOKMARKS, QJsonDocument(newArr).toJson(QJsonDocument::Compact));
}

QList<Repository> AppSettings::loadBookmarks()
{
    auto s = makeSettings();
    QList<Repository> list;
    QJsonArray arr = QJsonDocument::fromJson(s.value(KEY_BOOKMARKS).toByteArray()).array();

    for (const QJsonValue& val : arr) {
        QJsonObject obj = val.toObject();
        Repository repo;
        repo.id = obj["id"].toInt();
        repo.name = obj["name"].toString();
        repo.fullName = obj["fullName"].toString();
        repo.description = obj["description"].toString();
        repo.language = obj["language"].toString();
        repo.stargazers = obj["stargazers"].toInt();
        repo.forks = obj["forks"].toInt();
        repo.updatedAt = obj["updatedAt"].toString();
        repo.isPrivate = obj["isPrivate"].toBool();
        list.append(repo);
    }
    return list;
}
