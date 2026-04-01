#pragma once
#include <QString>

// Parsed GitHub repository object mirroring fields from GET /users/{username}/repos
struct Repository
{
    // Identity
    int     id           = 0;
    QString name         = {};
    QString fullName     = {};
    QString description  = {};
    QString htmlUrl      = {};
    QString language     = {};

    // Stats
    int     stargazers   = 0;
    int     forks        = 0;
    int     watchers     = 0;
    int     openIssues   = 0;

    // Flags
    bool    isPrivate    = false;
    bool    isFork       = false;
    bool    isArchived   = false;
    bool    hasIssues    = true;

    // Timestamps
    QString createdAt    = {};
    QString updatedAt    = {};
    QString pushedAt     = {};

    bool    isValid      = false;

    [[nodiscard]] QString displayLanguage() const
    {
        return language.isEmpty() ? "Unknown" : language;
    }

    [[nodiscard]] QString shortDescription() const
    {
        if (description.isEmpty()) return "No description";
        return description.length() > 100
                   ? description.left(97) + "…"
                   : description;
    }
};
