#pragma once

#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QList>

#include "repocard.h"
#include "repository.h"

class RepoListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RepoListWidget(QWidget* parent = nullptr);

    void setRepos(const QList<Repository>& repos, bool hasMore);
    void appendRepos(const QList<Repository>& repos, bool hasMore);
    void setLoadingMore(bool loading);
    void reset();

    // Allows the Bookmarks page to customize the header
    void setHeaderText(const QString& text);

signals:
    void loadMoreRequested();
    void bookmarkToggled(const Repository& repo, bool isBookmarked);
    void repoSelected(const Repository& repo);

private:
    void setupUI();
    void ensurePoolSize(int requiredSize);
    void updateHeader();

    QLabel* m_headerLabel    = nullptr;
    QScrollArea* m_scrollArea     = nullptr;
    QWidget* m_cardContainer  = nullptr;
    QVBoxLayout* m_cardLayout     = nullptr;
    QWidget* m_loadMoreWidget = nullptr;
    QPushButton* m_loadMoreButton = nullptr;

    QList<RepoCard*> m_cardPool;
    int              m_shownCount = 0;
    QString          m_baseHeaderText = "Repositories";
};
