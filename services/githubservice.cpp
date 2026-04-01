#include "githubservice.h"
#include "apiclient.h"
#include "userprofile.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

GitHubService& GitHubService::instance()
{
    static GitHubService s_instance;
    return s_instance;
}

GitHubService::GitHubService(QObject* parent)
    : QObject(parent)
{}

void GitHubService::searchUser(const QString& username)
{
    if (username.trimmed().isEmpty()) return;

    // Cancel previous request to prevent race conditions
    ApiClient::instance().cancelPendingRequests();

    const QString endpoint = QString("/users/%1").arg(username.trimmed());

    m_activeRequests++;
    emit loadingChanged(true);

    qDebug() << "[GitHubService] Searching user:" << username;

    ApiClient::instance().get(endpoint, {},
                              [this, username](const ApiResponse& response)
                              {
                                  m_activeRequests = qMax(0, m_activeRequests - 1);

                                  // Ignore cancelled requests (user typed new query)
                                  if (response.isCancelled) {
                                      m_activeRequests = qMax(0, m_activeRequests - 1);
                                      if (m_activeRequests == 0) emit loadingChanged(false);
                                      return;
                                  }

                                  if (!response.success) {
                                      qWarning() << "[GitHubService] User search failed:" << response.errorMsg;
                                      emit loadingChanged(false);
                                      emit error(response.errorMsg);
                                      return;
                                  }

                                  const QJsonObject obj = response.data.object();
                                  const UserProfile profile = parseUserProfile(obj);

                                  if (profile.login.isEmpty()) {
                                      emit loadingChanged(false);
                                      emit error("Received unexpected data from GitHub. Try again.");
                                      return;
                                  }

                                  qDebug() << "[GitHubService] Found user:"
                                           << profile.login
                                           << "| repos:" << profile.publicRepos
                                           << "| followers:" << profile.followers;

                                  emit loadingChanged(false);
                                  emit userFound(profile);
                              });
}

void GitHubService::fetchRepositories(const QString& username,
                                      int page,
                                      int perPage)
{
    if (username.trimmed().isEmpty()) return;

    const QString endpoint =
        QString("/users/%1/repos").arg(username.trimmed());

    const QList<QPair<QString,QString>> params = {
        { "sort",     "pushed" },
        { "per_page", QString::number(perPage) },
        { "page",     QString::number(page) }
    };

    m_activeRequests++;
    emit loadingChanged(true);

    qDebug() << "[GitHubService] Fetching repos for:"
             << username << "page:" << page;

    ApiClient::instance().get(endpoint, params,
                              [this, username, page, perPage](const ApiResponse& response)
                              {
                                  m_activeRequests = qMax(0, m_activeRequests - 1);

                                  if (response.isCancelled) {
                                      m_activeRequests = qMax(0, m_activeRequests - 1);
                                      if (m_activeRequests == 0) emit loadingChanged(false);
                                      return;
                                  }

                                  if (!response.success) {
                                      qWarning() << "[GitHubService] Repo fetch failed:" << response.errorMsg;
                                      emit loadingChanged(false);
                                      emit error(response.errorMsg);
                                      return;
                                  }

                                  const QJsonArray arr = response.data.array();

                                  if (arr.isEmpty() && page == 1) {
                                      emit loadingChanged(false);
                                      emit reposLoaded({}, page, 0);
                                      return;
                                  }

                                  const QList<Repository> repos = parseRepositoryList(arr);

                                  qDebug() << "[GitHubService] Loaded" << repos.size()
                                           << "repos for" << username << "page" << page;

                                  emit loadingChanged(false);
                                  emit reposLoaded(repos, page, perPage);
                              });
}

void GitHubService::cancelAll()
{
    ApiClient::instance().cancelPendingRequests();
    m_activeRequests = 0;
    emit loadingChanged(false);
}

UserProfile GitHubService::parseUserProfile(const QJsonObject& obj)
{
    UserProfile p;

    p.login           = obj["login"].toString();
    p.name            = obj["name"].toString();
    p.avatarUrl       = obj["avatar_url"].toString();
    p.bio             = obj["bio"].toString();
    p.location        = obj["location"].toString();
    p.company         = obj["company"].toString().trimmed();
    p.blog            = obj["blog"].toString();
    p.htmlUrl         = obj["html_url"].toString();
    p.type            = obj["type"].toString();
    p.email           = obj["email"].toString();
    p.twitterUsername = obj["twitter_username"].toString();
    p.hireable        = obj["hireable"].toBool(false);

    p.publicRepos = obj["public_repos"].toInt();
    p.publicGists = obj["public_gists"].toInt();
    p.followers   = obj["followers"].toInt();
    p.following   = obj["following"].toInt();

    p.createdAt = QDateTime::fromString(obj["created_at"].toString(), Qt::ISODate);
    p.updatedAt = QDateTime::fromString(obj["updated_at"].toString(), Qt::ISODate);

    return p;
}

Repository GitHubService::parseRepository(const QJsonObject& obj)
{
    Repository r;

    r.id          = jsonInt   (obj, "id");
    r.name        = jsonString(obj, "name");
    r.fullName    = jsonString(obj, "full_name");
    r.description = jsonString(obj, "description");
    r.htmlUrl     = jsonString(obj, "html_url");
    r.language    = jsonString(obj, "language");

    r.stargazers  = jsonInt(obj, "stargazers_count");
    r.forks       = jsonInt(obj, "forks_count");
    r.watchers    = jsonInt(obj, "watchers_count");
    r.openIssues  = jsonInt(obj, "open_issues_count");

    r.isPrivate   = jsonBool(obj, "private");
    r.isFork      = jsonBool(obj, "fork");
    r.isArchived  = jsonBool(obj, "archived");
    r.hasIssues   = jsonBool(obj, "has_issues", true);

    r.createdAt   = jsonString(obj, "created_at");
    r.updatedAt   = jsonString(obj, "updated_at");
    r.pushedAt    = jsonString(obj, "pushed_at");

    r.isValid = !r.name.isEmpty();

    return r;
}

QList<Repository> GitHubService::parseRepositoryList(const QJsonArray& arr)
{
    QList<Repository> repos;
    repos.reserve(arr.size());

    for (const QJsonValue& val : arr) {
        if (!val.isObject()) continue;
        const Repository repo = parseRepository(val.toObject());
        if (repo.isValid)
            repos.append(repo);
    }

    return repos;
}

// Safe JSON helpers handle null and undefined values gracefully
QString GitHubService::jsonString(const QJsonObject& obj,
                                  const QString& key,
                                  const QString& defaultVal)
{
    const QJsonValue val = obj.value(key);
    if (val.isUndefined() || val.isNull())
        return defaultVal;
    return val.toString(defaultVal);
}

int GitHubService::jsonInt(const QJsonObject& obj,
                           const QString& key,
                           int defaultVal)
{
    const QJsonValue val = obj.value(key);
    if (val.isUndefined() || val.isNull())
        return defaultVal;
    return val.toInt(defaultVal);
}

bool GitHubService::jsonBool(const QJsonObject& obj,
                             const QString& key,
                             bool defaultVal)
{
    const QJsonValue val = obj.value(key);
    if (val.isUndefined() || val.isNull())
        return defaultVal;
    return val.toBool(defaultVal);
}

void GitHubService::fetchTrending(int days, int limit)
{
    const QString since = QDateTime::currentDateTimeUtc().addDays(-days).toString("yyyy-MM-dd");
    const QString query = QString("created:>%1").arg(since);

    m_activeRequests++;
    emit loadingChanged(true);

    ApiClient::instance().get("/search/repositories", {
                                                          { "q",        query },
                                                          { "sort",     "stars" },
                                                          { "order",    "desc" },
                                                          { "per_page", QString::number(limit) }
                                                      }, [this](const ApiResponse& response) {
                                  m_activeRequests = qMax(0, m_activeRequests - 1);
                                  if (response.isCancelled) {
                                      if (m_activeRequests == 0) emit loadingChanged(false);
                                      return;
                                  }
                                  if (!response.success) {
                                      emit error(response.errorMsg);
                                      emit loadingChanged(false);
                                      return;
                                  }

                                  const QJsonArray items = response.data.object()["items"].toArray();
                                  QList<Repository> repos;
                                  repos.reserve(items.size());
                                  for (const QJsonValue& val : items) repos.append(parseRepository(val.toObject()));

                                  emit loadingChanged(false);
                                  emit trendingLoaded(repos);
                              });
}

void GitHubService::fetchExploreRepos(const QString& sortType, int limit)
{
    const QString query = QString("%1:>1000").arg(sortType);

    m_activeRequests++;
    emit loadingChanged(true);

    ApiClient::instance().get("/search/repositories", {
                                                          { "q",        query },
                                                          { "sort",     sortType },
                                                          { "order",    "desc" },
                                                          { "per_page", QString::number(limit) }
                                                      }, [this](const ApiResponse& response) {
                                  m_activeRequests = qMax(0, m_activeRequests - 1);
                                  if (response.isCancelled) {
                                      if (m_activeRequests == 0) emit loadingChanged(false);
                                      return;
                                  }
                                  if (!response.success) {
                                      emit error(response.errorMsg);
                                      emit loadingChanged(false);
                                      return;
                                  }

                                  const QJsonArray items = response.data.object()["items"].toArray();
                                  QList<Repository> repos;
                                  repos.reserve(items.size());
                                  for (const QJsonValue& val : items) repos.append(parseRepository(val.toObject()));

                                  emit loadingChanged(false);
                                  emit exploreReposLoaded(repos);
                              });
}

void GitHubService::fetchTopDevelopers(int limit)
{
    m_activeRequests++;
    emit loadingChanged(true);

    ApiClient::instance().get("/search/users", {
                                                   { "q",        "followers:>1000" },
                                                   { "sort",     "followers" },
                                                   { "order",    "desc" },
                                                   { "per_page", QString::number(limit) }
                                               }, [this](const ApiResponse& response) {
                                  m_activeRequests = qMax(0, m_activeRequests - 1);
                                  if (response.isCancelled) {
                                      if (m_activeRequests == 0) emit loadingChanged(false);
                                      return;
                                  }
                                  if (!response.success) {
                                      emit error(response.errorMsg);
                                      emit loadingChanged(false);
                                      return;
                                  }

                                  const QJsonArray items = response.data.object()["items"].toArray();
                                  QList<UserProfile> devs;
                                  devs.reserve(items.size());
                                  for (const QJsonValue& val : items) devs.append(parseUserProfile(val.toObject()));

                                  emit loadingChanged(false);
                                  emit topDevelopersLoaded(devs);
                              });
}
