#include "repodetailwidget.h"
#include "userprofile.h"
#include "thememanager.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDateTime>
#include <QPainter>
#include <QStyle>

RepoDetailWidget::RepoDetailWidget(QWidget* parent) : QWidget(parent)
{
    setObjectName("contentWidget");
    setupUI();
    clear();
}

QIcon RepoDetailWidget::loadIcon(const QString& iconName) {
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#8b949e") : QColor("#57606a"));
    painter.end();
    return QIcon(pixmap);
}

void RepoDetailWidget::setupUI()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    // Premium Empty State
    m_emptyStateLabel = new QLabel(this);
    m_emptyStateLabel->setText("<div style='text-align: center;'>"
                               "<span style='font-size: 48px; color: #8b949e;'>♡</span><br><br>"
                               "<b style='font-size: 16px;'>No Repository Selected</b><br>"
                               "<span style='font-size: 13px; color: #8b949e;'>Select a bookmark from the list to view details.</span>"
                               "</div>");
    m_emptyStateLabel->setAlignment(Qt::AlignCenter);

    // ── Content Area ──
    m_contentWidget = new QWidget(this);
    auto* contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(32, 32, 32, 32);
    contentLayout->setSpacing(20);

    // Title & Badges
    m_fullNameLabel = new QLabel(this);
    m_fullNameLabel->setStyleSheet("font-size: 20px; font-weight: 700; color: #2f81f7;");
    m_fullNameLabel->setWordWrap(true);

    m_languageBadge = new QLabel(this);
    m_languageBadge->setObjectName("languageBadge");
    m_privateBadge = new QLabel(this);
    m_privateBadge->setObjectName("privateBadge");

    auto* badgeLayout = new QHBoxLayout;
    badgeLayout->setContentsMargins(0, 0, 0, 0);
    badgeLayout->setSpacing(8);
    badgeLayout->addWidget(m_languageBadge);
    badgeLayout->addWidget(m_privateBadge);
    badgeLayout->addStretch();

    // Description
    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setObjectName("repoDescription");
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet("font-size: 14px; line-height: 1.5;");

    // RESPONSIVE 2x2 STATS GRID
    auto* statsWidget = new QWidget(this);
    statsWidget->setObjectName("settingsCard");
    m_statsLayout = new QGridLayout(statsWidget);
    m_statsLayout->setContentsMargins(16, 16, 16, 16);
    m_statsLayout->setSpacing(16);

    // Dates
    m_datesLabel = new QLabel(this);
    m_datesLabel->setObjectName("repoMeta");

    // GitHub Button
    m_openGithubBtn = new QPushButton(" Open in GitHub ↗", this);
    m_openGithubBtn->setObjectName("primaryButton");
    m_openGithubBtn->setCursor(Qt::PointingHandCursor);
    m_openGithubBtn->setFixedWidth(160);

    connect(m_openGithubBtn, &QPushButton::clicked, this, [this]() {
        if (!m_currentRepo.htmlUrl.isEmpty()) {
            QDesktopServices::openUrl(QUrl(m_currentRepo.htmlUrl));
        }
    });

    contentLayout->addWidget(m_fullNameLabel);
    contentLayout->addLayout(badgeLayout);
    contentLayout->addWidget(m_descriptionLabel);
    contentLayout->addSpacing(8);
    contentLayout->addWidget(statsWidget);
    contentLayout->addWidget(m_datesLabel);
    contentLayout->addSpacing(16);
    contentLayout->addWidget(m_openGithubBtn);
    contentLayout->addStretch();

    root->addWidget(m_emptyStateLabel);
    root->addWidget(m_contentWidget);
}

void RepoDetailWidget::addStat(QGridLayout* layout, int row, int col, const QString& iconName, const QString& value, const QString& label)
{
    auto* block = new QWidget(this);
    auto* hLayout = new QHBoxLayout(block);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(8);
    hLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    auto* iconLabel = new QLabel(block);
    iconLabel->setPixmap(loadIcon(iconName).pixmap(18, 18));

    auto* textLayout = new QVBoxLayout;
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(2);

    auto* valLabel = new QLabel(value, block);
    valLabel->setObjectName("statValue");
    valLabel->setStyleSheet("font-size: 14px;");

    auto* textLabel = new QLabel(label, block);
    textLabel->setObjectName("statLabel");

    textLayout->addWidget(valLabel);
    textLayout->addWidget(textLabel);

    hLayout->addWidget(iconLabel);
    hLayout->addLayout(textLayout);

    layout->addWidget(block, row, col);
}

void RepoDetailWidget::setRepository(const Repository& repo)
{
    m_currentRepo = repo;
    m_emptyStateLabel->setVisible(false);
    m_contentWidget->setVisible(true);

    m_fullNameLabel->setText(repo.fullName.isEmpty() ? repo.name : repo.fullName);
    m_descriptionLabel->setText(repo.description.isEmpty() ? "No description provided." : repo.description);

    m_languageBadge->setText(repo.language);
    m_languageBadge->setVisible(!repo.language.isEmpty());

    m_privateBadge->setText(repo.isPrivate ? "Private" : "Public");
    m_privateBadge->setObjectName(repo.isPrivate ? "privateBadge" : "languageBadge");
    m_privateBadge->style()->unpolish(m_privateBadge);
    m_privateBadge->style()->polish(m_privateBadge);

    QDateTime created = QDateTime::fromString(repo.createdAt, Qt::ISODate);
    QDateTime updated = QDateTime::fromString(repo.updatedAt, Qt::ISODate);
    m_datesLabel->setText(QString("Created: %1   ·   Last Updated: %2")
                              .arg(created.isValid() ? created.toString("MMM d, yyyy") : "Unknown")
                              .arg(updated.isValid() ? updated.toString("MMM d, yyyy") : "Unknown"));

    // Clear old stats from the grid
    QLayoutItem* item;
    while ((item = m_statsLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Populate the 2x2 Grid
    // Row 0
    addStat(m_statsLayout, 0, 0, "star", UserProfile::formatCount(repo.stargazers), "STARS");
    addStat(m_statsLayout, 0, 1, "git-fork", UserProfile::formatCount(repo.forks), "FORKS");
    // Row 1
    addStat(m_statsLayout, 1, 0, "eye", UserProfile::formatCount(repo.watchers), "WATCHERS");
    addStat(m_statsLayout, 1, 1, "circle-dot", UserProfile::formatCount(repo.openIssues), "OPEN ISSUES");
}

void RepoDetailWidget::clear()
{
    m_currentRepo = Repository();
    m_emptyStateLabel->setVisible(true);
    m_contentWidget->setVisible(false);
}
