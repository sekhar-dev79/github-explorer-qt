#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include "repository.h"

class TrendingPage : public QWidget
{
    Q_OBJECT
public:
    explicit TrendingPage(QWidget* parent = nullptr);

    void onRefreshClicked();
    void setLoading(bool loading);
    void showRepos(const QList<Repository>& repos);
    void showError(const QString& message);
    void activatePage();

signals:
    void refreshRequested(int days);

private slots:
    void onPeriodChanged(int index);

private:
    void setupUI();
    QWidget* buildRepoCard(const Repository& repo);
    void clearLayout();
    QIcon loadIcon(const QString& iconName);

    QLabel* m_headerTitle   = nullptr;
    QComboBox* m_periodCombo = nullptr;
    QPushButton* m_refreshBtn = nullptr;
    QLabel* m_statusLabel   = nullptr;
    QWidget* m_repoContainer = nullptr;
    QVBoxLayout* m_repoLayout = nullptr;
    QScrollArea* m_scrollArea = nullptr;

    int           m_currentDays = 7;
    bool          m_hasLoaded   = false;
};
