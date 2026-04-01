#pragma once

#include <QWidget>
#include <QSplitter>
#include "../widgets/repolistwidget.h"
#include "../widgets/repodetailwidget.h"

class BookmarksPage : public QWidget
{
    Q_OBJECT
public:
    explicit BookmarksPage(QWidget* parent = nullptr);
    void activatePage();

private:
    void loadBookmarks();

    QSplitter* m_splitter = nullptr;
    RepoListWidget* m_listWidget = nullptr;
    RepoDetailWidget* m_detailWidget = nullptr;
};
