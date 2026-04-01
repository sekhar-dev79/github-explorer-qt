#include "mainwindow.h"
#include "thememanager.h"
#include "constants.h"
#include "apiclient.h"
#include "githubservice.h"
#include "appsettings.h"

#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QShortcut>
#include <QStatusBar>
#include <QDateTime>
#include <QKeySequence>
#include <QPainter>
#include <QPixmap>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setupUI();
    setupShortcuts();
    connectServices();

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &MainWindow::onThemeChanged);

    connect(&ApiClient::instance(), &ApiClient::rateLimitUpdated,
            this, [this](int remaining, qint64 resetTs) {
                const qint64 now      = QDateTime::currentSecsSinceEpoch();
                const qint64 secsLeft = resetTs - now;
                const int    minsLeft = static_cast<int>(qMax(0LL, secsLeft / 60));
                statusBar()->showMessage(
                    QString("Rate limit: %1 remaining  ·  Resets in %2 min")
                        .arg(remaining).arg(minsLeft));
            });

    connect(m_settingsPage,         &SettingsPage::tokenChanged,
            &ApiClient::instance(), &ApiClient::setAuthToken);

    connect(&ApiClient::instance(), &ApiClient::rateLimitUpdated,
            m_settingsPage,         &SettingsPage::updateRateLimit);

    loadSettings();
}

QIcon MainWindow::loadIcon(const QString& iconName) {
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#c9d1d9") : QColor("#24292f"));
    painter.end();
    return QIcon(pixmap);
}

void MainWindow::setupUI()
{
    setWindowTitle(AppConstants::APP_NAME);
    setMinimumSize(800, 600);
    resize(1024, 768);

    const QRect screen = QApplication::primaryScreen()->availableGeometry();
    move((screen.width()  - width())  / 2,
         (screen.height() - height()) / 2);

    setupHeader();
    setupSidebar();
    setupContentStack();
    updateThemeButtonLabel();

    auto* bodyWidget  = new QWidget(this);
    auto* bodyLayout  = new QHBoxLayout(bodyWidget);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);
    bodyLayout->addWidget(m_sidebarWidget);
    bodyLayout->addWidget(m_contentStack, 1);

    auto* centralWidget  = new QWidget(this);
    auto* centralLayout  = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(m_headerBar);
    centralLayout->addWidget(bodyWidget, 1);

    setCentralWidget(centralWidget);
    statusBar()->showMessage(QString("Ready  ·  Qt %1  ·  C++17").arg(QT_VERSION_STR));
}

// Intercepts clicks on the brand container to navigate Home
bool MainWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_brandContainer && event->type() == QEvent::MouseButtonPress) {
        navigateToPage(PAGE_WELCOME);
        return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setupHeader()
{
    m_headerBar = new QWidget(this);
    m_headerBar->setObjectName("headerBar");
    m_headerBar->setFixedHeight(56);

    // Brand Container (Clickable Area)
    m_brandContainer = new QWidget(m_headerBar);
    m_brandContainer->setCursor(Qt::PointingHandCursor);
    m_brandContainer->installEventFilter(this);

    auto* brandLayout = new QHBoxLayout(m_brandContainer);
    brandLayout->setContentsMargins(8, 4, 8, 4);
    brandLayout->setSpacing(12);

    auto* brandIcon = new QLabel(m_brandContainer);
    brandIcon->setPixmap(loadIcon("github").pixmap(24, 24));
    brandIcon->setAlignment(Qt::AlignCenter);

    auto* textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(0);

    m_appTitleLabel = new QLabel("GitHub Explorer", m_brandContainer);
    m_appTitleLabel->setObjectName("appTitleLabel");

    auto* brandSub = new QLabel("Developer Tool", m_brandContainer);
    brandSub->setObjectName("mutedLabel");

    QFont subFont = brandSub->font();
    subFont.setPixelSize(10);
    subFont.setBold(true);
    subFont.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
    brandSub->setFont(subFont);

    textLayout->addWidget(m_appTitleLabel);
    textLayout->addWidget(brandSub);
    textLayout->setAlignment(Qt::AlignVCenter);

    brandLayout->addWidget(brandIcon);
    brandLayout->addLayout(textLayout);

    // Theme Toggle
    m_themeToggleButton = new QPushButton(m_headerBar);
    m_themeToggleButton->setObjectName("outlineButton");
    m_themeToggleButton->setCursor(Qt::PointingHandCursor);
    m_themeToggleButton->setFixedSize(80, 28);
    connect(m_themeToggleButton, &QPushButton::clicked, this, &MainWindow::onThemeToggleClicked);

    // Assemble Header
    auto* layout = new QHBoxLayout(m_headerBar);
    layout->setContentsMargins(12, 0, 16, 0);
    layout->addWidget(m_brandContainer);
    layout->addStretch();
    layout->addWidget(m_themeToggleButton);
}

void MainWindow::setupSidebar()
{
    m_sidebarWidget = new QWidget(this);
    m_sidebarWidget->setObjectName("sidebarWidget");
    m_sidebarWidget->setFixedWidth(220);

    // Nav section
    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);

    m_navSearch    = makeNavButton("search",      "Search");
    m_navExplore  = makeNavButton("trending-up", "Explore");
    m_navBookmarks = makeNavButton("bookmark",    "Bookmarks");
    m_navSettings  = makeNavButton("settings",    "Settings");

    m_navGroup->addButton(m_navSearch,    PAGE_SEARCH);
    m_navGroup->addButton(m_navExplore,  PAGE_EXPLORE);
    m_navGroup->addButton(m_navBookmarks, PAGE_BOOKMARKS);
    m_navGroup->addButton(m_navSettings,  PAGE_SETTINGS);

    connect(m_navGroup, &QButtonGroup::idClicked, this, &MainWindow::onNavButtonClicked);

    auto* navSectionLabel = new QLabel("NAVIGATION", m_sidebarWidget);
    navSectionLabel->setObjectName("navSectionLabel");

    auto* navContainer = new QWidget(m_sidebarWidget);
    navContainer->setObjectName("navSection");
    auto* navLayout = new QVBoxLayout(navContainer);
    navLayout->setContentsMargins(16, 24, 16, 12);
    navLayout->setSpacing(6);
    navLayout->addWidget(navSectionLabel);
    navLayout->addSpacing(12);
    navLayout->addWidget(m_navSearch);
    navLayout->addWidget(m_navExplore);
    navLayout->addWidget(m_navBookmarks);
    navLayout->addWidget(m_navSettings);
    navLayout->addStretch();

    // Separator
    auto* sep2 = new QFrame(m_sidebarWidget);
    sep2->setObjectName("separator");
    sep2->setFrameShape(QFrame::HLine);

    // Polished Footer
    auto* footerWidget = new QWidget(m_sidebarWidget);
    auto* footerLayout = new QVBoxLayout(footerWidget);
    footerLayout->setContentsMargins(20, 16, 20, 16);
    footerLayout->setSpacing(4); // Tight spacing for text stack

    auto* footerTitle = new QLabel(AppConstants::APP_NAME, footerWidget);
    footerTitle->setStyleSheet(QString("font-weight: 600; font-size: 13px; color: %1;")
                                   .arg(ThemeManager::instance().isDarkTheme() ? "#c9d1d9" : "#24292f"));

    auto* footerVersion = new QLabel(QString("Version %1").arg(AppConstants::APP_VERSION), footerWidget);
    footerVersion->setObjectName("mutedLabel");

    auto* footerQt = new QLabel(QString("Qt %1 · C++17").arg(QT_VERSION_STR), footerWidget);
    footerQt->setObjectName("mutedLabel");
    QFont f = footerQt->font();
    f.setPixelSize(10);
    footerQt->setFont(f);

    footerLayout->addWidget(footerTitle);
    footerLayout->addWidget(footerVersion);
    footerLayout->addWidget(footerQt);

    auto* sidebarLayout = new QVBoxLayout(m_sidebarWidget);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);
    sidebarLayout->addWidget(navContainer, 1);
    sidebarLayout->addWidget(sep2);
    sidebarLayout->addWidget(footerWidget);
}

void MainWindow::setupContentStack()
{
    m_contentStack = new QStackedWidget(this);
    m_contentStack->setObjectName("contentStack");

    m_welcomePage = new WelcomePage(m_contentStack);
    m_searchPage = new SearchPage(m_contentStack);
    m_explorePage = new ExplorePage(m_contentStack);
    m_bookmarksPage = new BookmarksPage(m_contentStack);
    m_settingsPage = new SettingsPage(m_contentStack);

    m_contentStack->addWidget(m_welcomePage);
    m_contentStack->addWidget(m_searchPage);
    m_contentStack->addWidget(m_explorePage);
    m_contentStack->addWidget(m_bookmarksPage);
    m_contentStack->addWidget(m_settingsPage);

    m_contentStack->setCurrentIndex(PAGE_WELCOME);

    // Only UI-to-UI connections belong here
    connect(m_welcomePage, &WelcomePage::startExploringRequested, this, [this]() { navigateToPage(PAGE_SEARCH); });
}

void MainWindow::setupShortcuts()
{
    auto* shortcutSearch = new QShortcut(QKeySequence("Ctrl+K"), this);
    connect(shortcutSearch, &QShortcut::activated, this, [this]() {
        navigateToPage(PAGE_SEARCH);
        m_searchPage->activatePage();
    });

    auto* shortcutEscape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(shortcutEscape, &QShortcut::activated, this, [this]() {
        if (m_contentStack->currentIndex() == PAGE_SEARCH)
            m_searchPage->clearSearch();
    });

    auto* shortcutRefresh = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(shortcutRefresh, &QShortcut::activated, this, [this]() {
        if (m_contentStack->currentIndex() == PAGE_EXPLORE)
            m_explorePage->refresh();
    });

    auto* sc1 = new QShortcut(QKeySequence("Ctrl+1"), this);
    auto* sc2 = new QShortcut(QKeySequence("Ctrl+2"), this);
    auto* sc3 = new QShortcut(QKeySequence("Ctrl+3"), this);

    connect(sc1, &QShortcut::activated, this, [this]() { navigateToPage(PAGE_SEARCH);   m_searchPage->activatePage(); });
    connect(sc2, &QShortcut::activated, this, [this]() { navigateToPage(PAGE_EXPLORE); m_explorePage->activatePage(); });
    connect(sc3, &QShortcut::activated, this, [this]() { navigateToPage(PAGE_SETTINGS); m_settingsPage->activatePage(); });

    auto* shortcutPrefs = new QShortcut(QKeySequence("Ctrl+,"), this);
    connect(shortcutPrefs, &QShortcut::activated, this, [this]() {
        navigateToPage(PAGE_SETTINGS);
        m_settingsPage->activatePage();
    });

    statusBar()->showMessage("Ctrl+K Search  ·  Ctrl+2 Explore  ·  Ctrl+, Settings  ·  Esc Clear");
}

void MainWindow::connectServices()
{
    auto& service = GitHubService::instance();

    //  SEARCH PAGE CONNECTIONS
    connect(m_searchPage, &SearchPage::searchTriggered, &service, &GitHubService::searchUser);
    connect(m_searchPage, &SearchPage::searchCleared, &service, &GitHubService::cancelAll);
    connect(&service, &GitHubService::loadingChanged, m_searchPage, &SearchPage::setLoading);
    connect(&service, &GitHubService::error, m_searchPage, &SearchPage::showError);

    connect(&service, &GitHubService::userFound, this, [this, &service](const UserProfile& profile) {
        m_searchPage->showUserProfile(profile);
        service.fetchRepositories(profile.login, 1, AppConstants::REPOS_PER_PAGE);

        AppSettings::addToSearchHistory(profile.login);
        m_searchPage->searchBar()->updateHistory(AppSettings::loadSearchHistory());
    });

    connect(&service, &GitHubService::reposLoaded, this, [this](const QList<Repository>& repos, int page, int perPage) {
        m_searchPage->showRepos(repos, page, perPage);
    });

    connect(m_searchPage, &SearchPage::loadMoreTriggered, this, [&service](const QString& login, int page) {
        service.fetchRepositories(login, page, AppConstants::REPOS_PER_PAGE);
    });

    //  EXPLORE PAGE CONNECTIONS (Trending, Stars, Forks, Developers)
    // 1. UI Requests -> Service Methods
    connect(m_explorePage, &ExplorePage::requestTrending, &service, &GitHubService::fetchTrending);
    connect(m_explorePage, &ExplorePage::requestTopRepos, &service, &GitHubService::fetchExploreRepos);
    connect(m_explorePage, &ExplorePage::requestTopDevelopers, &service, &GitHubService::fetchTopDevelopers);

    // 2. Service Results -> UI Display
    connect(&service, &GitHubService::trendingLoaded, m_explorePage, &ExplorePage::showRepos);
    connect(&service, &GitHubService::exploreReposLoaded, m_explorePage, &ExplorePage::showRepos);
    connect(&service, &GitHubService::topDevelopersLoaded, m_explorePage, &ExplorePage::showDevelopers);

    // 3. Error Handling
    connect(&service, &GitHubService::error, m_explorePage, &ExplorePage::showError);
}

NavButton* MainWindow::makeNavButton(const QString& icon, const QString& text)
{
    return new NavButton(icon, text, m_sidebarWidget);
}

void MainWindow::navigateToPage(int pageIndex)
{
    m_contentStack->setCurrentIndex(pageIndex);

    // Un-toggle old sidebar buttons and highlight the new one
    if (QAbstractButton* btn = m_navGroup->button(pageIndex)) {
        m_navGroup->blockSignals(true);
        btn->setChecked(true);
        m_navGroup->blockSignals(false);

        for (QAbstractButton* sibling : m_navGroup->buttons()) sibling->update();
        btn->update();
    }

    const QStringList names = { "Home", "Search", "Explore", "Bookmarks", "Settings" };
    if (pageIndex < names.size())
        statusBar()->showMessage(QString("GitHub Explorer  ·  %1").arg(names[pageIndex]));

    switch (pageIndex) {
    case PAGE_SEARCH:   m_searchPage->activatePage();   break;
    case PAGE_EXPLORE: m_explorePage->activatePage(); break;
    case PAGE_BOOKMARKS: m_bookmarksPage->activatePage(); break;
    case PAGE_SETTINGS: m_settingsPage->activatePage(); break;
    default: break;
    }
}

void MainWindow::onNavButtonClicked(int pageIndex)
{
    navigateToPage(pageIndex);
}

void MainWindow::onThemeToggleClicked()
{
    ThemeManager::instance().toggleTheme();
    setupSidebar();
    for (QAbstractButton* btn : m_navGroup->buttons()) btn->update();
}

void MainWindow::onThemeChanged(const QString&)
{
    updateThemeButtonLabel();
}

void MainWindow::updateThemeButtonLabel()
{
    const bool isDark = ThemeManager::instance().isDarkTheme();
    m_themeToggleButton->setIcon(loadIcon(isDark ? "sun" : "moon"));
    m_themeToggleButton->setText(isDark ? " Light" : " Dark");
}

void MainWindow::loadSettings()
{
    const QString savedTheme = AppSettings::loadTheme();
    ThemeManager::instance().applyTheme(savedTheme);

    const QString savedToken = AppSettings::loadAuthToken();
    if (!savedToken.isEmpty()) {
        ApiClient::instance().setAuthToken(savedToken);
        if (m_settingsPage) m_settingsPage->setTokenDisplay(savedToken);
    }

    const QByteArray geo = AppSettings::loadWindowGeometry();
    if (!geo.isEmpty()) restoreGeometry(geo);

    const QByteArray winState = AppSettings::loadWindowState();
    if (!winState.isEmpty()) restoreState(winState);

    const QStringList history = AppSettings::loadSearchHistory();
    if (!history.isEmpty() && m_searchPage && m_searchPage->searchBar()) {
        m_searchPage->searchBar()->updateHistory(history);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    AppSettings::saveWindowGeometry(saveGeometry());
    AppSettings::saveWindowState(saveState());
    AppSettings::saveTheme(ThemeManager::instance().currentTheme());
    AppSettings::saveAuthToken(ApiClient::instance().hasAuthToken() ? m_settingsPage->currentToken() : QString{});
    event->accept();
}
