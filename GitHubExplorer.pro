QT       += core gui widgets network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = GitHubExplorer
TEMPLATE = app
CONFIG   += c++17

# Include paths for simple #include "classname.h"
INCLUDEPATH += $$PWD \
               $$PWD/models \
               $$PWD/network \
               $$PWD/services \
               $$PWD/theme \
               $$PWD/ui \
               $$PWD/ui/pages \
               $$PWD/ui/widgets \
               $$PWD/utils

SOURCES += \
    main.cpp \
    network/apiclient.cpp \
    network/avatarcache.cpp \
    services/githubservice.cpp \
    theme/thememanager.cpp \
    ui/mainwindow.cpp \
    ui/pages/bookmarkspage.cpp \
    ui/pages/explorepage.cpp \
    ui/pages/searchpage.cpp \
    ui/pages/settingspage.cpp \
    ui/pages/trendingpage.cpp \
    ui/pages/welcomepage.cpp \
    ui/widgets/avatarwidget.cpp \
    ui/widgets/navbutton.cpp \
    ui/widgets/repocard.cpp \
    ui/widgets/repodetailwidget.cpp \
    ui/widgets/repolistwidget.cpp \
    ui/widgets/searchbarwidget.cpp \
    ui/widgets/toast.cpp \
    ui/widgets/userprofilewidget.cpp \
    utils/appsettings.cpp

HEADERS += \
    models/repository.h \
    models/userprofile.h \
    network/apiclient.h \
    network/avatarcache.h \
    services/githubservice.h \
    theme/thememanager.h \
    ui/mainwindow.h \
    ui/pages/bookmarkspage.h \
    ui/pages/explorepage.h \
    ui/pages/searchpage.h \
    ui/pages/settingspage.h \
    ui/pages/trendingpage.h \
    ui/pages/welcomepage.h \
    ui/widgets/avatarwidget.h \
    ui/widgets/navbutton.h \
    ui/widgets/repocard.h \
    ui/widgets/repodetailwidget.h \
    ui/widgets/repolistwidget.h \
    ui/widgets/searchbarwidget.h \
    ui/widgets/toast.h \
    ui/widgets/userprofilewidget.h \
    utils/appsettings.h \
    utils/constants.h \
    utils/helpers.h

RESOURCES += resources.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
