#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QCompleter>
#include <QStringListModel>
#include <QAction>
#include <QIcon>

class SearchBarWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SearchBarWidget(QWidget* parent = nullptr);

    void setLoading(bool loading);
    void setQuery(const QString& query);

    [[nodiscard]] QString query() const;
    [[nodiscard]] bool    isEmpty() const;

    void focusInput();
    void updateHistory(const QStringList& history);

signals:
    void searchRequested(const QString& query);
    void cleared();

private slots:
    void onTextEdited(const QString& text);
    void onDebounceTimeout();
    void onSearchButtonClicked();
    void onClearTriggered();

private:
    void setupUI();
    void setupDebounce();
    void triggerSearch();
    QIcon loadIcon(const QString& iconName);

    QCompleter* m_completer       = nullptr;
    QStringListModel* m_historyModel = nullptr;

    // Widgets
    QLineEdit* m_input           = nullptr;
    QPushButton* m_searchButton  = nullptr;
    QAction* m_clearAction       = nullptr;

    // Debounce
    QTimer* m_debounceTimer      = nullptr;

    // State
    bool m_isLoading = false;

    // Config
    static constexpr int DEBOUNCE_MS   = 500;
    static constexpr int MIN_QUERY_LEN = 2;
};
