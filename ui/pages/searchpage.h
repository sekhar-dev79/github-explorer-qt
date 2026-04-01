#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QIcon>
#include "searchbarwidget.h"
#include "userprofilewidget.h"
#include "repolistwidget.h"
#include "userprofile.h"
#include "repository.h"

class SearchPage : public QWidget
{
    Q_OBJECT
public:
    explicit SearchPage(QWidget* parent = nullptr);

    void activatePage();
    void setLoading(bool loading);
    void showError(const QString& message);
    void showIdle();
    void handleEscapeKey();
    void clearSearch();
    void showUserProfile(const UserProfile& profile);
    void showRepos(const QList<Repository>& repos, int page, int perPage);

    QWidget* resultsArea() const { return m_resultsPlaceholder; }
    [[nodiscard]] SearchBarWidget* searchBar() const { return m_searchBar; }

signals:
    void searchTriggered(const QString& query);
    void searchCleared();
    void loadMoreTriggered(const QString& login, int page);

private slots:
    void onSearchRequested(const QString& query);
    void onSearchCleared();
    void onRetryClicked();
    void onLoadMoreRequested();

private:
    void setupUI();
    void setupHeader();
    void setupSearchBar();
    void setupStateStack();
    void setupSafetyTimeout();
    void switchState(int stateIndex);
    QIcon loadIcon(const QString& iconName);

    // Results area widgets
    UserProfileWidget* m_profileWidget   = nullptr;
    RepoListWidget* m_repoListWidget     = nullptr;

    // UI regions
    QWidget* m_headerWidget       = nullptr;
    SearchBarWidget* m_searchBar  = nullptr;
    QLabel* m_hintLabel           = nullptr;
    QStackedWidget* m_stateStack  = nullptr;

    // State pages
    QWidget* m_idlePage           = nullptr;
    QWidget* m_loadingPage        = nullptr;
    QWidget* m_resultsPlaceholder = nullptr;
    QWidget* m_errorPage          = nullptr;

    // Loading page widgets
    QLabel* m_loadingQueryLabel   = nullptr;

    // Error page widgets
    QLabel* m_errorIconLabel      = nullptr;
    QLabel* m_errorTextLabel      = nullptr;
    QPushButton* m_retryButton    = nullptr;

    QTimer* m_safetyTimer         = nullptr;

    // State
    bool              m_isLoading         = false;
    QString           m_lastQuery;
    int               m_totalRepos        = 0;
    int               m_loadedReposCount  = 0;
    int               m_currentPage       = 0;
    QString           m_currentLogin;

    enum StateIndex {
        STATE_IDLE    = 0,
        STATE_LOADING = 1,
        STATE_RESULTS = 2,
        STATE_ERROR   = 3
    };
};
