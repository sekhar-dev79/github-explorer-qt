#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTimer>
#include <QIcon>

class SettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsPage(QWidget* parent = nullptr);

    void updateRateLimit(int remaining, qint64 resetTimestamp);
    void activatePage();
    void setTokenDisplay(const QString& token);

    [[nodiscard]] QString currentToken() const { return m_tokenInput->text().trimmed(); }

signals:
    void tokenChanged(const QString& token);

private slots:
    void onSaveTokenClicked();
    void onClearTokenClicked();
    void onToggleTokenVisibility();

private:
    void setupUI();
    void setupHeader();
    void setupTokenSection();
    void setupRateLimitSection();
    void setupAboutSection();
    void updateRateLimitBar();
    QIcon loadIcon(const QString& iconName);
    QWidget* buildSectionCard(const QString& title, const QString& description, const QString& iconName);
    void resizeEvent(QResizeEvent* event) override;

    // Header
    QWidget* m_headerWidget     = nullptr;

    // Token section
    QLineEdit* m_tokenInput     = nullptr;
    QPushButton* m_toggleVisBtn = nullptr;
    QPushButton* m_saveTokenBtn = nullptr;
    QPushButton* m_clearTokenBtn = nullptr;
    QLabel* m_tokenStatusLabel = nullptr;

    // Rate limit section
    QLabel* m_rateLimitLabel   = nullptr;
    QLabel* m_resetTimeLabel   = nullptr;
    QWidget* m_rateLimitBar    = nullptr;
    QWidget* m_rateLimitFill   = nullptr;

    // State
    int  m_rateLimitRemaining = -1;
    int  m_rateLimitTotal     = -1;
    bool m_hasToken           = false;
};
