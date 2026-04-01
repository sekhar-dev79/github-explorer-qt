#include "userprofilewidget.h"
#include "constants.h"
#include "thememanager.h"
#include <QFrame>
#include <QDesktopServices>
#include <QUrl>
#include <QPixmap>
#include <QPainter>

UserProfileWidget::UserProfileWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("userProfileWidget");
    setupUI();
}

QIcon UserProfileWidget::loadIcon(const QString& iconName) {
    QPixmap pixmap(QString(":/icons/%1.svg").arg(iconName));
    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), ThemeManager::instance().isDarkTheme() ? QColor("#8b949e") : QColor("#57606a"));
    painter.end();
    return QIcon(pixmap);
}

QWidget* UserProfileWidget::createMetaItem(const QString& iconName, QLabel*& textLabelOut) {
    auto* w = new QWidget(this);
    auto* l = new QHBoxLayout(w);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(6);

    auto* iconLabel = new QLabel(w);
    iconLabel->setPixmap(loadIcon(iconName).pixmap(14, 14));

    textLabelOut = new QLabel(w);
    textLabelOut->setObjectName("profileMetaItem");

    l->addWidget(iconLabel);
    l->addWidget(textLabelOut);
    l->addStretch();
    w->setVisible(false);

    return w;
}

void UserProfileWidget::addStat(const QString& iconName, QLabel* valueLabel, const QString& labelText, bool addDivider, QHBoxLayout* layout) {
    auto* block = new QWidget(m_statsRow);
    auto* bl = new QVBoxLayout(block);
    // Ultra-compact margins for stats
    bl->setContentsMargins(AppConstants::SPACING_MD, AppConstants::SPACING_SM,
                           AppConstants::SPACING_MD, AppConstants::SPACING_SM);
    bl->setSpacing(2);
    bl->setAlignment(Qt::AlignCenter);

    auto* iconLabel = new QLabel(block);
    iconLabel->setPixmap(loadIcon(iconName).pixmap(14, 14));
    iconLabel->setAlignment(Qt::AlignCenter);

    valueLabel->setObjectName("profileStatValue");
    valueLabel->setAlignment(Qt::AlignCenter);

    auto* label = new QLabel(labelText, block);
    label->setObjectName("profileStatLabel");
    label->setAlignment(Qt::AlignCenter);

    bl->addWidget(iconLabel);
    bl->addWidget(valueLabel);
    bl->addWidget(label);
    layout->addWidget(block, 1);

    if (addDivider) {
        auto* div = new QFrame(m_statsRow);
        div->setObjectName("separator");
        div->setFrameShape(QFrame::VLine);
        div->setFixedWidth(1);
        layout->addWidget(div);
    }
}

void UserProfileWidget::setupUI()
{
    // Root layout (Ultra-compact)
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(
        AppConstants::SPACING_MD, AppConstants::SPACING_MD,
        AppConstants::SPACING_MD, AppConstants::SPACING_MD
        );
    root->setSpacing(AppConstants::SPACING_SM);

    // Row 1: Avatar + Identity
    auto* identityRow = new QHBoxLayout;
    identityRow->setSpacing(AppConstants::SPACING_MD);
    identityRow->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_avatar = new AvatarWidget(48, this);

    auto* nameCol = new QVBoxLayout;
    nameCol->setSpacing(2);

    m_displayName = new QLabel(this);
    m_displayName->setObjectName("profileDisplayName");

    m_loginLabel = new QLabel(this);
    m_loginLabel->setObjectName("profileLogin");

    m_memberLabel = new QLabel(this);
    m_memberLabel->setObjectName("profileMetaItem");

    nameCol->addWidget(m_displayName);
    nameCol->addWidget(m_loginLabel);
    nameCol->addWidget(m_memberLabel);
    nameCol->addStretch();

    // GitHub profile link button
    m_githubLinkBtn = new QPushButton("GitHub ↗", this);
    m_githubLinkBtn->setObjectName("outlineButton");
    m_githubLinkBtn->setCursor(Qt::PointingHandCursor);
    m_githubLinkBtn->setVisible(false);

    identityRow->addWidget(m_avatar);
    identityRow->addLayout(nameCol, 1);
    identityRow->addWidget(m_githubLinkBtn, 0, Qt::AlignTop);

    root->addLayout(identityRow);

    // Bio
    m_bioLabel = new QLabel(this);
    m_bioLabel->setObjectName("profileBio");
    m_bioLabel->setWordWrap(true);
    m_bioLabel->setVisible(false);
    root->addWidget(m_bioLabel);

    // Meta row (Icons + Text)
    m_metaRow = new QWidget(this);
    auto* metaLayout = new QVBoxLayout(m_metaRow);
    metaLayout->setContentsMargins(0, 4, 0, 4);
    metaLayout->setSpacing(4);

    // Make sure you have these icons downloaded in your resources!
    m_locationContainer = createMetaItem("map-pin", m_locationLabel);
    m_companyContainer  = createMetaItem("building", m_companyLabel);
    m_blogContainer     = createMetaItem("link", m_blogLabel);
    m_twitterContainer  = createMetaItem("twitter", m_twitterLabel);

    metaLayout->addWidget(m_locationContainer);
    metaLayout->addWidget(m_companyContainer);
    metaLayout->addWidget(m_blogContainer);
    metaLayout->addWidget(m_twitterContainer);
    m_metaRow->setVisible(false);
    root->addWidget(m_metaRow);

    // Separator
    auto* sep = new QFrame(this);
    sep->setObjectName("separator");
    sep->setFrameShape(QFrame::HLine);
    root->addWidget(sep);

    // Stats row
    m_statsRow = new QWidget(this);
    m_statsRow->setObjectName("profileStatsRow");
    auto* statsLayout = new QHBoxLayout(m_statsRow);
    statsLayout->setContentsMargins(0, 0, 0, 0);
    statsLayout->setSpacing(0);

    m_reposValue     = new QLabel("—", this);
    m_gistsValue     = new QLabel("—", this);
    m_followersValue = new QLabel("—", this);
    m_followingValue = new QLabel("—", this);

    addStat("book-marked", m_reposValue, "Repos", true, statsLayout);
    addStat("file-text", m_gistsValue, "Gists", true, statsLayout);
    addStat("users", m_followersValue, "Followers", true, statsLayout);
    addStat("user-check", m_followingValue, "Following", false, statsLayout);

    root->addWidget(m_statsRow);
}

void UserProfileWidget::setProfile(const UserProfile& p)
{
    m_avatar->loadAvatar(p.avatarUrl, p.name);
    m_displayName->setText(p.displayName());
    m_loginLabel->setText("@" + p.login);

    const QString since = p.memberSince();
    m_memberLabel->setText(since);
    m_memberLabel->setVisible(!since.isEmpty());

    m_bioLabel->setText(p.bio);
    m_bioLabel->setVisible(!p.bio.isEmpty());

    if (!p.htmlUrl.isEmpty()) {
        m_githubLinkBtn->setVisible(true);
        connect(m_githubLinkBtn, &QPushButton::clicked, this,
                [url = p.htmlUrl]() { QDesktopServices::openUrl(QUrl(url)); });
    }

    // Meta items
    bool anyMeta = false;

    auto setMeta = [&](QWidget* container, QLabel* label, const QString& text, bool isLink = false, const QString& url = {}) {
        if (text.isEmpty()) {
            container->setVisible(false);
            return;
        }
        anyMeta = true;
        container->setVisible(true);

        if (isLink && !url.isEmpty()) {
            label->setText(QString("<a href=\"%1\" style=\"color: #2f81f7; text-decoration: none;\">%2</a>")
                               .arg(url.toHtmlEscaped(), text.toHtmlEscaped()));
            label->setOpenExternalLinks(true);
            label->setTextInteractionFlags(Qt::TextBrowserInteraction);
        } else {
            label->setText(text);
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        }
    };

    setMeta(m_locationContainer, m_locationLabel, p.location);
    setMeta(m_companyContainer,  m_companyLabel, p.company);

    if (!p.blog.isEmpty()) {
        const QString blogUrl = p.blog.startsWith("http") ? p.blog : "https://" + p.blog;
        const QString displayBlog = QUrl(blogUrl).host().isEmpty() ? p.blog : QUrl(blogUrl).host();
        setMeta(m_blogContainer, m_blogLabel, displayBlog, true, blogUrl);
    } else {
        m_blogContainer->setVisible(false);
    }

    if (!p.twitterUsername.isEmpty()) {
        const QString twitterUrl = "https://twitter.com/" + p.twitterUsername;
        setMeta(m_twitterContainer, m_twitterLabel, "@" + p.twitterUsername, true, twitterUrl);
    } else {
        m_twitterContainer->setVisible(false);
    }

    m_metaRow->setVisible(anyMeta);

    // Stats
    m_reposValue->setText(UserProfile::formatCount(p.publicRepos));
    m_gistsValue->setText(UserProfile::formatCount(p.publicGists));
    m_followersValue->setText(UserProfile::formatCount(p.followers));
    m_followingValue->setText(UserProfile::formatCount(p.following));
}

void UserProfileWidget::reset()
{
    m_avatar->reset();
    m_displayName->clear();
    m_loginLabel->clear();
    m_memberLabel->setVisible(false);
    m_bioLabel->setVisible(false);
    m_metaRow->setVisible(false);
    m_githubLinkBtn->setVisible(false);
    m_githubLinkBtn->disconnect();

    m_locationContainer->setVisible(false);
    m_companyContainer->setVisible(false);
    m_blogContainer->setVisible(false);
    m_twitterContainer->setVisible(false);

    m_reposValue->setText("—");
    m_gistsValue->setText("—");
    m_followersValue->setText("—");
    m_followingValue->setText("—");
}
