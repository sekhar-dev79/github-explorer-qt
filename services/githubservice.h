#pragma once

#include <QObject>
#include <QList>

#include "userprofile.h"
#include "repository.h"

class GitHubService : public QObject
{
    Q_OBJECT

public:
    static GitHubService& instance();

    void searchUser(const QString& username);
    void fetchRepositories(const QString& username, int page = 1, int perPage = 30);
    void cancelAll();

public slots:
    void fetchTrending(int days = 7, int limit = 30);
    void fetchExploreRepos(const QString& sortType, int limit = 30);
    void fetchTopDevelopers(int limit = 30);

signals:
    void userFound(const UserProfile& profile);
    void reposLoaded(const QList<Repository>& repos, int page, int perPage);
    void loadingChanged(bool isLoading);
    void error(const QString& message);

    void trendingLoaded(const QList<Repository>& repos);
    void exploreReposLoaded(const QList<Repository>& repos);
    void topDevelopersLoaded(const QList<UserProfile>& devs);

private:
    explicit GitHubService(QObject* parent = nullptr);
    GitHubService(const GitHubService&)            = delete;
    GitHubService& operator=(const GitHubService&) = delete;

    [[nodiscard]] static UserProfile parseUserProfile(const QJsonObject& obj);
    [[nodiscard]] static Repository  parseRepository (const QJsonObject& obj);
    [[nodiscard]] static QList<Repository> parseRepositoryList(const QJsonArray& arr);

    [[nodiscard]] static QString jsonString(const QJsonObject& obj, const QString& key, const QString& defaultVal = {});
    [[nodiscard]] static int     jsonInt   (const QJsonObject& obj, const QString& key, int defaultVal = 0);
    [[nodiscard]] static bool    jsonBool  (const QJsonObject& obj, const QString& key, bool defaultVal = false);

    int m_activeRequests = 0;
};
