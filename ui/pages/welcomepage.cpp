#include "welcomepage.h"
#include "constants.h"
#include "thememanager.h"
#include <QFrame>
#include <QPainter>
#include <QPixmap>

WelcomePage::WelcomePage(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("contentWidget");
    setupUI();
}

QIcon WelcomePage::loadIcon(const QString& iconName)
{
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#8b949e") : QColor("#57606a"));
    painter.end();
    return QIcon(pixmap);
}

void WelcomePage::setupUI()
{
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto* centerContainer = new QWidget(this);
    auto* centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setAlignment(Qt::AlignCenter);
    centerLayout->setSpacing(24);

    m_titleLabel = new QLabel("GitHub Explorer", centerContainer);
    m_titleLabel->setObjectName("welcomeTitle");
    m_titleLabel->setAlignment(Qt::AlignCenter);

    m_subtitleLabel = new QLabel(
        "Search developers, browse repositories,\n"
        "track trending projects, and save bookmarks.",
        centerContainer
        );
    m_subtitleLabel->setObjectName("welcomeSubtitle");
    m_subtitleLabel->setAlignment(Qt::AlignCenter);

    m_ctaButton = new QPushButton(" Search Developers", centerContainer);

    // White icon for primary button
    QPixmap searchPix(":/icons/search.svg");
    QPainter btnPainter(&searchPix);
    btnPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    btnPainter.fillRect(searchPix.rect(), QColor("#ffffff"));
    btnPainter.end();
    m_ctaButton->setIcon(QIcon(searchPix));

    m_ctaButton->setObjectName("primaryButton");
    m_ctaButton->setCursor(Qt::PointingHandCursor);
    m_ctaButton->setFixedWidth(200);

    connect(m_ctaButton, &QPushButton::clicked,
            this, &WelcomePage::startExploringRequested);

    auto* ctaLayout = new QHBoxLayout;
    ctaLayout->setContentsMargins(0, 0, 0, 0);
    ctaLayout->addStretch();
    ctaLayout->addWidget(m_ctaButton);
    ctaLayout->addStretch();

    auto* divider = new QFrame(centerContainer);
    divider->setObjectName("separator");
    divider->setFrameShape(QFrame::HLine);
    divider->setFixedWidth(320);

    auto* divLayout = new QHBoxLayout;
    divLayout->setContentsMargins(0, 0, 0, 0);
    divLayout->addStretch();
    divLayout->addWidget(divider);
    divLayout->addStretch();

    // Feature chips
    const QList<QPair<QString, QString>> features = {
        { "search",      "User Search"  },
        { "book-marked", "Repositories" },
        { "trending-up", "Trending"     },
        { "bookmark",    "Bookmarks"    }
    };

    auto* chipsRow    = new QWidget(centerContainer);
    auto* chipsLayout = new QHBoxLayout(chipsRow);
    chipsLayout->setContentsMargins(0, 0, 0, 0);
    chipsLayout->setSpacing(12);
    chipsLayout->addStretch();

    for (const auto& [iconName, labelText] : features) {
        auto* chip = new QWidget(chipsRow);
        chip->setObjectName("featureChip");

        auto* chipLayout = new QHBoxLayout(chip);
        chipLayout->setContentsMargins(12, 4, 14, 4);
        chipLayout->setSpacing(6);

        auto* chipIcon = new QLabel(chip);
        chipIcon->setPixmap(loadIcon(iconName).pixmap(12, 12));

        auto* chipText = new QLabel(labelText, chip);
        chipText->setObjectName("featureChipText");

        chipLayout->addWidget(chipIcon);
        chipLayout->addWidget(chipText);
        chipsLayout->addWidget(chip);
    }
    chipsLayout->addStretch();

    auto* versionLabel = new QLabel(
        QString("v%1  ·  Qt %2  ·  C++17")
            .arg(AppConstants::APP_VERSION)
            .arg(QT_VERSION_STR),
        centerContainer
        );
    versionLabel->setObjectName("mutedLabel");
    versionLabel->setAlignment(Qt::AlignCenter);

    centerLayout->addStretch(2);
    centerLayout->addWidget(m_titleLabel);
    centerLayout->addWidget(m_subtitleLabel);
    centerLayout->addSpacing(8);
    centerLayout->addLayout(ctaLayout);
    centerLayout->addSpacing(16);
    centerLayout->addLayout(divLayout);
    centerLayout->addSpacing(16);
    centerLayout->addWidget(chipsRow);
    centerLayout->addStretch(2);
    centerLayout->addWidget(versionLabel);
    centerLayout->addSpacing(16);

    rootLayout->addWidget(centerContainer);
}
