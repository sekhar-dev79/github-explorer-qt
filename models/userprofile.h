#pragma once
#include <QString>
#include <QDateTime>

struct UserProfile
{
    // Identity
    QString  login;
    QString  name;
    QString  avatarUrl;
    QString  htmlUrl;
    QString  type;            // "User" | "Organization"

    // Biography
    QString  bio;
    QString  company;
    QString  blog;
    QString  location;
    QString  email;
    QString  twitterUsername;
    bool     hireable   = false;

    // Stats
    int  publicRepos   = 0;
    int  publicGists   = 0;
    int  followers     = 0;
    int  following     = 0;

    // Timestamps (stored as QDateTime for formatting flexibility)
    QDateTime createdAt;
    QDateTime updatedAt;

    [[nodiscard]] QString displayName() const
    {
        return name.isEmpty() ? login : name;
    }

    // Format counts: 1200 → "1.2k", 574 → "574"
    [[nodiscard]] static QString formatCount(int n)
    {
        if (n >= 1'000'000) return QString::number(n / 1'000'000.0, 'f', 1) + "M";
        if (n >= 1'000)     return QString::number(n / 1'000.0,     'f', 1) + "k";
        return QString::number(n);
    }

    [[nodiscard]] QString memberSince() const
    {
        if (!createdAt.isValid()) return {};
        return "Member since " + createdAt.toString("MMM yyyy");
    }
};
