#pragma once
#include <QString>

namespace AppConstants
{
// Application metadata
inline constexpr char APP_VERSION[]    = "1.0.0";
inline constexpr char APP_NAME[]       = "GitHub Explorer";
inline constexpr char GITHUB_API_URL[] = "https://api.github.com";
inline constexpr char ORG_NAME[]       = "MyOrg";

inline constexpr char SETTINGS_THEME_KEY[] = "appearance/theme";
inline constexpr char THEME_DARK[]         = "dark";
inline constexpr char THEME_LIGHT[]        = "light";

static constexpr int REPOS_PER_PAGE = 30;

// GitHub API rate limit (unauth: 60/hr, auth: 5000/hr)
inline constexpr int RATE_LIMIT_UNAUTH = 60;
inline constexpr int RATE_LIMIT_AUTH   = 5000;

// Spacing (8pt grid)
inline constexpr int SPACING_XS  = 4;
inline constexpr int SPACING_SM  = 8;
inline constexpr int SPACING_MD  = 12;
inline constexpr int SPACING_LG  = 16;
inline constexpr int SPACING_XL  = 24;
inline constexpr int SPACING_XXL = 32;

// UI dimensions
inline constexpr int HEADER_HEIGHT        = 48;
inline constexpr int SIDEBAR_WIDTH        = 220;
inline constexpr int NAV_BUTTON_HEIGHT    = 38;
inline constexpr int INPUT_HEIGHT         = 36;
inline constexpr int BUTTON_HEIGHT        = 34;
inline constexpr int BORDER_RADIUS        = 6;
inline constexpr int NAV_INDICATOR_WIDTH  = 3;
inline constexpr int NAV_INDICATOR_HEIGHT = 18;
inline constexpr int MIN_WINDOW_WIDTH     = 1060;
inline constexpr int MIN_WINDOW_HEIGHT    = 680;
}
