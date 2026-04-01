#include "explorepage.h"
#include "thememanager.h"
#include "avatarwidget.h"
#include <QPainter>
#include <QPixmap>
#include <QFrame>
#include <QDesktopServices>
#include <QUrl>

ExplorePage::ExplorePage(QWidget* parent) : QWidget(parent)
{
    setObjectName("contentWidget");
    setupUI();
}

QIcon ExplorePage::loadIcon(const QString& iconName)
{
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#c9d1d9") : QColor("#24292f"));
    painter.end();
    return QIcon(pixmap);
}

void ExplorePage::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* header = new QWidget(this);
    header->setObjectName("pageHeader");
    header->setFixedHeight(56);

    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(20, 0, 20, 0);
    headerLayout->setSpacing(16);

    auto* tabContainer = new QWidget(header);
    tabContainer->setObjectName("settingsCard");
    tabContainer->setFixedHeight(32);
    auto* tabLayout = new QHBoxLayout(tabContainer);
    tabLayout->setContentsMargins(2, 2, 2, 2);
    tabLayout->setSpacing(2);

    m_tabGroup = new QButtonGroup(this);
    m_tabGroup->setExclusive(true);

    auto* btnTrending = createTabButton(" Trending", "trending-up", 0);
    auto* btnStars    = createTabButton(" Top Starred", "star", 1);
    auto* btnForks    = createTabButton(" Most Forked", "git-fork", 2);
    auto* btnDevs     = createTabButton(" Top Developers", "users", 3);

    tabLayout->addWidget(btnTrending);
    tabLayout->addWidget(btnStars);
    tabLayout->addWidget(btnForks);
    tabLayout->addWidget(btnDevs);

    m_timeCombo = new QComboBox(header);
    m_timeCombo->setObjectName("compactCombo");
    m_timeCombo->addItem("This week", 7);
    m_timeCombo->addItem("This month", 30);
    m_timeCombo->addItem("Today", 1);
    m_timeCombo->setCursor(Qt::PointingHandCursor);

    m_limitCombo = new QComboBox(header);
    m_limitCombo->setObjectName("compactCombo");
    m_limitCombo->addItem("Top 30", 30);
    m_limitCombo->addItem("Top 50", 50);
    m_limitCombo->addItem("Top 100", 100);
    m_limitCombo->setCursor(Qt::PointingHandCursor);

    connect(m_tabGroup, &QButtonGroup::idClicked, this, &ExplorePage::onTabChanged);
    connect(m_timeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &ExplorePage::onFiltersChanged);
    connect(m_limitCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &ExplorePage::onFiltersChanged);

    headerLayout->addWidget(tabContainer);
    headerLayout->addStretch();
    headerLayout->addWidget(m_timeCombo);
    headerLayout->addWidget(m_limitCombo);

    auto* sep = new QFrame(this);
    sep->setObjectName("separator");
    sep->setFrameShape(QFrame::HLine);

    m_stack = new QStackedWidget(this);

    auto* statusPage = new QWidget();
    auto* statusLayout = new QVBoxLayout(statusPage);
    m_statusLabel = new QLabel("Select a tab to explore...", statusPage);
    m_statusLabel->setObjectName("mutedLabel");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    statusLayout->addWidget(m_statusLabel);

    m_repoList = new RepoListWidget(this);

    m_devScrollArea = new QScrollArea(this);
    m_devScrollArea->setObjectName("trendingScroll");
    m_devScrollArea->setWidgetResizable(true);
    m_devScrollArea->setFrameShape(QFrame::NoFrame);

    m_devContainer = new QWidget();
    m_devContainer->setObjectName("contentWidget");
    m_devLayout = new QVBoxLayout(m_devContainer);
    m_devLayout->setContentsMargins(20, 20, 20, 32);
    m_devLayout->setSpacing(8);
    m_devLayout->addStretch();
    m_devScrollArea->setWidget(m_devContainer);

    m_stack->addWidget(statusPage);
    m_stack->addWidget(m_repoList);
    m_stack->addWidget(m_devScrollArea);

    root->addWidget(header);
    root->addWidget(sep);
    root->addWidget(m_stack, 1);
}

QPushButton* ExplorePage::createTabButton(const QString& text, const QString& iconName, int id)
{
    auto* btn = new QPushButton(text, this);
    btn->setIcon(loadIcon(iconName));
    btn->setCheckable(true);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setObjectName("ghostButton");
    btn->setFixedHeight(26);
    m_tabGroup->addButton(btn, id);
    return btn;
}

void ExplorePage::activatePage()
{
    if (m_currentIndex == -1) {
        m_tabGroup->button(0)->setChecked(true);
        onTabChanged(0);
    }
}

void ExplorePage::refresh()
{
    int indexToRefresh = (m_currentIndex == -1) ? 0 : m_currentIndex;
    m_tabGroup->button(indexToRefresh)->setChecked(true);
    onTabChanged(indexToRefresh);
}

void ExplorePage::onFiltersChanged()
{
    refresh();
}

void ExplorePage::onTabChanged(int index)
{
    m_currentIndex = index;
    setLoading(true);

    int limit = m_limitCombo->currentData().toInt();

    // Time combo only visible on Trending tab
    m_timeCombo->setVisible(index == 0);

    if (index == 0) {
        int days = m_timeCombo->currentData().toInt();
        m_repoList->setHeaderText("Trending Repositories");
        emit requestTrending(days, limit);
    }
    else if (index == 1) {
        m_repoList->setHeaderText("Most Starred Repositories");
        emit requestTopRepos("stars", limit);
    }
    else if (index == 2) {
        m_repoList->setHeaderText("Most Forked Repositories");
        emit requestTopRepos("forks", limit);
    }
    else if (index == 3) {
        emit requestTopDevelopers(limit);
    }
}

void ExplorePage::setLoading(bool loading)
{
    if (loading) {
        m_stack->setCurrentIndex(0);
        m_statusLabel->setText("Fetching data from GitHub...");
    }
}

void ExplorePage::showError(const QString& message)
{
    m_stack->setCurrentIndex(0);
    m_statusLabel->setText("⚠ Error: " + message);
}

void ExplorePage::showRepos(const QList<Repository>& repos)
{
    m_repoList->setRepos(repos, false);
    m_stack->setCurrentIndex(1);
}

void ExplorePage::showDevelopers(const QList<UserProfile>& devs)
{
    clearDevLayout();

    for (const auto& dev : devs) {
        m_devLayout->addWidget(buildDevCard(dev));
    }
    m_devLayout->addStretch();
    m_stack->setCurrentIndex(2);
}

void ExplorePage::clearDevLayout()
{
    QLayoutItem* item;
    while ((item = m_devLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

QWidget* ExplorePage::buildDevCard(const UserProfile& profile)
{
    auto* card = new QWidget(m_devContainer);
    card->setObjectName("settingsCard");

    auto* layout = new QHBoxLayout(card);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(16);

    auto* avatar = new AvatarWidget(40, card);
    avatar->loadAvatar(profile.avatarUrl, profile.name);

    auto* textLayout = new QVBoxLayout();
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);

    auto* nameLabel = new QLabel(QString("<b>%1</b>").arg(profile.login), card);
    nameLabel->setTextFormat(Qt::RichText);

    const QString typeStr = profile.type.isEmpty() ? "User" : profile.type;
    auto* typeLabel = new QLabel(QString("GitHub %1  ·  <a href='%2' style='color:#8b949e; text-decoration:none;'>%2</a>")
                                     .arg(typeStr).arg(profile.htmlUrl), card);
    typeLabel->setObjectName("settingsCardDesc");
    typeLabel->setOpenExternalLinks(true);

    textLayout->addWidget(nameLabel);
    textLayout->addWidget(typeLabel);

    const QString url = profile.htmlUrl;
    auto* visitBtn = new QPushButton(" ↗ Profile", card);
    visitBtn->setObjectName("outlineButton");
    visitBtn->setCursor(Qt::PointingHandCursor);
    connect(visitBtn, &QPushButton::clicked, this, [url]() { QDesktopServices::openUrl(QUrl(url)); });

    layout->addWidget(avatar);
    layout->addLayout(textLayout, 1);
    layout->addWidget(visitBtn);

    return card;
}
