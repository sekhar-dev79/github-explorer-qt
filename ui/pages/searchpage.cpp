#include "searchpage.h"
#include "thememanager.h"
#include "apiclient.h"
#include <QFrame>
#include <QPainter>
#include <QPixmap>

SearchPage::SearchPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("contentWidget");
    setupUI();
}

QIcon SearchPage::loadIcon(const QString& iconName)
{
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);

    // Error icons are red, others use muted text color
    if (iconName == "alert-triangle") {
        painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#f85149") : QColor("#cf222e"));
    } else {
        painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#8b949e") : QColor("#57606a"));
    }

    painter.end();
    return QIcon(pixmap);
}

void SearchPage::setupUI()
{
    setupHeader();
    setupSearchBar();
    setupStateStack();
    setupSafetyTimeout();

    auto* searchContainer = new QWidget(this);
    searchContainer->setObjectName("searchContainer");

    auto* searchContainerLayout = new QVBoxLayout(searchContainer);
    searchContainerLayout->setContentsMargins(20, 16, 20, 16);
    searchContainerLayout->setSpacing(8);
    searchContainerLayout->addWidget(m_searchBar);
    searchContainerLayout->addWidget(m_hintLabel);

    auto* sep = new QFrame(this);
    sep->setObjectName("separator");
    sep->setFrameShape(QFrame::HLine);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(m_headerWidget);
    rootLayout->addWidget(searchContainer);
    rootLayout->addWidget(sep);
    rootLayout->addWidget(m_stateStack, 1);
}

void SearchPage::setupHeader()
{
    m_headerWidget = new QWidget(this);
    m_headerWidget->setObjectName("pageHeader");
    m_headerWidget->setFixedHeight(48);

    auto* title = new QLabel("Search Developers", m_headerWidget);
    title->setObjectName("pageTitle");

    auto* layout = new QHBoxLayout(m_headerWidget);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->addWidget(title);
    layout->addStretch();
}

void SearchPage::setupSearchBar()
{
    m_searchBar = new SearchBarWidget(this);

    m_hintLabel = new QLabel("Try  torvalds  ·  gvanrossum  ·  antirez  ·  tj", this);
    m_hintLabel->setObjectName("searchHintLabel");

    connect(m_searchBar, &SearchBarWidget::searchRequested, this, &SearchPage::onSearchRequested);
    connect(m_searchBar, &SearchBarWidget::cleared, this, &SearchPage::onSearchCleared);
}

void SearchPage::setupSafetyTimeout()
{
    m_safetyTimer = new QTimer(this);
    m_safetyTimer->setInterval(8000);
    m_safetyTimer->setSingleShot(true);

    connect(m_safetyTimer, &QTimer::timeout, this, [this]() {
        if (m_isLoading) {
            showError("Request timed out. Check your connection and try again.");
        }
    });
}

void SearchPage::setupStateStack()
{
    m_stateStack = new QStackedWidget(this);
    m_stateStack->setObjectName("stateStack");

    // Idle state: empty message with suggestions
    m_idlePage = new QWidget(this);
    m_idlePage->setObjectName("contentWidget");
    {
        auto* layout = new QVBoxLayout(m_idlePage);
        layout->setContentsMargins(20, 40, 20, 20);
        layout->setSpacing(16);

        auto* icon = new QLabel(m_idlePage);
        icon->setPixmap(loadIcon("users").pixmap(48, 48));
        icon->setAlignment(Qt::AlignCenter);

        auto* title = new QLabel("Find a GitHub Developer", m_idlePage);
        title->setObjectName("emptyStateTitle");
        title->setAlignment(Qt::AlignCenter);

        auto* body = new QLabel(
            "Enter a GitHub username above to explore their profile,\n"
            "repositories, commits, and issues.", m_idlePage);
        body->setObjectName("emptyStateBody");
        body->setAlignment(Qt::AlignCenter);

        auto* tipsRow = new QWidget(m_idlePage);
        auto* tipsLayout = new QHBoxLayout(tipsRow);
        tipsLayout->setContentsMargins(0, 0, 0, 0);
        tipsLayout->setSpacing(8);
        tipsLayout->addStretch();

        const QStringList tips = { "torvalds", "gvanrossum", "antirez", "tj" };
        for (const QString& tip : tips) {
            auto* chip = new QPushButton(tip, tipsRow);
            chip->setObjectName("suggestionChip");
            chip->setCursor(Qt::PointingHandCursor);
            connect(chip, &QPushButton::clicked, this, [this, tip]() {
                m_searchBar->setQuery(tip);
                onSearchRequested(tip);
            });
            tipsLayout->addWidget(chip);
        }
        tipsLayout->addStretch();

        layout->addStretch(2);
        layout->addWidget(icon);
        layout->addSpacing(8);
        layout->addWidget(title);
        layout->addWidget(body);
        layout->addSpacing(24);
        layout->addWidget(tipsRow);
        layout->addStretch(3);
    }

    // Loading state
    m_loadingPage = new QWidget(this);
    m_loadingPage->setObjectName("contentWidget");
    {
        auto* layout = new QVBoxLayout(m_loadingPage);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(16);

        m_loadingQueryLabel = new QLabel(m_loadingPage);
        m_loadingQueryLabel->setObjectName("loadingQueryLabel");
        m_loadingQueryLabel->setAlignment(Qt::AlignCenter);

        auto* spinnerLabel = new QLabel(m_loadingPage);
        spinnerLabel->setPixmap(loadIcon("loader").pixmap(36, 36));
        spinnerLabel->setAlignment(Qt::AlignCenter);

        auto* loadingText = new QLabel("Searching GitHub…", m_loadingPage);
        loadingText->setObjectName("loadingText");
        loadingText->setAlignment(Qt::AlignCenter);

        layout->addStretch();
        layout->addWidget(m_loadingQueryLabel);
        layout->addSpacing(8);
        layout->addWidget(spinnerLabel);
        layout->addWidget(loadingText);
        layout->addStretch();
    }

    // Results state: profile + repos
    m_resultsPlaceholder = new QWidget(this);
    m_resultsPlaceholder->setObjectName("resultsArea");
    {
        m_profileWidget  = new UserProfileWidget(m_resultsPlaceholder);
        m_repoListWidget = new RepoListWidget(m_resultsPlaceholder);

        connect(m_repoListWidget, &RepoListWidget::loadMoreRequested, this, &SearchPage::onLoadMoreRequested);

        auto* sep = new QFrame(m_resultsPlaceholder);
        sep->setObjectName("separator");
        sep->setFrameShape(QFrame::HLine);

        auto* l = new QVBoxLayout(m_resultsPlaceholder);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(0);
        l->addWidget(m_profileWidget);
        l->addWidget(sep);
        l->addWidget(m_repoListWidget, 1);
    }

    // Error state
    m_errorPage = new QWidget(this);
    m_errorPage->setObjectName("contentWidget");
    {
        auto* layout = new QVBoxLayout(m_errorPage);
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(16);

        m_errorIconLabel = new QLabel(m_errorPage);
        m_errorIconLabel->setPixmap(loadIcon("alert-triangle").pixmap(48, 48));
        m_errorIconLabel->setAlignment(Qt::AlignCenter);

        m_errorTextLabel = new QLabel("Something went wrong.", m_errorPage);
        m_errorTextLabel->setObjectName("errorText");
        m_errorTextLabel->setAlignment(Qt::AlignCenter);
        m_errorTextLabel->setWordWrap(true);

        m_retryButton = new QPushButton("Try Again", m_errorPage);
        m_retryButton->setObjectName("outlineButton");
        m_retryButton->setFixedWidth(120);
        m_retryButton->setCursor(Qt::PointingHandCursor);

        connect(m_retryButton, &QPushButton::clicked, this, &SearchPage::onRetryClicked);

        layout->addStretch();
        layout->addWidget(m_errorIconLabel);
        layout->addSpacing(8);
        layout->addWidget(m_errorTextLabel);
        layout->addSpacing(24);
        layout->addWidget(m_retryButton, 0, Qt::AlignHCenter);
        layout->addStretch();
    }

    m_stateStack->addWidget(m_idlePage);
    m_stateStack->addWidget(m_loadingPage);
    m_stateStack->addWidget(m_resultsPlaceholder);
    m_stateStack->addWidget(m_errorPage);

    m_stateStack->setCurrentIndex(STATE_IDLE);
}

void SearchPage::activatePage()
{
    m_searchBar->focusInput();
}

void SearchPage::setLoading(bool loading)
{
    m_isLoading = loading;
    m_searchBar->setLoading(loading);
    const int currentState = m_stateStack->currentIndex();

    if (loading) {
        m_safetyTimer->start();
        if (currentState != STATE_RESULTS) {
            if (m_loadingQueryLabel)
                m_loadingQueryLabel->setText(QString("Searching for  \"%1\"").arg(m_lastQuery));
            switchState(STATE_LOADING);
        }
    } else {
        m_safetyTimer->stop();
        if (currentState == STATE_LOADING) {
            switchState(STATE_IDLE);
        }
    }
}

void SearchPage::showError(const QString& message)
{
    m_searchBar->setLoading(false);
    m_safetyTimer->stop();
    m_isLoading = false;

    if (m_repoListWidget) m_repoListWidget->setLoadingMore(false);
    if (m_errorTextLabel) m_errorTextLabel->setText(message);

    switchState(STATE_ERROR);
}

void SearchPage::showIdle()
{
    m_isLoading = false;
    m_safetyTimer->stop();
    m_searchBar->setLoading(false);
    switchState(STATE_IDLE);
}

void SearchPage::handleEscapeKey()
{
    if (m_isLoading) {
        ApiClient::instance().cancelPendingRequests();
        onSearchCleared();
        return;
    }
    if (!m_searchBar->query().isEmpty()) {
        onSearchCleared();
        return;
    }
    if (m_stateStack->currentIndex() == STATE_RESULTS) {
        onSearchCleared();
    }
}

void SearchPage::clearSearch()
{
    if (m_lastQuery.isEmpty() && m_stateStack->currentIndex() == STATE_IDLE) return;
    onSearchCleared();
}

void SearchPage::switchState(int stateIndex)
{
    m_stateStack->setCurrentIndex(stateIndex);
}

void SearchPage::onSearchRequested(const QString& query)
{
    m_lastQuery = query;
    if (m_profileWidget)  m_profileWidget->reset();
    if (m_repoListWidget) m_repoListWidget->reset();

    m_isLoading = true;
    m_searchBar->setLoading(true);
    m_safetyTimer->start();

    if (m_loadingQueryLabel)
        m_loadingQueryLabel->setText(QString("Searching for  \"%1\"").arg(query));

    switchState(STATE_LOADING);
    emit searchTriggered(query);
}

void SearchPage::onSearchCleared()
{
    m_lastQuery.clear();
    setLoading(false);
    if (m_profileWidget)  m_profileWidget->reset();
    if (m_repoListWidget) m_repoListWidget->reset();
    switchState(STATE_IDLE);
    emit searchCleared();
}

void SearchPage::onRetryClicked()
{
    if (!m_lastQuery.isEmpty()) onSearchRequested(m_lastQuery);
}

void SearchPage::showUserProfile(const UserProfile& profile)
{
    m_searchBar->setLoading(false);
    m_safetyTimer->stop();
    m_isLoading = false;

    m_totalRepos       = profile.publicRepos;
    m_loadedReposCount = 0;
    m_currentPage      = 0;
    m_currentLogin     = profile.login;

    m_profileWidget->setProfile(profile);
    m_repoListWidget->reset();
    switchState(STATE_RESULTS);
}

void SearchPage::showRepos(const QList<Repository>& repos, int page, int perPage)
{
    m_currentPage      = page;
    m_loadedReposCount = (page - 1) * perPage + static_cast<int>(repos.size());
    const bool hasMore = (m_loadedReposCount < m_totalRepos);

    if (page == 1) {
        m_repoListWidget->setRepos(repos, hasMore);
    } else {
        m_repoListWidget->appendRepos(repos, hasMore);
    }
}

void SearchPage::onLoadMoreRequested()
{
    emit loadMoreTriggered(m_currentLogin, m_currentPage + 1);
}
