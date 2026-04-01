#pragma once

#include <QString>
#include <QHash>
#include <QDateTime>

// Pure utility functions; language colors sourced from GitHub's linguist project
class Helpers
{
public:
    Helpers() = delete;

    // Returns hex color for a GitHub language badge, falls back to grey
    [[nodiscard]] static QString languageColor(const QString& lang)
    {
        static const QHash<QString, QString> colors = {
                                                       { "C",           "#555555" },
                                                       { "C++",         "#f34b7d" },
                                                       { "C#",          "#178600" },
                                                       { "Python",      "#3572A5" },
                                                       { "JavaScript",  "#f1e05a" },
                                                       { "TypeScript",  "#3178c6" },
                                                       { "Java",        "#b07219" },
                                                       { "Go",          "#00ADD8" },
                                                       { "Rust",        "#dea584" },
                                                       { "Ruby",        "#701516" },
                                                       { "Swift",       "#F05138" },
                                                       { "Kotlin",      "#A97BFF" },
                                                       { "PHP",         "#4F5D95" },
                                                       { "Shell",       "#89e051" },
                                                       { "Bash",        "#89e051" },
                                                       { "HTML",        "#e34c26" },
                                                       { "CSS",         "#563d7c" },
                                                       { "SCSS",        "#c6538c" },
                                                       { "Vue",         "#41B883" },
                                                       { "Scala",       "#c22d40" },
                                                       { "Makefile",    "#427819" },
                                                       { "OpenSCAD",    "#e5cd31" },
                                                       { "Groovy",      "#4298b8" },
                                                       { "Lua",         "#000080" },
                                                       { "Perl",        "#0298c3" },
                                                       { "Haskell",     "#5e5086" },
                                                       { "Elixir",      "#6e4a7e" },
                                                       { "Dart",        "#00B4AB" },
                                                       { "R",           "#198CE7" },
                                                       { "MATLAB",      "#e16737" },
                                                       { "Dockerfile",  "#384d54" },
                                                       };
        return colors.value(lang, "#6e7681");
    }

    // Returns white or dark text based on background luminance (WCAG formula)
    [[nodiscard]] static QString badgeTextColor(const QString& bgHex)
    {
        QString hex = bgHex;
        if (hex.startsWith('#')) hex = hex.mid(1);
        bool ok = false;
        const int rgb = hex.toInt(&ok, 16);
        if (!ok) return "#ffffff";

        const int r = (rgb >> 16) & 0xFF;
        const int g = (rgb >>  8) & 0xFF;
        const int b =  rgb        & 0xFF;

        const double lum = 0.2126 * r + 0.7152 * g + 0.0722 * b;
        return lum > 160 ? "#1a1a1a" : "#ffffff";
    }

    // Converts ISO 8601 timestamp to relative time (e.g., "3 days ago")
    [[nodiscard]] static QString relativeTime(const QString& isoTimestamp)
    {
        const QDateTime dt =
            QDateTime::fromString(isoTimestamp, Qt::ISODate).toLocalTime();
        if (!dt.isValid()) return isoTimestamp;

        const qint64 secs = dt.secsTo(QDateTime::currentDateTime());

        if (secs < 60)          return "just now";
        if (secs < 3600)        return QString("%1 min ago").arg(secs / 60);
        if (secs < 86400)       return QString("%1h ago").arg(secs / 3600);
        if (secs < 86400 * 30)  return QString("%1d ago").arg(secs / 86400);
        if (secs < 86400 * 365) return QString("%1mo ago").arg(secs / (86400 * 30));
        return QString("%1y ago").arg(secs / (86400 * 365));
    }

    // Formats large numbers: 1200 → "1.2K", 1050000 → "1M"
    [[nodiscard]] static QString formatCount(int n)
    {
        if (n >= 1'000'000)
            return QString("%1M").arg(n / 1'000'000.0, 0, 'f', 1);
        if (n >= 1000)
            return QString("%1K").arg(n / 1000.0, 0, 'f', n % 1000 >= 100 ? 1 : 0);
        return QString::number(n);
    }
};
