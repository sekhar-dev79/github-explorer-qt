#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "repository.h"

class RepoCard : public QWidget
{
    Q_OBJECT
public:
    explicit RepoCard(QWidget* parent = nullptr);

    void setRepository(const Repository& repo);

protected:
    void mousePressEvent(QMouseEvent* event) override;

signals:
    void bookmarkToggled(const Repository& repo, bool isBookmarked);
    void clicked(const Repository& repo);

private slots:
    void onBookmarkClicked();

private:
    void setupUI();
    void updateBookmarkIcon();
    QIcon loadIcon(const QString& iconName);

    Repository m_repo;
    bool m_isBookmarked = false;

    // Row 1
    QLabel* m_nameLabel        = nullptr;
    QLabel* m_descLabel        = nullptr;
    QPushButton* m_bookmarkBtn = nullptr;

    // Row 2
    QLabel* m_languageDot      = nullptr;
    QLabel* m_languageLabel    = nullptr;
    QLabel* m_starsIcon        = nullptr;
    QLabel* m_starsLabel       = nullptr;
    QLabel* m_forksIcon        = nullptr;
    QLabel* m_forksLabel       = nullptr;
    QLabel* m_updatedLabel     = nullptr;
    QLabel* m_privateBadge     = nullptr;
};
