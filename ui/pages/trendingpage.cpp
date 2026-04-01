#include "trendingpage.h"
#include "userprofile.h"
#include "appsettings.h"
#include "thememanager.h"
#include "toast.h"
#include <QFrame>
#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QPixmap>

TrendingPage::TrendingPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("contentWidget");
    setupUI();
}

QIcon TrendingPage::loadIcon(const QString& iconName)
{
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#8b949e") : QColor("#57606a"));
    painter.end();
    return QIcon(pixmap);
}

void TrendingPage::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* header = new QWidget(this);
    header->setObjectName("pageHeader");
    header->setFixedHeight(48);

    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    headerLayout->setSpacing(12);

    m_headerTitle = new QLabel("Trending Repositories", header);
    m_headerTitle->setObjectName("pageTitle");

    m_periodCombo = new QComboBox(header);
    m_periodCombo->setObjectName("compactCombo");
    m_periodCombo->addItem("This week",  7);
    m_periodCombo->addItem("This month", 30);
    m_periodCombo->addItem("Today",      1);
    m_periodCombo->setCursor(Qt::PointingHandCursor);

    m_refreshBtn = new QPushButton("Refresh", header);
    m_refreshBtn->setObjectName("secondaryButton");
    m_refreshBtn->setCursor(Qt::PointingHandCursor);

    headerLayout->addWidget(m_headerTitle);
    headerLayout->addStretch();
    headerLayout->addWidget(m_periodCombo);
    headerLayout->addWidget(m_refreshBtn);

    connect(m_periodCombo, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &TrendingPage::onPeriodChanged);
    connect(m_refreshBtn, &QPushButton::clicked,
            this, &TrendingPage::onRefreshClicked);

    auto* sep = new QFrame(this);
    sep->setObjectName("separator");
    sep->setFrameShape(QFrame::HLine);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("trendingScroll");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_repoContainer = new QWidget(m_scrollArea);
    m_repoContainer->setObjectName("contentWidget");

    m_repoLayout = new QVBoxLayout(m_repoContainer);
    m_repoLayout->setContentsMargins(20, 20, 20, 20);
    m_repoLayout->setSpacing(8);

    m_statusLabel = new QLabel(m_repoContainer);
    m_statusLabel->setObjectName("mutedLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setText("Select a period and click Refresh\nto load trending repositories.");
    m_repoLayout->addWidget(m_statusLabel);
    m_repoLayout->addStretch();

    m_scrollArea->setWidget(m_repoContainer);

    root->addWidget(header);
    root->addWidget(sep);
    root->addWidget(m_scrollArea, 1);
}

void TrendingPage::clearLayout()
{
    QLayoutItem* item;
    while ((item = m_repoLayout->takeAt(0)) != nullptr) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
    m_statusLabel = nullptr;
}

void TrendingPage::activatePage()
{
    if (!m_hasLoaded)
        onRefreshClicked();
}

void TrendingPage::onPeriodChanged(int /*index*/)
{
    m_currentDays = m_periodCombo->currentData().toInt();
}

void TrendingPage::onRefreshClicked()
{
    m_currentDays = m_periodCombo->currentData().toInt();
    setLoading(true);
    emit refreshRequested(m_currentDays);
}

void TrendingPage::setLoading(bool loading)
{
    m_refreshBtn->setEnabled(!loading);
    m_periodCombo->setEnabled(!loading);
    m_refreshBtn->setText(loading ? "Loading…" : "Refresh");

    if (loading) {
        clearLayout();

        m_statusLabel = new QLabel("Loading trending repositories…", m_repoContainer);
        m_statusLabel->setObjectName("mutedLabel");
        m_statusLabel->setAlignment(Qt::AlignCenter);
        m_repoLayout->addWidget(m_statusLabel);
        m_repoLayout->addStretch();
    }
}

void TrendingPage::showRepos(const QList<Repository>& repos)
{
    m_hasLoaded = true;
    m_refreshBtn->setEnabled(true);
    m_periodCombo->setEnabled(true);
    m_refreshBtn->setText("Refresh");

    clearLayout();

    if (repos.isEmpty()) {
        auto* empty = new QLabel("No trending repos found for this period.", m_repoContainer);
        empty->setObjectName("mutedLabel");
        empty->setAlignment(Qt::AlignCenter);
        m_repoLayout->addWidget(empty);
        m_repoLayout->addStretch();
        return;
    }

    const QString periodStr = m_periodCombo->currentText().toLower();
    auto* subtitle = new QLabel(
        QString("Top %1 repositories trending %2  ·  sorted by stars")
            .arg(repos.size()).arg(periodStr),
        m_repoContainer);
    subtitle->setObjectName("trendingSubtitle");
    m_repoLayout->addWidget(subtitle);

    for (int i = 0; i < repos.size(); ++i) {
        QWidget* card = buildRepoCard(repos.at(i));
        m_repoLayout->addWidget(card);

        if (i < repos.size() - 1) {
            auto* div = new QFrame(m_repoContainer);
            div->setObjectName("separator");
            div->setFrameShape(QFrame::HLine);
            m_repoLayout->addWidget(div);
        }
    }

    m_repoLayout->addStretch();
}

void TrendingPage::showError(const QString& message)
{
    m_refreshBtn->setEnabled(true);
    m_periodCombo->setEnabled(true);
    m_refreshBtn->setText("Retry");

    clearLayout();

    auto* errLabel = new QLabel("⚠  " + message, m_repoContainer);
    errLabel->setObjectName("errorText");
    errLabel->setAlignment(Qt::AlignCenter);
    errLabel->setWordWrap(true);
    m_repoLayout->addWidget(errLabel);
    m_repoLayout->addStretch();
}

QWidget* TrendingPage::buildRepoCard(const Repository& repo)
{
    auto* card = new QWidget(m_repoContainer);
    card->setObjectName("trendingCard");

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(8);

    // Top row: owner/name + language badge + bookmark button
    const QString ownerPart = repo.fullName.section('/', 0, 0);
    const QString repoNamePart = repo.name;

    auto* topRow = new QHBoxLayout;
    topRow->setSpacing(8);

    auto* repoNameLabel = new QLabel(QString("<b>%1</b> / %2").arg(ownerPart.toHtmlEscaped(), repoNamePart.toHtmlEscaped()), card);
    repoNameLabel->setObjectName("trendingRepoName");
    repoNameLabel->setTextInteractionFlags(Qt::NoTextInteraction);
    repoNameLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    topRow->addWidget(repoNameLabel, 1);

    if (!repo.language.isEmpty()) {
        auto* langBadge = new QLabel(repo.language, card);
        langBadge->setObjectName("languageBadge");
        topRow->addWidget(langBadge);
    }

    auto* bookmarkBtn = new QPushButton(card);
    bookmarkBtn->setObjectName("navButton");
    bookmarkBtn->setFixedSize(24, 24);
    bookmarkBtn->setCursor(Qt::PointingHandCursor);
    bookmarkBtn->setToolTip("Bookmark Repository");
    bookmarkBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    // Update bookmark icon based on state
    auto updateBookmarkIcon = [bookmarkBtn](bool bookmarked) {
        QPixmap pixmap(QString(":/icons/bookmark.svg"));
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        if (bookmarked) {
            painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#2f81f7") : QColor("#0969da"));
        } else {
            painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#8b949e") : QColor("#57606a"));
        }
        painter.end();
        bookmarkBtn->setIcon(QIcon(pixmap));
    };

    bool initialBookmark = AppSettings::isBookmarked(repo.id);
    bookmarkBtn->setProperty("isBookmarked", initialBookmark);
    updateBookmarkIcon(initialBookmark);

    connect(bookmarkBtn, &QPushButton::clicked, this, [this, repo, bookmarkBtn, updateBookmarkIcon]() {
        bool current = bookmarkBtn->property("isBookmarked").toBool();
        bool newState = !current;

        bookmarkBtn->setProperty("isBookmarked", newState);
        AppSettings::saveBookmark(repo, newState);
        updateBookmarkIcon(newState);

        if (newState) {
            Toast::show(this->window(), "Saved to Bookmarks", "check-circle");
        } else {
            Toast::show(this->window(), "Removed from Bookmarks", "trash-2");
        }
    });

    topRow->addWidget(bookmarkBtn, 0);
    layout->addLayout(topRow);

    // Description
    if (!repo.description.isEmpty()) {
        auto* desc = new QLabel(repo.description, card);
        desc->setObjectName("trendingRepoDesc");
        desc->setWordWrap(true);
        desc->setTextInteractionFlags(Qt::NoTextInteraction);
        layout->addWidget(desc);
    }

    // Stats row: stars and forks
    auto* bottomRow = new QHBoxLayout;
    bottomRow->setSpacing(12);

    auto* starsIcon = new QLabel(card);
    starsIcon->setPixmap(loadIcon("star").pixmap(12, 12));
    auto* starsLabel = new QLabel(UserProfile::formatCount(repo.stargazers), card);
    starsLabel->setObjectName("repoStat");

    auto* forksIcon = new QLabel(card);
    forksIcon->setPixmap(loadIcon("git-fork").pixmap(12, 12));
    auto* forksLabel = new QLabel(UserProfile::formatCount(repo.forks), card);
    forksLabel->setObjectName("repoStat");

    bottomRow->addWidget(starsIcon);
    bottomRow->addWidget(starsLabel);
    bottomRow->addSpacing(8);
    bottomRow->addWidget(forksIcon);
    bottomRow->addWidget(forksLabel);
    bottomRow->addStretch();

    layout->addLayout(bottomRow);

    // Open on GitHub button
    const QString url = repo.htmlUrl;

    auto* visitBtn = new QPushButton(" ↗ Open on GitHub", card);
    visitBtn->setObjectName("outlineButton");
    visitBtn->setCursor(Qt::PointingHandCursor);

    connect(visitBtn, &QPushButton::clicked, this, [url]() {
        QDesktopServices::openUrl(QUrl(url));
    });

    layout->addWidget(visitBtn, 0, Qt::AlignLeft);

    return card;
}
