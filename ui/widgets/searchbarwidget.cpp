#include "searchbarwidget.h"
#include "thememanager.h"

#include <QPainter>
#include <QPixmap>
#include <QAbstractItemView>

SearchBarWidget::SearchBarWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("searchBarWidget");
    setupUI();
    setupDebounce();
}

QIcon SearchBarWidget::loadIcon(const QString& iconName) {
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#8b949e") : QColor("#57606a"));
    painter.end();
    return QIcon(pixmap);
}

void SearchBarWidget::setupUI()
{
    // Search input
    m_input = new QLineEdit(this);

    m_historyModel = new QStringListModel(this);
    m_completer = new QCompleter(m_historyModel, this);
    m_completer->setFilterMode(Qt::MatchContains);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setMaxVisibleItems(8);

    m_completer->popup()->setObjectName("searchCompleterPopup");
    m_input->setCompleter(m_completer);

    connect(m_completer, qOverload<const QString&>(&QCompleter::activated),
            this, [this](const QString& suggestion) {
                m_input->setText(suggestion);
                emit searchRequested(suggestion);
            });

    m_input->setObjectName("searchInput");
    m_input->setPlaceholderText("Search GitHub username…");
    m_input->setMaxLength(39);
    m_input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Left Icon: Lucide Search
    auto* searchIconAction = new QAction(m_input);
    searchIconAction->setIcon(loadIcon("search"));
    m_input->addAction(searchIconAction, QLineEdit::LeadingPosition);

    // Right Icon: Lucide Clear (X)
    m_clearAction = new QAction(m_input);
    m_clearAction->setIcon(loadIcon("x"));
    m_clearAction->setVisible(false); // Hidden when empty
    connect(m_clearAction, &QAction::triggered, this, &SearchBarWidget::onClearTriggered);
    m_input->addAction(m_clearAction, QLineEdit::TrailingPosition);

    // Search button
    m_searchButton = new QPushButton("Search", this);
    m_searchButton->setObjectName("primaryButton");
    m_searchButton->setCursor(Qt::PointingHandCursor);
    m_searchButton->setEnabled(false);

    // Layout (Ultra-Compact)
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8); // Tight spacing between input and button
    layout->addWidget(m_input, 1);
    layout->addWidget(m_searchButton);

    // Connections
    connect(m_input, &QLineEdit::textEdited, this, &SearchBarWidget::onTextEdited);
    connect(m_input, &QLineEdit::returnPressed, this, &SearchBarWidget::onSearchButtonClicked);
    connect(m_searchButton, &QPushButton::clicked, this, &SearchBarWidget::onSearchButtonClicked);
}

void SearchBarWidget::setupDebounce()
{
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setInterval(DEBOUNCE_MS);
    m_debounceTimer->setSingleShot(true);

    connect(m_input, &QLineEdit::textEdited, m_debounceTimer, qOverload<>(&QTimer::start));
    connect(m_debounceTimer, &QTimer::timeout, this, &SearchBarWidget::onDebounceTimeout);
}

void SearchBarWidget::onTextEdited(const QString& text)
{
    const QString trimmed = text.trimmed();
    const bool hasEnoughChars = trimmed.length() >= MIN_QUERY_LEN;

    m_searchButton->setEnabled(hasEnoughChars && !m_isLoading);
    m_clearAction->setVisible(!text.isEmpty()); // Show X if text exists

    if (trimmed.isEmpty()) {
        m_debounceTimer->stop();
        emit cleared();
    }
}

void SearchBarWidget::onClearTriggered()
{
    m_input->clear();
    m_clearAction->setVisible(false);
    m_searchButton->setEnabled(false);
    m_debounceTimer->stop();
    emit cleared();
    m_input->setFocus();
}

void SearchBarWidget::onDebounceTimeout()
{
    triggerSearch();
}

void SearchBarWidget::onSearchButtonClicked()
{
    m_debounceTimer->stop();
    triggerSearch();
}

void SearchBarWidget::triggerSearch()
{
    const QString q = m_input->text().trimmed();
    if (q.length() < MIN_QUERY_LEN || m_isLoading) return;
    emit searchRequested(q);
}

void SearchBarWidget::setLoading(bool loading)
{
    m_isLoading = loading;
    m_searchButton->setEnabled(!loading && m_input->text().trimmed().length() >= MIN_QUERY_LEN);
    m_searchButton->setText(loading ? "…" : "Search");

    if (loading) {
        m_debounceTimer->stop();
    }
}

void SearchBarWidget::setQuery(const QString& query)
{
    m_input->blockSignals(true);
    m_input->setText(query);
    m_input->blockSignals(false);

    const bool hasEnough = query.trimmed().length() >= MIN_QUERY_LEN;
    m_searchButton->setEnabled(hasEnough && !m_isLoading);
    m_clearAction->setVisible(!query.isEmpty());
}

QString SearchBarWidget::query() const { return m_input->text().trimmed(); }
bool SearchBarWidget::isEmpty() const { return m_input->text().trimmed().isEmpty(); }
void SearchBarWidget::focusInput() { m_input->setFocus(); m_input->selectAll(); }

void SearchBarWidget::updateHistory(const QStringList& history)
{
    m_historyModel->setStringList(history);
}
