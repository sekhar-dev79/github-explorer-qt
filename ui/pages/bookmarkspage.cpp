#include "bookmarkspage.h"
#include "appsettings.h"
#include "thememanager.h"

BookmarksPage::BookmarksPage(QWidget* parent) : QWidget(parent)
{
    setObjectName("contentWidget");
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setStyleSheet(QString(
                                  "QSplitter::handle {"
                                  "   background-color: %1;"
                                  "   width: 1px;"
                                  "}"
                                  ).arg(ThemeManager::instance().isDarkTheme() ? "#30363d" : "#d0d7de"));

    m_listWidget = new RepoListWidget(this);
    m_listWidget->setHeaderText("Saved Bookmarks");
    m_listWidget->setMinimumWidth(320);

    m_detailWidget = new RepoDetailWidget(this);
    m_detailWidget->setMinimumWidth(380);

    m_splitter->addWidget(m_listWidget);
    m_splitter->addWidget(m_detailWidget);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 2);

    layout->addWidget(m_splitter);

    connect(m_listWidget, &RepoListWidget::repoSelected, this, [this](const Repository& repo) {
        m_detailWidget->setRepository(repo);
    });

    connect(m_listWidget, &RepoListWidget::bookmarkToggled, this, [this](const Repository&, bool isBookmarked) {
        if (!isBookmarked) {
            loadBookmarks();
            m_detailWidget->clear();
        }
    });
}

void BookmarksPage::activatePage()
{
    loadBookmarks();
    m_detailWidget->clear();
}

void BookmarksPage::loadBookmarks()
{
    QList<Repository> bookmarks = AppSettings::loadBookmarks();
    m_listWidget->setRepos(bookmarks, false);
}
