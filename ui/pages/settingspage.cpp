#include "settingspage.h"
#include "constants.h"
#include "thememanager.h"
#include <QFrame>
#include <QStyle>
#include <QDateTime>
#include <QPainter>
#include <QPixmap>

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("contentWidget");
    setupUI();
}

QIcon SettingsPage::loadIcon(const QString& iconName)
{
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#c9d1d9") : QColor("#24292f"));
    painter.end();
    return QIcon(pixmap);
}

void SettingsPage::setupUI()
{
    setupHeader();
    setupTokenSection();
    setupRateLimitSection();
    setupAboutSection();

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* sep1 = new QFrame(this);
    sep1->setObjectName("separator");
    sep1->setFrameShape(QFrame::HLine);

    // Wrap page content in scroll area for overflow handling
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("trendingScroll");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* scrollContent = new QWidget(scrollArea);
    scrollContent->setObjectName("contentWidget");
    auto* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(20, 20, 20, 32);
    scrollLayout->setSpacing(16);

    // Token section
    auto* tokenCard = buildSectionCard("GitHub Personal Access Token",
                                       "Increases your API rate limit from 60 to 5,000 requests per hour.",
                                       "key");
    auto* tokenCardBody = qobject_cast<QVBoxLayout*>(tokenCard->layout());

    auto* inputRow = new QHBoxLayout;
    inputRow->setContentsMargins(0, 0, 0, 0);
    inputRow->setSpacing(8);
    inputRow->addWidget(m_tokenInput, 1);
    inputRow->addWidget(m_toggleVisBtn);

    auto* btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(0, 0, 0, 0);
    btnRow->setSpacing(8);
    btnRow->addWidget(m_saveTokenBtn);
    btnRow->addWidget(m_clearTokenBtn);
    btnRow->addStretch();

    tokenCardBody->addSpacing(8);
    tokenCardBody->addLayout(inputRow);
    tokenCardBody->addSpacing(4);
    tokenCardBody->addLayout(btnRow);
    tokenCardBody->addWidget(m_tokenStatusLabel);

    // Guide section
    auto* guideCard = buildSectionCard("How to get a token", "", "info");
    auto* guideCardBody = qobject_cast<QVBoxLayout*>(guideCard->layout());

    auto* guideLabel = new QLabel(guideCard);
    guideLabel->setObjectName("settingsCardDesc");
    guideLabel->setWordWrap(true);
    guideLabel->setTextFormat(Qt::RichText);
    guideLabel->setOpenExternalLinks(true);
    guideLabel->setText(
        "<b>Step 1:</b> <a href=\"https://github.com/settings/tokens/new\" style=\"color: #2f81f7; text-decoration: none;\">Click here to open GitHub Developer Settings ↗</a><br><br>"
        "<b>Step 2:</b> Sign in and enter a Note (e.g., <i>GitHub Explorer</i>).<br><br>"
        "<b>Step 3:</b> Check the boxes for <b>read:user</b> and <b>public_repo</b>.<br><br>"
        "<b>Step 4:</b> Scroll down, click <b>Generate token</b>, and paste it into the field above."
        );
    guideCardBody->addWidget(guideLabel);

    // Rate limit section
    auto* rateLimitCard = buildSectionCard("API Rate Limit",
                                           "GitHub limits unauthenticated requests to 60/hour. Add a token to raise this to 5,000/hour.",
                                           "activity");
    auto* rlCardBody = qobject_cast<QVBoxLayout*>(rateLimitCard->layout());
    rlCardBody->addWidget(m_rateLimitLabel);
    rlCardBody->addWidget(m_rateLimitBar);
    rlCardBody->addWidget(m_resetTimeLabel);

    // About section
    auto* aboutCard = buildSectionCard("About", "", "github");
    auto* aboutBody = qobject_cast<QVBoxLayout*>(aboutCard->layout());

    auto* appNameLabel = new QLabel(QString("<b>%1</b> v%2").arg(AppConstants::APP_NAME).arg(AppConstants::APP_VERSION), this);
    appNameLabel->setObjectName("settingsAboutName");

    auto* builtWithLabel = new QLabel("Built with Qt " + QString(QT_VERSION_STR) + " · C++17", this);
    builtWithLabel->setObjectName("settingsAboutMeta");

    auto* apiLabel = new QLabel("Data provided by the <a href=\"https://docs.github.com/en/rest\" style=\"color: #2f81f7; text-decoration: none;\">GitHub REST API</a>", this);
    apiLabel->setObjectName("settingsAboutMeta");
    apiLabel->setOpenExternalLinks(true);

    aboutBody->addWidget(appNameLabel);
    aboutBody->addSpacing(4);
    aboutBody->addWidget(builtWithLabel);
    aboutBody->addWidget(apiLabel);

    scrollLayout->addWidget(tokenCard);
    scrollLayout->addWidget(guideCard);
    scrollLayout->addWidget(rateLimitCard);
    scrollLayout->addWidget(aboutCard);
    scrollLayout->addStretch();

    scrollArea->setWidget(scrollContent);

    root->addWidget(m_headerWidget);
    root->addWidget(sep1);
    root->addWidget(scrollArea, 1);
}

QWidget* SettingsPage::buildSectionCard(const QString& title, const QString& description, const QString& iconName)
{
    auto* card = new QWidget(this);
    card->setObjectName("settingsCard");

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 16, 20, 20);
    layout->setSpacing(8);

    auto* titleRow = new QHBoxLayout;
    titleRow->setContentsMargins(0, 0, 0, 0);
    titleRow->setSpacing(8);

    auto* iconLabel = new QLabel(card);
    iconLabel->setPixmap(loadIcon(iconName).pixmap(18, 18));

    auto* titleLabel = new QLabel(title, card);
    titleLabel->setObjectName("settingsCardTitle");

    titleRow->addWidget(iconLabel);
    titleRow->addWidget(titleLabel);
    titleRow->addStretch();

    layout->addLayout(titleRow);

    if (!description.isEmpty()) {
        auto* descLabel = new QLabel(description, card);
        descLabel->setObjectName("settingsCardDesc");
        descLabel->setWordWrap(true);
        layout->addWidget(descLabel);
    }

    auto* sep = new QFrame(card);
    sep->setObjectName("separator");
    sep->setFrameShape(QFrame::HLine);
    layout->addWidget(sep);
    layout->addSpacing(4);

    return card;
}

void SettingsPage::setupHeader()
{
    m_headerWidget = new QWidget(this);
    m_headerWidget->setObjectName("pageHeader");
    m_headerWidget->setFixedHeight(48);

    auto* title = new QLabel("Settings", m_headerWidget);
    title->setObjectName("pageTitle");

    auto* layout = new QHBoxLayout(m_headerWidget);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->addWidget(title);
    layout->addStretch();
}

void SettingsPage::setupTokenSection()
{
    m_tokenInput = new QLineEdit(this);
    m_tokenInput->setObjectName("tokenInput");
    m_tokenInput->setPlaceholderText("ghp_xxxxxxxxxxxxxxxxxxxx");
    m_tokenInput->setEchoMode(QLineEdit::Password);

    m_toggleVisBtn = new QPushButton("Show", this);
    m_toggleVisBtn->setObjectName("outlineButton");
    m_toggleVisBtn->setFixedWidth(60);
    m_toggleVisBtn->setCursor(Qt::PointingHandCursor);

    connect(m_toggleVisBtn, &QPushButton::clicked, this, &SettingsPage::onToggleTokenVisibility);

    m_saveTokenBtn = new QPushButton("Save Token", this);
    m_saveTokenBtn->setObjectName("primaryButton");
    m_saveTokenBtn->setCursor(Qt::PointingHandCursor);

    m_clearTokenBtn = new QPushButton("Clear", this);
    m_clearTokenBtn->setObjectName("secondaryButton");
    m_clearTokenBtn->setCursor(Qt::PointingHandCursor);

    connect(m_saveTokenBtn,  &QPushButton::clicked, this, &SettingsPage::onSaveTokenClicked);
    connect(m_clearTokenBtn, &QPushButton::clicked, this, &SettingsPage::onClearTokenClicked);

    m_tokenStatusLabel = new QLabel(this);
    m_tokenStatusLabel->setObjectName("tokenStatusLabel");
    m_tokenStatusLabel->setWordWrap(true);
    m_tokenStatusLabel->setMinimumHeight(24);
    m_tokenStatusLabel->setVisible(false);

    QSizePolicy sp = m_tokenStatusLabel->sizePolicy();
    sp.setRetainSizeWhenHidden(true);
    m_tokenStatusLabel->setSizePolicy(sp);
}

void SettingsPage::setupRateLimitSection()
{
    m_rateLimitLabel = new QLabel("No data yet — make a search first.", this);
    m_rateLimitLabel->setObjectName("rateLimitText");

    m_resetTimeLabel = new QLabel(this);
    m_resetTimeLabel->setObjectName("rateLimitResetText");
    m_resetTimeLabel->setVisible(false);

    m_rateLimitBar = new QWidget(this);
    m_rateLimitBar->setObjectName("rateLimitTrack");
    m_rateLimitBar->setFixedHeight(6);
    m_rateLimitBar->setVisible(false);

    m_rateLimitFill = new QWidget(m_rateLimitBar);
    m_rateLimitFill->setObjectName("rateLimitFill");
    m_rateLimitFill->setFixedHeight(6);
}

void SettingsPage::setupAboutSection() {}

void SettingsPage::activatePage() {}

void SettingsPage::updateRateLimit(int remaining, qint64 resetTimestamp)
{
    m_rateLimitRemaining = remaining;
    const bool isAuth = (remaining > AppConstants::RATE_LIMIT_UNAUTH);
    const int  total  = isAuth ? AppConstants::RATE_LIMIT_AUTH : AppConstants::RATE_LIMIT_UNAUTH;
    m_rateLimitTotal = total;

    const QString tierStr = isAuth ? "5,000 (authenticated)" : "60 (unauthenticated)";
    m_rateLimitLabel->setText(QString("%1 of %2 requests remaining this hour").arg(remaining).arg(tierStr));

    if (resetTimestamp > 0) {
        const QDateTime resetLocal = QDateTime::fromSecsSinceEpoch(resetTimestamp).toLocalTime();
        m_resetTimeLabel->setText("Resets at " + resetLocal.toString("hh:mm") + " (local time)");
        m_resetTimeLabel->setVisible(true);
    }

    m_rateLimitBar->setVisible(true);
    updateRateLimitBar();
}

void SettingsPage::updateRateLimitBar()
{
    if (m_rateLimitTotal <= 0) return;

    const double ratio = static_cast<double>(m_rateLimitRemaining) / static_cast<double>(m_rateLimitTotal);

    if (ratio > 0.5) {
        m_rateLimitFill->setObjectName("rateLimitFill");
    } else if (ratio > 0.1) {
        m_rateLimitFill->setObjectName("rateLimitFillWarning");
    } else {
        m_rateLimitFill->setObjectName("rateLimitFillCritical");
    }
    m_rateLimitFill->style()->unpolish(m_rateLimitFill);
    m_rateLimitFill->style()->polish(m_rateLimitFill);

    const int fillWidth = qRound(m_rateLimitBar->width() * ratio);
    m_rateLimitFill->setFixedWidth(qMax(fillWidth, 0));
}

void SettingsPage::setTokenDisplay(const QString& token)
{
    m_tokenInput->setText(token);
    m_tokenStatusLabel->setText("Token loaded from previous session.");
    m_tokenStatusLabel->setObjectName("tokenStatusSuccess");
    m_tokenStatusLabel->style()->unpolish(m_tokenStatusLabel);
    m_tokenStatusLabel->style()->polish(m_tokenStatusLabel);
    m_tokenStatusLabel->setVisible(true);
    m_hasToken = true;
}

void SettingsPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateRateLimitBar();
}

void SettingsPage::onSaveTokenClicked()
{
    const QString token = m_tokenInput->text().trimmed();

    if (token.isEmpty()) {
        m_tokenStatusLabel->setText("Please enter a token before saving.");
        m_tokenStatusLabel->setObjectName("tokenStatusError");
    } else if (!token.startsWith("ghp_") && !token.startsWith("github_pat_")) {
        m_tokenStatusLabel->setText("Invalid format. Expected ghp_… or github_pat_…");
        m_tokenStatusLabel->setObjectName("tokenStatusWarning");
    } else {
        m_tokenStatusLabel->setText("Token saved. Rate limit raised to 5,000/hour.");
        m_tokenStatusLabel->setObjectName("tokenStatusSuccess");
    }

    m_tokenStatusLabel->style()->unpolish(m_tokenStatusLabel);
    m_tokenStatusLabel->style()->polish(m_tokenStatusLabel);
    m_tokenStatusLabel->setVisible(true);

    if (!token.isEmpty()) {
        m_hasToken = true;
        emit tokenChanged(token);
    }
}

void SettingsPage::onClearTokenClicked()
{
    m_tokenInput->clear();
    m_hasToken = false;

    m_tokenStatusLabel->setText("Token cleared. Rate limit is now 60/hour.");
    m_tokenStatusLabel->setObjectName("tokenStatusLabel");
    m_tokenStatusLabel->style()->unpolish(m_tokenStatusLabel);
    m_tokenStatusLabel->style()->polish(m_tokenStatusLabel);
    m_tokenStatusLabel->setVisible(true);

    emit tokenChanged(QString{});
}

void SettingsPage::onToggleTokenVisibility()
{
    const bool isHidden = (m_tokenInput->echoMode() == QLineEdit::Password);
    m_tokenInput->setEchoMode(isHidden ? QLineEdit::Normal : QLineEdit::Password);
    m_toggleVisBtn->setText(isHidden ? "Hide" : "Show");
}
