#include "repolistwidget.h"
#include "toast.h"
#include <QFrame>

RepoListWidget::RepoListWidget(QWidget* parent) : QWidget(parent) {
    setObjectName("repoListWidget");
    setupUI();
    setMinimumWidth(320); // Responsive Constraint
}

void RepoListWidget::setHeaderText(const QString& text) {
    m_baseHeaderText = text;
    updateHeader();
}

void RepoListWidget::setupUI() {
    m_headerLabel = new QLabel(m_baseHeaderText, this);
    m_headerLabel->setObjectName("sectionHeader");

    auto* headerWidget = new QWidget(this);
    headerWidget->setObjectName("repoListHeader");
    headerWidget->setFixedHeight(36);

    auto* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->addWidget(m_headerLabel);
    headerLayout->addStretch();

    m_cardContainer = new QWidget();
    m_cardContainer->setObjectName("cardContainer");
    m_cardLayout = new QVBoxLayout(m_cardContainer);
    m_cardLayout->setContentsMargins(0, 0, 0, 0);
    m_cardLayout->setSpacing(0);

    m_loadMoreWidget = new QWidget();
    m_loadMoreWidget->setObjectName("loadMoreArea");

    m_loadMoreButton = new QPushButton("Load More", m_loadMoreWidget);
    m_loadMoreButton->setObjectName("loadMoreButton");
    m_loadMoreButton->setCursor(Qt::PointingHandCursor);

    auto* lmLayout = new QHBoxLayout(m_loadMoreWidget);
    lmLayout->setContentsMargins(16, 12, 16, 16);
    lmLayout->addStretch();
    lmLayout->addWidget(m_loadMoreButton);
    lmLayout->addStretch();
    m_loadMoreWidget->hide();

    auto* scrollContent = new QWidget();
    scrollContent->setObjectName("repoScrollContent");
    auto* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(0);
    scrollLayout->addWidget(m_cardContainer);
    scrollLayout->addWidget(m_loadMoreWidget);
    scrollLayout->addStretch();

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("repoScrollArea");
    m_scrollArea->setWidget(scrollContent);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    auto* sep = new QFrame(this);
    sep->setObjectName("separator");
    sep->setFrameShape(QFrame::HLine);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->addWidget(headerWidget);
    rootLayout->addWidget(sep);
    rootLayout->addWidget(m_scrollArea, 1);

    connect(m_loadMoreButton, &QPushButton::clicked, this, [this]() {
        setLoadingMore(true);
        emit loadMoreRequested();
    });
}

void RepoListWidget::setRepos(const QList<Repository>& repos, bool hasMore) {
    for (int i = 0; i < m_shownCount; ++i) m_cardPool[i]->hide();
    m_shownCount = 0;
    m_loadMoreWidget->hide();
    appendRepos(repos, hasMore);
    m_scrollArea->verticalScrollBar()->setValue(0);
}

void RepoListWidget::appendRepos(const QList<Repository>& repos, bool hasMore) {
    if (repos.isEmpty() && m_shownCount == 0) {
        updateHeader();
        m_loadMoreWidget->hide();
        return;
    }
    ensurePoolSize(m_shownCount + repos.size());
    for (int i = 0; i < repos.size(); ++i) {
        const int poolIdx = m_shownCount + i;
        m_cardPool[poolIdx]->setRepository(repos[i]);
        m_cardPool[poolIdx]->show();
    }
    m_shownCount += repos.size();
    updateHeader();
    if (hasMore) {
        m_loadMoreButton->setEnabled(true);
        m_loadMoreButton->setText("Load More");
        m_loadMoreWidget->show();
    } else {
        m_loadMoreWidget->hide();
    }
}

void RepoListWidget::setLoadingMore(bool loading) {
    m_loadMoreButton->setEnabled(!loading);
    m_loadMoreButton->setText(loading ? "Loading…" : "Load More");
}

void RepoListWidget::reset() {
    for (int i = 0; i < m_shownCount; ++i) m_cardPool[i]->hide();
    m_shownCount = 0;
    m_headerLabel->setText(m_baseHeaderText);
    m_loadMoreWidget->hide();
}

void RepoListWidget::ensurePoolSize(int requiredSize) {
    while (m_cardPool.size() < requiredSize) {
        auto* card = new RepoCard(m_cardContainer);
        card->hide();

        // Forward signals AND trigger the global toast
        connect(card, &RepoCard::bookmarkToggled, this, [this](const Repository& repo, bool isBookmarked) {
            if (isBookmarked) {
                // Use this->window() so the toast floats over the entire application
                Toast::show(this->window(), "Saved to Bookmarks", "check-circle");
            } else {
                Toast::show(this->window(), "Removed from Bookmarks", "trash-2");
            }
            emit bookmarkToggled(repo, isBookmarked);
        });

        connect(card, &RepoCard::clicked, this, &RepoListWidget::repoSelected);
        m_cardLayout->addWidget(card);
        m_cardPool.append(card);
    }
}

void RepoListWidget::updateHeader() {
    if (m_shownCount == 0) {
        m_headerLabel->setText(QString("No %1").arg(m_baseHeaderText));
    } else {
        m_headerLabel->setText(QString("%1  (%2)").arg(m_baseHeaderText).arg(m_shownCount));
    }
}
