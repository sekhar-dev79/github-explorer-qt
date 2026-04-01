#pragma once

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QScrollArea>
#include <QLabel>
#include "repository.h"
#include "userprofile.h"
#include "repolistwidget.h"

class ExplorePage : public QWidget
{
    Q_OBJECT
public:
    explicit ExplorePage(QWidget* parent = nullptr);

    void activatePage();
    void refresh();
    void showRepos(const QList<Repository>& repos);
    void showDevelopers(const QList<UserProfile>& devs);
    void setLoading(bool loading);
    void showError(const QString& message);

signals:
    void requestTrending(int days, int limit);
    void requestTopRepos(const QString& sortType, int limit);
    void requestTopDevelopers(int limit);

private slots:
    void onTabChanged(int index);
    void onFiltersChanged();

private:
    void setupUI();
    QPushButton* createTabButton(const QString& text, const QString& iconName, int id);
    QIcon loadIcon(const QString& iconName);
    QWidget* buildDevCard(const UserProfile& profile);
    void clearDevLayout();

    QButtonGroup* m_tabGroup = nullptr;
    QComboBox* m_timeCombo = nullptr;
    QComboBox* m_limitCombo = nullptr;

    QStackedWidget* m_stack = nullptr;
    RepoListWidget* m_repoList = nullptr;

    QWidget* m_devContainer = nullptr;
    QVBoxLayout* m_devLayout = nullptr;
    QScrollArea* m_devScrollArea = nullptr;

    QLabel* m_statusLabel = nullptr;
    int m_currentIndex = -1;
};
