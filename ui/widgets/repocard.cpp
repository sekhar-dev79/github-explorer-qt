#include "repocard.h"
#include "userprofile.h"
#include "thememanager.h"
#include "appsettings.h"
#include <QFrame>
#include <QStyle>
#include <QDateTime>
#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QMouseEvent>

static QString languageColour(const QString& lang)
{
    static const QHash<QString, QString> colours = {
        { "C++", "#f34b7d" }, { "C", "#555555" }, { "Python", "#3572A5" }, { "JavaScript", "#f1e05a" },
        { "TypeScript", "#3178c6" }, { "Java", "#b07219" }, { "Go", "#00ADD8" }, { "Rust", "#dea584" },
        { "Ruby", "#701516" }, { "Swift", "#F05138" }, { "Kotlin", "#A97BFF" }, { "C#", "#178600" },
        { "PHP", "#4F5D95" }, { "Shell", "#89e051" }, { "HTML", "#e34c26" }, { "CSS", "#563d7c" }
    };
    return colours.value(lang, "#8b949e");
}

static QString relativeTime(const QDateTime& dt)
{
    if (!dt.isValid()) return {};
    const qint64 secsAgo = dt.secsTo(QDateTime::currentDateTimeUtc());
    if (secsAgo < 60) return "just now";
    if (secsAgo < 3600) return QString("%1 min ago").arg(secsAgo / 60);
    if (secsAgo < 86400) return QString("%1 hr ago").arg(secsAgo / 3600);
    if (secsAgo < 86400 * 30) return QString("%1 days ago").arg(secsAgo / 86400);
    if (secsAgo < 86400 * 365) return dt.toString("MMM d");
    return dt.toString("MMM yyyy");
}

RepoCard::RepoCard(QWidget* parent) : QWidget(parent)
{
    setObjectName("repoCard");
    setupUI();
}

void RepoCard::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    emit clicked(m_repo);
}

QIcon RepoCard::loadIcon(const QString& iconName)
{
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#8b949e") : QColor("#57606a"));
    painter.end();
    return QIcon(pixmap);
}

void RepoCard::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 8, 16, 8);
    root->setSpacing(4);

    auto* row1 = new QHBoxLayout;
    row1->setSpacing(8);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName("repoName");
    m_nameLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    m_nameLabel->setMinimumWidth(20);

    m_privateBadge = new QLabel("Private", this);
    m_privateBadge->setObjectName("privateBadge");
    m_privateBadge->setVisible(false);

    m_descLabel = new QLabel(this);
    m_descLabel->setObjectName("repoDescription");
    m_descLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_descLabel->setMinimumWidth(10);
    m_descLabel->setTextInteractionFlags(Qt::NoTextInteraction);

    m_bookmarkBtn = new QPushButton(this);
    m_bookmarkBtn->setObjectName("navButton");
    m_bookmarkBtn->setFixedSize(24, 24);
    m_bookmarkBtn->setCursor(Qt::PointingHandCursor);
    m_bookmarkBtn->setToolTip("Bookmark Repository");
    m_bookmarkBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(m_bookmarkBtn, &QPushButton::clicked, this, &RepoCard::onBookmarkClicked);

    row1->addWidget(m_nameLabel);
    row1->addWidget(m_privateBadge);
    row1->addWidget(m_descLabel, 1);
    row1->addWidget(m_bookmarkBtn, 0);

    root->addLayout(row1);

    auto* row2 = new QHBoxLayout;
    row2->setSpacing(6);
    row2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_languageDot = new QLabel("●", this);
    m_languageDot->setObjectName("languageDot");
    m_languageLabel = new QLabel(this);
    m_languageLabel->setObjectName("repoMeta");

    m_starsIcon = new QLabel(this);
    m_starsIcon->setPixmap(loadIcon("star").pixmap(12, 12));
    m_starsLabel = new QLabel(this);
    m_starsLabel->setObjectName("repoMeta");

    m_forksIcon = new QLabel(this);
    m_forksIcon->setPixmap(loadIcon("git-fork").pixmap(12, 12));
    m_forksLabel = new QLabel(this);
    m_forksLabel->setObjectName("repoMeta");

    m_updatedLabel = new QLabel(this);
    m_updatedLabel->setObjectName("repoMetaFaint");
    m_updatedLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_updatedLabel->setMinimumWidth(10);
    m_updatedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    row2->addWidget(m_languageDot);
    row2->addWidget(m_languageLabel);
    row2->addSpacing(8);
    row2->addWidget(m_starsIcon);
    row2->addWidget(m_starsLabel);
    row2->addSpacing(8);
    row2->addWidget(m_forksIcon);
    row2->addWidget(m_forksLabel);
    row2->addStretch();
    row2->addWidget(m_updatedLabel);

    root->addLayout(row2);
}

void RepoCard::setRepository(const Repository& repo)
{
    m_repo = repo;
    m_isBookmarked = AppSettings::isBookmarked(repo.id);
    updateBookmarkIcon();

    m_nameLabel->setText(repo.name);
    m_privateBadge->setVisible(repo.isPrivate);

    if (repo.description.isEmpty()) {
        m_descLabel->setText("No description");
        m_descLabel->setObjectName("repoDescriptionEmpty");
    } else {
        m_descLabel->setText(repo.description);
        m_descLabel->setObjectName("repoDescription");
    }
    m_descLabel->style()->unpolish(m_descLabel);
    m_descLabel->style()->polish(m_descLabel);

    if (repo.language.isEmpty()) {
        m_languageDot->setVisible(false);
        m_languageLabel->setVisible(false);
    } else {
        m_languageDot->setVisible(true);
        m_languageLabel->setVisible(true);
        m_languageDot->setStyleSheet(QString("color: %1; font-size: 10px;").arg(languageColour(repo.language)));
        m_languageLabel->setText(repo.language);
    }

    bool hasStars = repo.stargazers > 0;
    m_starsIcon->setVisible(hasStars);
    m_starsLabel->setVisible(hasStars);
    if (hasStars) m_starsLabel->setText(UserProfile::formatCount(repo.stargazers));

    bool hasForks = repo.forks > 0;
    m_forksIcon->setVisible(hasForks);
    m_forksLabel->setVisible(hasForks);
    if (hasForks) m_forksLabel->setText(UserProfile::formatCount(repo.forks));

    const QString updated = relativeTime(QDateTime::fromString(repo.updatedAt, Qt::ISODate));
    m_updatedLabel->setText(updated.isEmpty() ? QString() : "Updated " + updated);
    m_updatedLabel->setVisible(!updated.isEmpty());
}

void RepoCard::onBookmarkClicked()
{
    m_isBookmarked = !m_isBookmarked;
    updateBookmarkIcon();
    AppSettings::saveBookmark(m_repo, m_isBookmarked);
    emit bookmarkToggled(m_repo, m_isBookmarked);
}

void RepoCard::updateBookmarkIcon()
{
    QPixmap pixmap(QString(":/icons/bookmark.svg"));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);

    if (m_isBookmarked) {
        painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#2f81f7") : QColor("#0969da"));
    } else {
        painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#8b949e") : QColor("#57606a"));
    }
    painter.end();

    m_bookmarkBtn->setIcon(QIcon(pixmap));
}
