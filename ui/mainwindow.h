#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QShortcut>
#include <QIcon>
#include <QEvent>
#include "navbutton.h"
#include "welcomepage.h"
#include "bookmarkspage.h"
#include "searchpage.h"
#include "settingspage.h"
#include "explorepage.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    void navigateToPage(int pageIndex);

private slots:
    void onThemeToggleClicked();
    void onThemeChanged(const QString& themeName);
    void onNavButtonClicked(int pageIndex);

protected:
    void closeEvent(QCloseEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setupUI();
    void setupHeader();
    void setupSidebar();
    void setupContentStack();
    void setupShortcuts();
    void updateThemeButtonLabel();
    void loadSettings();
    void connectServices();

    NavButton* makeNavButton(const QString& icon, const QString& text);
    QIcon loadIcon(const QString& iconName);

    // Header
    QWidget* m_headerBar            = nullptr;
    QWidget* m_brandContainer       = nullptr;
    QLabel* m_appTitleLabel         = nullptr;
    QPushButton* m_themeToggleButton = nullptr;

    // Sidebar
    QWidget* m_sidebarWidget = nullptr;
    NavButton* m_navSearch   = nullptr;
    NavButton* m_navExplore  = nullptr;
    NavButton* m_navBookmarks = nullptr;
    NavButton* m_navSettings = nullptr;
    QButtonGroup* m_navGroup = nullptr;

    // Content
    QStackedWidget* m_contentStack = nullptr;
    WelcomePage* m_welcomePage = nullptr;
    SearchPage* m_searchPage = nullptr;
    ExplorePage* m_explorePage = nullptr;
    BookmarksPage* m_bookmarksPage = nullptr;
    SettingsPage* m_settingsPage = nullptr;

    enum PageIndex {
        PAGE_WELCOME   = 0,
        PAGE_SEARCH    = 1,
        PAGE_EXPLORE   = 2,
        PAGE_BOOKMARKS = 3,
        PAGE_SETTINGS  = 4
    };
};
