#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>
#include "userprofile.h"
#include "avatarwidget.h"

class UserProfileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserProfileWidget(QWidget* parent = nullptr);

    void setProfile(const UserProfile& profile);
    void reset();

private:
    void setupUI();
    QIcon loadIcon(const QString& iconName);
    QWidget* createMetaItem(const QString& iconName, QLabel*& textLabelOut);
    void addStat(const QString& iconName, QLabel* valueLabel, const QString& labelText, bool addDivider, QHBoxLayout* layout);

    // Avatar + name section
    AvatarWidget* m_avatar      = nullptr;
    QLabel* m_displayName       = nullptr;
    QLabel* m_loginLabel        = nullptr;
    QLabel* m_bioLabel          = nullptr;
    QLabel* m_memberLabel       = nullptr;

    // Meta row (location, company, blog, twitter)
    QWidget* m_locationContainer = nullptr;
    QLabel* m_locationLabel     = nullptr;
    QWidget* m_companyContainer = nullptr;
    QLabel* m_companyLabel      = nullptr;
    QWidget* m_blogContainer    = nullptr;
    QLabel* m_blogLabel         = nullptr;
    QWidget* m_twitterContainer = nullptr;
    QLabel* m_twitterLabel      = nullptr;

    // Stats row
    QLabel* m_reposValue     = nullptr;
    QLabel* m_gistsValue     = nullptr;
    QLabel* m_followersValue = nullptr;
    QLabel* m_followingValue = nullptr;

    // GitHub link button
    QPushButton* m_githubLinkBtn = nullptr;

    // Containers
    QWidget* m_metaRow  = nullptr;
    QWidget* m_statsRow = nullptr;
};
