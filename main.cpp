#include <QApplication>
#include <QIcon>
#include "mainwindow.h"
#include "thememanager.h"
#include "constants.h"

int main(int argc, char* argv[])
{
    // 1. Create QApplication first — required before any Qt widget or style
    QApplication app(argc, argv);

    // 2. Set application identity (used by QSettings)
    QApplication::setApplicationName(AppConstants::APP_NAME);
    QApplication::setOrganizationName(AppConstants::ORG_NAME);
    QApplication::setApplicationVersion(AppConstants::APP_VERSION);

    // 3. Set Application Icon
    QApplication::setWindowIcon(QIcon(":/icons/app.png"));

    // 4. Use Fusion style as the base
    QApplication::setStyle("Fusion");

    // 5. Apply saved (or default) theme BEFORE showing the window
    ThemeManager::instance().loadSavedTheme();

    // 6. Create and show the main window
    MainWindow window;
    window.show();

    return QApplication::exec();
}
