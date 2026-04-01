#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include "repository.h"

class RepoDetailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit RepoDetailWidget(QWidget* parent = nullptr);

    void setRepository(const Repository& repo);
    void clear();

private:
    void setupUI();
    QIcon loadIcon(const QString& iconName);
    void addStat(QGridLayout* layout, int row, int col, const QString& icon, const QString& value, const QString& label);

    QWidget* m_contentWidget = nullptr;
    QLabel* m_emptyStateLabel = nullptr;

    QLabel* m_fullNameLabel = nullptr;
    QLabel* m_descriptionLabel = nullptr;
    QLabel* m_datesLabel = nullptr;

    QLabel* m_languageBadge = nullptr;
    QLabel* m_privateBadge = nullptr;

    QGridLayout* m_statsLayout = nullptr;
    QPushButton* m_openGithubBtn = nullptr;

    Repository m_currentRepo;
};
